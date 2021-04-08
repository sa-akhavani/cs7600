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

struct s_and_d {
  long s;
  long d;
};

long mult_modulo(long x, long y, long modulo) {
  assert( x < modulo && y < modulo );
  if (x < INT_MAX && y < INT_MAX) {
    return x*y % modulo;
  } else {
    long multiplier = LONG_MAX / modulo;
    long sum = 0;
    while ( x > 0 ) {
      sum = (sum + (x % multiplier)*y) % modulo;
      x = x - (x % multiplier);
      x = x / multiplier;
      y = y * multiplier % modulo;
    }
    return sum;
  }
}

// We can do this modulo n for very large numbers.
long ipow_modulo(long base, long exp, long modulo) {
  long result = 1;
  while (1) {
    if (exp % 2 == 1) {
      result = mult_modulo(result, base, modulo);
    }
    exp = exp / 2;
    assert( exp >= 0);
    if (exp == 0) {
      break;
    }
    base = mult_modulo(base, base, modulo);
  }
  return result;
}

void find_s_d(long n, struct s_and_d *s_d) {
  long s = 0;
  long d = n-1;
  while (d % 2 == 0) {
    s++;
    d = d/2;
  }
  s_d->s = s;
  s_d->d = d;
}

long rand_lt_n(long n) {
  return (rand() * INT_MAX + rand()) % (n-1) + 1;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s POSITIVE_INTEGER\n"
                    "       %s NETATIV_INTEGER [no pause, non-stop]\n",
            argv[0], argv[0]);
    exit(1);
  }
  errno = 0;
  long num = strtol(argv[1], NULL, 10);
  int do_sleep = 1;
  if (errno != 0 || num > LONG_MAX-20 || num < LONG_MIN) {
    fprintf(stderr, "%s: Overflow: Requested number is larger than %ld\n",
            argv[0], LONG_MAX-20);
    exit(1);
  }
  assert(num!=0);
  if (num < 0) {
    do_sleep = 0;
    num = -num;
  } else {
    do_sleep = 1;
  }
  long NUM_PRIMALITY_TESTS = 100;
  printf("Testing for primes, starting with: %ld\n", num);
  
  long n;
  int MAX_ITERATIONS = 20;
  if (! do_sleep) {
    MAX_ITERATIONS = 10000;
  }
  for (n = num; n < num + MAX_ITERATIONS; n=n+2) { // skipping even numbers
    long num_composite_witnesses = 0;
    printf("Is %ld probably prime? ", n);
    struct s_and_d s_d;
    find_s_d(n, &s_d);
    long s = s_d.s;
    long d = s_d.d;
    long i;
    for(i = 0; i < NUM_PRIMALITY_TESTS; i++) {
      long a = rand_lt_n(n); // random number with a < n
    
      if (ipow_modulo(a, d, n) % n == 1) {
        continue;
      }
      long r;
      long is_composite_witness = 1;
      for (r = 0; r < s; r++) {
        long exp = ipow_modulo(2,r,n)*d % n;
        if (ipow_modulo(a, exp, n) % n == n-1) {
          is_composite_witness = 0;
          break;
        }
      }
      if (is_composite_witness) {
        num_composite_witnesses++;
      }
    }
    if (num_composite_witnesses < NUM_PRIMALITY_TESTS/4) {
      printf("Yes\n");
    } else {
      printf("No\n");
    }
    if (do_sleep) {
      sleep(1);
    }
  }
  return 0;
}
