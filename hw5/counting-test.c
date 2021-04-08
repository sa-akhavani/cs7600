// Miller-Rabin test:
//    https://en.wikipedia.org/wiki/Primality_test#Fermat_primality_test
//  If you want to check the number, possibly try this site:
//    https://www.numere-prime.ro/prime-factorization-of-numbers-break-down-to-prime-factors.php

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s POSITIVE_INTEGER\n", argv[0]);
    exit(1);
  }
  errno = 0;
  long num = strtol(argv[1], NULL, 10);
  if (errno != 0 || num > LONG_MAX-20) {
    fprintf(stderr, "%s: Overflow: Requested number is larger than %ld\n",
            argv[0], LONG_MAX-20);
    exit(1);
  }
  assert(num>0);
  printf("Testing for counting, starting with: %ld\n", num);
  
  long n;
  for (n = num; n < num + 10; n=n+1) {
    printf("The next counting number is: %ld\n", n);
    sleep(1);
  }
  return 0;
}
