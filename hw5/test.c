#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h> // for dirname()
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>

int main(int argc, char* argv[]) {
  char **target_argv = argv+1;
  struct stat stat_buf;
  if (target_argv[0] == NULL || stat(target_argv[0], &stat_buf) == -1) {
    fprintf(stderr, "*** Missing target executable or no such file.\n\n");
    exit(1);
  }

  char test_num = argv[0][strlen(argv[0])-1];
  if (test_num < '1' || test_num > '5') {
    test_num = '1';
  }

  char buf[1000];
  buf[sizeof(buf)-1] = '\0';
  snprintf(buf, sizeof buf, "%s/libconstructor%c.so",
           dirname(argv[0]), test_num);
  // Guard against buffer overrun:
  assert(buf[sizeof(buf)-1] == '\0');
  setenv("LD_PRELOAD", buf, 1);
  setenv("TARGET", argv[1], 1);

  int rc = execvp(target_argv[0], target_argv);
  // We should not reach here.
  fprintf(stderr, "Executable '%s' not found.\n", target_argv[0]);
  return 1;
}
