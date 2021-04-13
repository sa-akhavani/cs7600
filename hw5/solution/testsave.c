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
#include <ucontext.h>

void my_constructor();
static int is_in_signal_handler = 0;

#define NAME_LEN 80
#define HEADER_LEN 

struct ckpt_segment {
  void *start;
  void *end;
  char rwxp[4];
  int is_register_context; // Is this a context from sigsetjmp? If so, ignore the start_address, end_address, and read/write/execute.
  int data_size;  // Size of DATA after this HEADER.
  char name[NAME_LEN]; // for debugging only
};


void save(int fd, void* start_address, int length) {
  int rc = write(fd, (void *)start_address, length);
  if (rc == -1) {
    perror("write");
    exit(1);
  }
  while (rc < length) {
    rc += write(fd, (char *)start_address + rc, length - rc);
  }
  assert(rc == length);
}

void save_header(struct ckpt_segment segment_header, int fd) {
  char buf[200];

  if (segment_header.is_register_context) {
    sprintf(buf, "%d|NULL|NULL|----|%d|NULL", 
        segment_header.is_register_context, segment_header.data_size);

  } else {
        int data_size = segment_header.end - segment_header.start + 1;
        sprintf(buf, "%d|%p|%p|%c%c%c%c|%d|%s", 
            segment_header.is_register_context, segment_header.start, segment_header.end, 
            segment_header.rwxp[0], segment_header.rwxp[1], segment_header.rwxp[2], segment_header.rwxp[3], 
            data_size, segment_header.name);
  }
  
  
  int nbytes = strlen(buf);
  int rc = write(fd, buf, nbytes);
  if (rc == -1) {
    perror("write");
    exit(1);
  }
  while (rc < nbytes) {
    rc += write(fd, buf + rc, nbytes - rc);
  }

  assert(rc == nbytes);
}

// print a newline into the checkpoint file
void save_newline(int fd) {
    char newline[4] = "\n,\n";
    int rc = write(fd, newline, 3);
    if (rc == -1) {
        perror("write");
        exit(1);
    }
    assert(rc == 3);
}

// Checkpoint file structure (newline separated):
// Header1
// Data1
// Header2
// Data2
// ...
void save_into_checkpoint_file(struct ckpt_segment proc_maps[], struct ckpt_segment cntx, ucontext_t *context, char *filename) {
  int fd = open(filename, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
  if (fd == -1) {perror("open");}

  // save context
  save_header(cntx, fd);
  // print new line after each segment
  save_newline(fd);
  save(fd, context, cntx.data_size);
  save_newline(fd);

  // save /proc/self/maps
  int i = 0;
  for (i = 0; proc_maps[i].start != NULL; i++) {
    // ignore [vdso], [vvar], and [vsyscall] and do not save them into checkpoint file
    if (!strcmp(proc_maps[i].name, "[vdso]") || !strcmp(proc_maps[i].name, "[vsyscall]") || !strcmp(proc_maps[i].name, "[vvar]")) {
      continue;
    }

    save_header(proc_maps[i], fd);
    // print new line after each segment
    save_newline(fd);
    int data_size = proc_maps[i].end - proc_maps[i].start + 1;
    printf("%d\n", data_size);
    save(fd, &proc_maps[i].start, data_size);
    // print new line after each segment
    save_newline(fd);
  }

  close(fd);
}


// Returns 0 on success
int match_one_line(int proc_maps_fd,
                   struct ckpt_segment *ckpt_segment, char *filename) {
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
    ckpt_segment -> start = NULL;
    ckpt_segment -> end = NULL;
    return EOF;
  }
  if (rc == 3) { // if no filename on /proc/self/maps line:
    strncpy(ckpt_segment -> name,
            "ANONYMOUS_SEGMENT", strlen("ANONYMOUS_SEGMENT")+1);
  } else {
    assert( rc == 4 );
    strncpy(ckpt_segment -> name, filename, NAME_LEN-1);
    ckpt_segment->name[NAME_LEN-1] = '\0';
  }
  ckpt_segment -> start = (void *)start;
  ckpt_segment -> end = (void *)end;
  memcpy(ckpt_segment->rwxp, rwxp, 4);
  return 0;
}


// This might cause scanf to call mmap().  If this is a problem, call it a second
//   time, and scanf() can re-use the previous memory, instead of mmap'ing more.
int proc_self_maps(struct ckpt_segment proc_maps[]) {
  // NOTE:  fopen calls malloc() and potentially extends the heap segment.
  int proc_maps_fd = open("/proc/self/maps", O_RDONLY);
  char filename [100]; // for debugging
  int i = 0;
  int rc = -2; // any value that will not terminate the 'for' loop.
  for (i = 0; rc != EOF; i++) {
    rc = match_one_line(proc_maps_fd, &proc_maps[i], filename);
  }
  close(proc_maps_fd);
  return 0;
}



void main() {
  // Save Context
  ucontext_t context;  // A context includes the register values
  static int is_restart; // static local variables are stored in data segment
  is_restart = 0;
  getcontext(&context);
  if (is_restart == 1) {
    is_restart = 0;
    return;
  }
  is_restart = 1;

  // Create a context header to save into myckpt
  struct ckpt_segment cntx;
  cntx.is_register_context = 1;
  cntx.data_size = sizeof(context);

  // Read /proc/self/maps
  struct ckpt_segment proc_maps[1000];
  assert( proc_self_maps(proc_maps) == 0 );
//   assert( proc_self_maps(proc_maps) == 0 ); // proc_self_maps called twice
  

  // saving each memory segment into mycckpt
  char *filename = "myckpt";
  save_into_checkpoint_file(proc_maps, cntx, &context, filename);


  // Debugging
//   int i = 0;
//   for (i = 0; proc_maps[i].start != NULL; i++) {
//     printf("%s (%c%c%c)\n"
//            "  Address-range: %p - %p\n",
//            proc_maps[i].name,
//            proc_maps[i].rwxp[0], proc_maps[i].rwxp[1], proc_maps[i].rwxp[2],
//            proc_maps[i].start, proc_maps[i].end);
//   }
}