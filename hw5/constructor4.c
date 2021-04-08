#define _GNU_SOURCE /* Required for 'constructor' attribute (GNU extension) */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void my_constructor();
int main(int, char *[]);
static int is_in_signal_handler = 0;

void signal_handler(int signal) {
  is_in_signal_handler = 1;
  fprintf(stderr, "\n\n*** RESTARTING:\n");
  // my_constructor() will declare this handler for SIGINT one more time.
  // Unfortunately, we're inside the SIGINT signal handler, and so
  //   the Linux kernel will block any new SIGINT by the user until
  //   we exit from this SIGINT handler.  So, ^C will be ineffective
  //   if used a second time, until the current call to my_constructor
  //   completes.  At that time, any pending ^C can take effect
  //   and bring us back into this signal handler.
  my_constructor();
  is_in_signal_handler = 0;
}

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
  signal(SIGINT, &signal_handler);

  fprintf(stderr, "*************************************\n"
                  "***  We are running, using test4. ***\n"
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
  if (! is_in_signal_handler) {
    exit(0);
  }
}
