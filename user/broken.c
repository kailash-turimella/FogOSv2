/**
 * @file broken.c
 * @author mmalensek
 *
 * This program contains a series of buggy, broken, or strange C functions for
 * you to ponder. Your job is to analyze each function, fix whatever bugs the
 * functions might have, and then explain what went wrong. Sometimes the
 * compiler will give you a hint.
 *
 *  ____________
 * < Good luck! >
 *  ------------
 *      \   ^__^
 *       \  (oo)\_______
 *          (__)\       )\/\
 *              ||----w |
 *              ||     ||
 */

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


static int func_counter = 1;
#define FUNC_START() printf("\n\n%d.) %s\n", func_counter++, __func__);

#pragma GCC diagnostic ignored "-Waggressive-loop-optimizations"
#pragma GCC diagnostic ignored "-Wdangling-pointer"
#pragma GCC diagnostic ignored "-Wformat-overflow"

/**
 * This code example was taken from the book 'Mastering C Pointers,' one of the
 * not so good ways to learn pointers. It was trying to demonstrate how to print
 * 'BNGULAR'... with pointers...? Maybe?
 *
 * (1) Fix the problem.
 * (2) Explain what's wrong with this code:
 *      String literals are stored in read only memory
 *      '66' in this example is 'B'. What we can do is set a[0] = 'B' to replace the first char in the string.
 */
void
angular(void)
{
  FUNC_START();

  char a[] = "ANGULAR";
  a[0] = 'B';
  printf("%s\n", a);
}

/**
 * This function is the next step after 'Hello world' -- it takes user input and
 * prints it back out! (Wow).
 *
 * But, unfortunately, it doesn't work.
 *
 * (1) Fix the problem.
 * (2) Explain what's wrong with this code:
 *
 *  The main problem for this one is that we have to allocate space
 	for the string. I tried to change gets to fgets but I don't think 
 	that works because using stdio.h didn't work.
 
 */
void
greeter(void)
{
  FUNC_START();

  char name[128];

  printf("Please enter your name: ");
  gets(name, 128);

  // Remove newline character
  char *p = name;
  for ( ; *p != '\n' && *p != 0; p++) { }
  *p = '\0';

  printf("Hello, %s!\n", name);
}

/**
 * This code isn't so much broken, but more of an exercise in understanding how
 * C data types are represented in memory.
 *
 * (1) Fill in your guesses below to see if you can get all 6 correct on the
 *     first try.
 * (2) Explain *WHY* you get the sizes you do for each of the 6 entries.
 *     sizeof(int) and sizeof(float) are fairly straightforward.

 	A char is always 1. Both ints and floats are 4. 
 	things is 48 because it is an array of 12 ints. so 12 x 4 = 48
 	'A' is an int so thats why its 4 bytes.
 	"A" is a string literal so it is 2 bytes. The null terminator "\0" is included in there.
 *
 */
#define SIZE_CHECK(sz, guess) (sz), sz == guess ? "Right!" : "Wrong!"
void
sizer(void)
{
  FUNC_START();

  int guesses[] = {1, 4, 4, 48, 4, 2 }; // fill in 6 guesses here
  if (sizeof(guesses) / sizeof(int) != 6) {
    printf("Please enter a guess for each of the sizes below.\n");
    printf("Example: guesses[] = { 1, 4, 0, 0, 0, 0 }\n");
    printf("would mean sizeof(char) == 1, sizeof(int) == 4, and so on");
    return;
  }

  int things[12] = { 0 };
  printf("sizeof(char)   = %ld\t[%s]\n", SIZE_CHECK(sizeof(char), guesses[0]));
  printf("sizeof(int)    = %ld\t[%s]\n", SIZE_CHECK(sizeof(int), guesses[1]));
  printf("sizeof(float)  = %ld\t[%s]\n", SIZE_CHECK(sizeof(float), guesses[2]));
  printf("sizeof(things) = %ld\t[%s]\n", SIZE_CHECK(sizeof(things), guesses[3]));
  printf("sizeof('A')    = %ld\t[%s]\n", SIZE_CHECK(sizeof('A'), guesses[4]));
  printf("sizeof(\"A\")    = %ld\t[%s]\n", SIZE_CHECK(sizeof("A"), guesses[5]));
}

/**
 * This 'useful' function prints out an array of integers with their indexes, or
 * at least tries to. It even has a special feature where it adds '12' to the
 * array.
 *
 * (1) Fix the problem.
 * (2) Explain what's wrong with this code:
 *
 *   One problem I found is that the loop is until < 1000, which exceeds the size we set for
     the stuff array which is 100. 
     Another problem or inconvinience is the 14[stuff + 1] = 12. At first I didn't understand it,
     but after searching online what it means. It basically just sets 12 to index 15. 
 */
