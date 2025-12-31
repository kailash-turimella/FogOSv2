#define SBRK_ERROR ((char *)-1)

struct stat;

// system calls
int fork(void);
int exit(int) __attribute__((noreturn));
int wait(int *status);
int pipe(int *pipefd);
int write(int fd, const void *buf, int count);
int read(int fd, void *buf, int count);
int close(int fd);
int kill(int pid);
int exec(const char* path, char **argv);
int open(const char*/*str*/ path, int mode);
int mknod(const char* /*str*/ path, short major, short minor);
int unlink(const char* /*str*/  path);
int fstat(int fd, struct stat* st);
int link(const char* /*str*/old_path, const char* /*str*/ new_path);
int mkdir(const char* /*str*/ path);
int chdir(const char* /*str*/ path);
int dup(int fd);
int getpid(void);
char* sys_sbrk(int n, int incr);
int pause(int);
int uptime(void);
int shutdown(void);
int reboot(void);
int clock(void);
int strace_on(void);
int wait2(int *status, int *counter);
int nice(int n);
int getcwd(char *, int);
int freemem(void);
void* mmap(void);

// ulib.c
int stat(const char*, struct stat*);
char* strcpy(char*, const char*);
void *memmove(void*, const void*, int);
char* strchr(const char*, char c);
int strcmp(const char*, const char*);
char* gets(char*, int max);
uint strlen(const char*);
void* memset(void*, int, uint);
int atoi(const char*);
int memcmp(const void *, const void *, uint);
void *memcpy(void *, const void *, uint);
char* sbrk(int);
char* sbrklazy(int);
int getline(char **buf, int sizep, int fd);

// printf.c
void fprintf(int, const char*, ...) __attribute__ ((format (printf, 2, 3)));
void printf(const char*, ...) __attribute__ ((format (printf, 1, 2)));

// umalloc.c
void* malloc(uint);
void free(void*);
void* calloc(uint, uint);
void* realloc(void*, uint);
void malloc_name(void *ptr, const char *name);
void malloc_print(void);
void malloc_setfsm(int);
