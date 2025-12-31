#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

int fgets(char *buf, unsigned int max, int fd);

uint
strspn(const char *str, const char *chars)
{
  uint i, j;
  for (i = 0; str[i] != '\0'; i++) {
    for (j = 0; chars[j] != str[i]; j++) {
      if (chars[j] == '\0')
        return i;
    }
  }
  return i;
}

uint
strcspn(const char *str, const char *chars)
{
  const char *p, *sp;
  char c, sc;
  for (p = str;;) {
    c = *p++;
    sp = chars;
    do {
      if ((sc = *sp++) == c) {
        return (p - 1 - str);
      }
    } while (sc != 0);
  }
}

char
*next_token(char **str_ptr, const char *delim)
{
  if (*str_ptr == 0) {
    return 0;
  }

  uint tok_start = strspn(*str_ptr, delim);
  uint tok_end = strcspn(*str_ptr + tok_start, delim);

  /* Zero length token. We must be finished. */
  if (tok_end  == 0) {
    *str_ptr = 0;
    return 0;
  }

  /* Take note of the start of the current token. We'll return it later. */
  char *current_ptr = *str_ptr + tok_start;

  /* Shift pointer forward (to the end of the current token) */
  *str_ptr += tok_start + tok_end;

  if (**str_ptr == '\0') {
    /* If the end of the current token is also the end of the string, we
         * must be at the last token. */
    *str_ptr = 0;
  } else {
    /* Replace the matching delimiter with a NUL character to terminate the
         * token string. */
    **str_ptr = '\0';

    /* Shift forward one character over the newly-placed NUL so that
         * next_pointer now points at the first character of the next token. */
    (*str_ptr)++;
  }

  return current_ptr;
}

// helper func to just print the basic prompt
static void print_prompt(int last_status, int cmd_num) {
	char path[128];
	int n = getcwd(path, sizeof(path));
	if (n < 0) {
		path[0] = '/'; path[1] = 0;
	}
	printf("[%d]-[%d]-[%s]$ ", last_status, cmd_num, path);
}

static int is_blank(const char *s) {
	for (; *s; s++) {
		if (*s!= ' ' && *s!= '\t' && *s!= '\n' && *s!= '\r') return 0;
	}
	return 1;
}

// helper fun to remove remove newliens and return characters
static void strip_trailing_newline(char *buf, int *len_io) {
	int len = *len_io;
	while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r')) {
		buf[--len] = 0;
	}
	*len_io = len;
}

// helper to check for comments and remove them
static void remove_comment(char *buf) {
	for (int i = 0; buf[i]; i++) {
		if (buf[i] == '#') {
			buf[i] = 0;
			break;
		}
	}
}

#define MAX_HIST 100
struct history_entry {
	int num;
	char line[256];
	int duration_ms;
};

static struct history_entry hist[MAX_HIST];

// method to add to the history
static void add_history(const char *line, int cmd_num) {
	if (!line || !*line) return;
	int slot = (cmd_num - 1) % MAX_HIST;
	hist[slot].num = cmd_num;

	int i = 0;
	for (; i < (int)sizeof(hist[slot].line) - 1 && line[i]; i++) {
		hist[slot].line[i] = line[i];
	}
	hist[slot].line[i] = 0;
}

// actual history command
static void history_command(int current_cmd_num, int show_time) {
	int last_done = current_cmd_num - 1;   // last completed command
	if (last_done < 1) return;

	int first = last_done - MAX_HIST + 1;
	if (first < 1) first = 1;

	for (int n = first; n <= last_done; n++) {
		int slot = (n - 1) % MAX_HIST;
		if (hist[slot].num == n && hist[slot].line[0] != 0) {
			if (show_time) {
				printf("[%d|%dms] %s\n", hist[slot].num, hist[slot].duration_ms, hist[slot].line);
			} else {
				printf("%d %s\n", hist[slot].num, hist[slot].line);
			}
		}
	}
}

// lookup in history with number arg
static const char* hist_num(int num) {
	if (num < 1) return 0;
	int slot = (num - 1) % MAX_HIST;

	// make sure that everything matches and line isn't empty
	if (hist[slot].num == num && hist[slot].line[0] != 0) return hist[slot].line;

	return 0;
}

// lookup in history with prefix arg
static const char* hist_prefix(const char *prefix, int last_done) {
	int prefix_len = 0;
	while(prefix[prefix_len])
		prefix_len++;

	// go through all the commands 
	// from newest to oldest
	for (int n = last_done; n >= 1; n--) {
		int slot = (n - 1) % MAX_HIST;

		if (hist[slot].num != n || hist[slot].line[0] == 0) 
			continue;
		
		const char *s = hist[slot].line;

		// compare prefix against start of the command
		int i = 0;
		while (i < prefix_len && s[i] == prefix[i])
			i++;

		// if they match return the string
		if (i == prefix_len)
			return s;
	}

	return 0;
}