void
displayer(void)
{
  FUNC_START();

  int stuff[100] = { 0 };

  /* Can you guess what the following does without running the program? */
  /* Rewrite it so it's easier to read. */
  stuff[15] = 12;

  for (int i = 0; i < 100; ++i) {
    printf("%d: %d\n", i, stuff[i]);
  }
}

/**
 * Adds up the contents of an array and prints the total. Unfortunately the
 * total is wrong! See main() for the arguments that were passed in.
 *
 * (1) Fix the problem.
 * (2) Explain what's wrong with this code:
 *
 *   The problem is that sizeof(arr) doesn't work as intended. It wouldn't get to the 
     end of the loop. To fix it I changed the function call so we can get the correct size of
     array is use that to loop through.
 */
void
adder(int *arr, int n)
{
  FUNC_START();

  int total = 0;

  for (int i = 0; i < n; ++i) {
  	total += arr[i];
  }

  printf("Total is: %d\n", total);
}

/**
 * This function is supposed to be somewhat like strcat, but it doesn't work.
 *
 * (1) Fix the problem.
 * (2) Explain what's wrong with this code:
 *	The problem is that it returns a pointer to a local array. When the function returns, it finishes.
 	To fix we use malloc to allocate it on the heap.
 *   
 */
char *
suffixer(char *a, char *b)
{
  FUNC_START();

  char *buf = malloc(strlen(a) + strlen(b) + 1);
  if (!buf) return 0;
  
  char *buf_start = buf;
  
  strcpy(buf, a);
  strcpy(buf + strlen(a), b);
  return buf_start;

}

/**
 * This is an excerpt of Elon Musk's Full Self Driving code. Unfortunately, it
 * keeps telling people to take the wrong turn. Figure out how to fix it, and
 * explain what's going on so Elon can get back to work on leaving Earth for
 * good.
 *
 * (1) Fix the problem.
 * (2) Explain what's wrong with this code:
 *
 *   One problem is that in the if statement we aren't checking if its actually
     true or not. So we have to add == 0 for it to work. The whole string comparison
     part is also not needed.

     There is also 2 overflow issues. 
 */
void
driver(void)
{
  FUNC_START();

  char street1[10] = { "fulton" };
  char street2[10] = { "gomery" };
  char street3[10] = { "piedmont" };
  char street4[10] = { "baker" };
  char street5[10] = { "haight" };

  if (strcmp(street1, street2) == 0) {
    char *new_name = "saint agnew ";
    memcpy(street4, new_name, strlen(new_name));
  }

  printf("Welcome to TeslaOS 0.1!\n");
  printf("Calculating route...\n");
  printf("Turn left at the intersection of %s and %s.\n", street5, street3);
}

/**
 * This function tokenizes a string by space, sort of like a basic strtok or
 * strsep. It has two subtle memory bugs for you to find.
 *
 * (1) Fix the problem.
 * (2) Explain what's wrong with this code:
 *
 *   One problem was is a size bug. the malloc allocates a byte too few
 	 so strcpy(line, str) overflows.
 	 Once the scan hits '\0', if it advances it would go past the buffer.
 	 
 */
void
tokenizer(void)
{
  FUNC_START();

  char *str = "Hope was a letter I never could send";
  char *line = malloc(strlen(str) + 1);
  char *base = line;
  char *c = line;

  strcpy(line, str);

  while (*c != '\0') {

    for ( ; *c != ' ' && *c != '\0'; c++) {
      // find the next space (or end of string)
    }

	if (*c == '\0') {
		printf("%s\n", line);
		break;
	}
	
    *c = '\0';
    printf("%s\n", line);

    line = c + 1;
    c = line;
  }

  free(base);
}

/**
* This function should print one thing you like about C, and one thing you
* dislike about it. Assuming you don't mess up, there will be no bugs to find
* here!
*/
void
finisher(void)
{
  FUNC_START();

  // TODO
   printf("Something I like about C is how you can control much more about how everything works.\n");
   printf("Something I don't like is that having a lot more control means more stuff can go wrong :( ");
}

int
main(void)
{
  printf("Starting up!");


  angular();

  greeter();

  sizer();

  displayer();

  int nums[] = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512 };
  adder(nums, sizeof(nums) / sizeof(nums[0]));

  char *result = suffixer("kayak", "ing");
  printf("Suffixed: %s\n", result);

  driver();

  tokenizer();

  finisher();

  return 0;
}
