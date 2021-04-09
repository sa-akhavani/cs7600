#define _GNU_SOURCE /* Required for 'constructor' attribute (GNU extension) */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

void my_constructor();
// int main(int, char *[]);
static int is_in_signal_handler = 0;

#define NAME_LEN 80
struct proc_maps_line {
  void *start;
  void *end;
  char rwxp[4];
  int read, write, execute; // Not used in this version of the code
  
  
  int is_register_context; // Is this a context from sigsetjmp? If so, ignore the start_address, end_address, and read/write/execute.
  int data_size;  // Size of DATA after this HEADER.
  
  
  char name[NAME_LEN]; // for debugging only
};


// Returns 0 on success
int match_one_line(int proc_maps_fd,
                   struct proc_maps_line *proc_maps_line, char *filename) {
  unsigned long int start, end;
  char rwxp[4];
  char tmp[10];
  int tmp_stdin = dup(0); // Make a copy of stdin
  dup2(proc_maps_fd, 0); // Duplicate proc_maps_fd to stain
  // scanf() reads stdin (fd:0). It's a dup of proc_maps_fd ('same struct file').
  int rc = scanf("%lx-%lx %4c %*s %*s %*[0-9 ]%[^\n]\n",
                 &start, &end, rwxp, filename);
  // fseek() removes the EOF indicator on stdin for any future calls to scanf().
  assert(fseek(stdin, 0, SEEK_CUR) == 0);
  dup2(tmp_stdin, 0); // Restore original stdin; proc_maps_fd offset was advanced.
  close(tmp_stdin);
  if (rc == EOF || rc == 0) { 
    proc_maps_line -> start = NULL;
    proc_maps_line -> end = NULL;
    return EOF;
  }
  if (rc == 3) { // if no filename on /proc/self/maps line:
    strncpy(proc_maps_line -> name,
            "ANONYMOUS_SEGMENT", strlen("ANONYMOUS_SEGMENT")+1);
  } else {
    assert( rc == 4 );
    strncpy(proc_maps_line -> name, filename, NAME_LEN-1);
    proc_maps_line->name[NAME_LEN-1] = '\0';
  }
  proc_maps_line -> start = (void *)start;
  proc_maps_line -> end = (void *)end;
  memcpy(proc_maps_line->rwxp, rwxp, 4);
  return 0;
}


// This might cause scanf to call mmap().  If this is a problem, call it a second
//   time, and scanf() can re-use the previous memory, instead of mmap'ing more.
int proc_self_maps(struct proc_maps_line proc_maps[]) {
  // NOTE:  fopen calls malloc() and potentially extends the heap segment.
  int proc_maps_fd = open("/proc/self/maps", O_RDONLY);
  char filename [100]; // for debugging
  int i = 0;
  int rc = -2; // any value that will not terminate the 'for' loop.
  for (i = 0; rc != EOF; i++) {
    rc = match_one_line(proc_maps_fd, &proc_maps[i], filename);
#ifdef DEBUG
    if (rc == 0) {
      printf("proc_self_maps: filename: %s\n", filename); // for debugging
    }
#endif
  }
  close(proc_maps_fd);
  return 0;
}


void signal_handler(int signal) {
  is_in_signal_handler = 1;
  fprintf(stderr, "\n\n*** IN SIGNAL HANDLER. CHECKPOINTING:\n");
  // sigsetjmp => writes a new sigjmp_buf
  // save sigjmp_buf (save registers)
  // save /proc/self/maps
  struct proc_maps_line proc_maps[1000];
  assert( proc_self_maps(proc_maps) == 0 );
  // We purposely call proc_self_maps twice.  'scanf()' might call mmap()
  //   during the first execution, above.  But it doesn't need to map
  //   in a second memory segment after that.  It just re-uses the original one.
  assert( proc_self_maps(proc_maps) == 0 );
  
  my_constructor();
  is_in_signal_handler = 0;
}

void __attribute__((constructor))
my_constructor() {
    // signal handler
    signal(SIGUSR2, &signal_handler);

    fprintf(stderr, "*************************************\n"
    "***  We are running on top of the program ***\n"
    "*************************************\n");

}