int
main(int argc, char *argv[])
{

  // -1. print a prompt
  //  0. get the user's command (stdin)
  //  1. fork
  //  2. exec
  

  int last_status = 0;  // 0 = success, 1 = fail
  int cmd_num = 1; 
  int scripting = 0;
  int script_fd = -1;

  if (argc > 1) {
  	script_fd = open(argv[1], O_RDONLY);
  	if (script_fd < 0) {
  		fprintf(2, "shell: cannot open %s", argv[1]);
  		exit(1);
  	}
  	scripting = 1;
  }  
  

  while (1) {
    if (!scripting) {
    	print_prompt(last_status, cmd_num);
    }
   	
    char buf[256];

    int r;
    if (scripting) {
    	int i = 0;
    	while (i + 1 < sizeof(buf)) {
    		char c;
    		r = read(script_fd, &c, 1);
    		if (r < 1) {
    			break;
    		}
    		buf[i++] = c;
    		if (c == '\n')
    			break;
    	}
    	buf[i] = 0;

    	if (i == 0) {
    		//fprintf(2, "shell: script EOF, exiting\n");
    		exit(0);
    	}
    } else {
    	if (gets(buf, sizeof(buf)) == 0) {
    		printf("\n");
    		exit(0);
    	}
    }


	int got = strlen(buf);
   
	// remove "\n" and comments
	strip_trailing_newline(buf, &got);
	remove_comment(buf);
	
	// skip blank lines
	if (is_blank(buf)) {
		continue;
	}

	/*
	Extra history commands:
	1. "!" -> last command
	2. "!5" -> command number 5
	3. "!prefix" -> most recent command with the prefix
	*/

	if (buf[0] == '!') {
		int last_done = cmd_num - 1;
		const char *expanded = 0;

		// the prev command
		if (buf[1] == '!' && buf[2] == 0) {
			expanded = hist_num(last_done);
		} else if (buf[1] >= '0' && buf[1] <= '9') {
			int n = 0;
			for (int i = 1; buf[i] >= '0' && buf[i] <= '9'; i++) {
				n = n * 10 + (buf[i] - '0');
			}
			expanded = hist_num(n);
		} else {
			// !prefix
			expanded = hist_prefix(buf + 1, last_done);
		}

		if (!expanded) {
			fprintf(2, "history: no match for '%s'\n", buf);
			last_status = 1;
			cmd_num++;
			continue;
		}

		
		int i = 0;
		while (i < (int)sizeof(buf) - 1 && expanded[i]) {
			buf[i] = expanded[i];
			i++;
		}
		buf[i] = 0;

		printf("%s\n", buf);
	}

	// record this command before executing so numbers align
    add_history(buf, cmd_num);


    char *args[32];
    int tokens = 0;
    char *next_tok = buf;
    char *curr_tok;
    /* Tokenize. */
    while ((curr_tok = next_token(&next_tok, " \n\t\r")) != 0 && tokens < 31) {
     // printf("Token: '%s'\n", curr_tok);
      args[tokens++] = curr_tok;
    }
    args[tokens] = 0;
    if (tokens == 0) {
    	continue;
    }


    // built in commands
    if (strcmp(args[0], "exit") == 0) {
    	exit(0);
    }

	// history command
    if (strcmp(args[0], "history") == 0) {
    	int show_time = 0;
    	if (tokens > 1 && strcmp(args[1], "-t") == 0)
    		show_time = 1;
    	history_command(cmd_num, show_time);
    	last_status = 0;
    	cmd_num++;
    	continue;
    }

    
    if (strcmp(args[0], "cd") == 0) {
    	if (tokens < 2) {
    		fprintf(2, "chdir: missing arg\n");
    		last_status = 1;
    	} else if (chdir(args[1]) < 0) {
    		fprintf(2, "chdir: no file or directory found: %s\n", args[1]);
    		last_status = 1;
    	} else {
    		last_status = 0;
    	}
    	cmd_num++;
    	continue;
    }

    uint64 start = clock();

    int pid = fork();
    if (pid < 0 ) {
      fprintf(2, "Fork failed");
      last_status = 1;
      cmd_num++;
      continue;
      
    } else if (pid == 0) {
      // child
      // execute the command
      exec(args[0], args);

	  fprintf(2, "exec failed: %s\n", args[0]);
	  exit(1);
      
    } else {
      wait(0);
      
      uint64 end = clock();
      uint64 diff = end - start;
      int diff_ms = (int)(diff / 1000000ULL);

      int slot = (cmd_num - 1) % MAX_HIST;
      hist[slot].duration_ms = diff_ms;
      
      cmd_num++;
    }
  }

  return 0;
}
