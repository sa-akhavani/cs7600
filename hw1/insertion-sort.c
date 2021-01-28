/* This code is modified from code found on the WEB at
 *   http://en.wikipedia.org/wiki/Insertion_sort
 * Note the bug fix that I added, below.  I've corrected the
 * the Wiki page with the bug fix.
 * Also, see that web page for an animation demonstrating the elagorithm.
 *  						- Gene Cooperman

/* insertSort.c */
#include <stdio.h>  /* include C declaration for printf() */
#include <stdlib.h> /* include C declaration for exit() */

void insertSort(char * a[], size_t length);
void print_array(char * a[], const int size);

int main(void)
{
  char * data[] = {"Joe", "Jenny", "Jill", "John", "Jeff", "Joyce",
		"Jerry", "Janice", "Jake", "Jonna", "Jack", "Jocelyn",
		"Jessie", "Jess", "Janet", "Jane"};
  int size = 16;

  printf("Initial array is:\n");
  print_array(data, size);
  insertSort(data, size);
  printf("Insertion sort is finished!\n");
  print_array(data, size);
  exit(0);
}


int str_lt (char *x, char *y) {
  for (; *x!='\0' && *y!='\0'; x++, y++) {
    if ( *x < *y ) return 1;
    if ( *y < *x ) return 0;
  }
  if ( *y == '\0' ) return 0;
  else return 1;
}

void insertSort(char *a[], size_t length) {
     // WAS: size_t i, j;
     // size_t can be 'unsigned int';  When j decrements from 0 to -1,
     //  this causes an underflow problem that is not caught by gcc-3.2.
     int i, j;
 
     for(i = 1; i < length; i++) {
         char *value = a[i];
         for (j = i-1; j >= 0 && str_lt(value, a[j]); j--) {
             a[j+1] = a[j];
         }
         a[j+1] = value;
     }
 }

void print_array(char * a[], const int size)
{
  int i=0;
  printf("[");
  while(i < size) printf("  %s", a[i++]);
  printf(" ]\n");
}
