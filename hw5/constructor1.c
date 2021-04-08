#define _GNU_SOURCE /* Required for 'constructor' attribute (GNU extension) */
#include <stdio.h>

void __attribute__((constructor))
my_constructor() {
  fprintf(stderr, "*************************************\n"
                  "***  We are running, using test1. ***\n"
                  "*************************************\n");
}
