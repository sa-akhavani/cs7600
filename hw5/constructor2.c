#define _GNU_SOURCE /* Required for 'constructor' attribute (GNU extension) */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int, char *[]);

#ifdef LOOKUP_MAIN_SYMBOL
typedef void *(*main_ptr_t)(char *, char *[]);
void *get_main_address(char *target) {
  char command[100];
  long unsigned int main_address;
  snprintf(command, sizeof command, "nm %s | grep ' main$'", target);
  FILE *cmd = popen(command, "r");
  fscanf(cmd, "%x", (void *)&main_address);
  return (void *)main_address;
}
#endif

void __attribute__((constructor))
my_constructor() {
  fprintf(stderr, "*************************************\n"
                  "***  We are running, using test2. ***\n"
                  "*************************************\n");
  fprintf(stderr, "*** What integer should we test? ");
  fflush(stdout);
  char buf[100] = {'\0'};  // null characters at end of string
  read(0, buf, sizeof(buf)-1);
  char *argv[] = {"UNKNOWN", "12345", NULL};
  argv[0] = getenv("TARGET");
  argv[1] = buf;
#ifdef LOOKUP_MAIN_SYMBOL
  // If the target was cmopiled with -rdynamic in 'gcc', then we don't
  //   need this code.  If we didn't use -rdynamic, then this code would
  //   find the address of 'main()' by more obscure means.
  main_ptr_t = get_main_address(getenv("TARGET"));
  (*main_ptr_t)(argv[/* argc */ 2, argv);
#endif
  // This assumes that our target was compiled with the -rdynamic to gcc.
  // Otherwise, we would define LOOKUP_MAIN_SYMBOL in this code.
#ifndef LOOKUP_MAIN_SYMBOL
  main(/* argc */ 2, argv);
#endif
  // The constructor exits now.  Otherwise, if it returns, then the linker
  // will then call 'main()' in the normal way.
  exit(0);
}
