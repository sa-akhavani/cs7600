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

struct ckpt_segment {
  void *start;
  void *end;
  char rwxp[4];
  int is_register_context; // Is this a context from sigsetjmp? If so, ignore the start_address, end_address, and read/write/execute.
  int data_size;  // Size of DATA after this HEADER.
  char name[NAME_LEN]; // for debugging only
};


void save_data(int fd, void* start_address, int length) {
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
    sprintf(buf, "%d|0|0|----|%d|NULL", 
        segment_header.is_register_context, segment_header.data_size);

  } else {
        int data_size = segment_header.end - segment_header.start;
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

// print a "\n" into the checkpoint file to separate each segment from others
void save_section_separator(int fd) {
    char newline[2] = "\n";
    int rc = write(fd, newline, 1);
    if (rc == -1) {
        perror("write");
        exit(1);
    }
    assert(rc == 1);
}

// Function that saves everything into a checkpoint files.
// myckpt stores data segments while myckpt.header stores the header data
void save_into_checkpoint_file(struct ckpt_segment proc_maps[], struct ckpt_segment cntx, ucontext_t *context, char *ckpt_filename, char *ckpt_header_filename) {
  
  int fd_h = open(ckpt_header_filename, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
  if (fd_h == -1) {perror("open");}

  int fd = open(ckpt_filename, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
  if (fd == -1) {perror("open");}

  // save context
  save_header(cntx, fd_h);
  save_section_separator(fd_h);
  // printf("saved context header\n");
  save_data(fd, context, cntx.data_size);
  save_section_separator(fd);
  // printf("saved context data\n");

  // save /proc/self/maps
  int i = 0;
  for (i = 0; proc_maps[i].start != NULL; i++) {
    // ignore [vdso], [vvar], and [vsyscall] and do not save them into checkpoint file
    if (!strcmp(proc_maps[i].name, "[vdso]") || !strcmp(proc_maps[i].name, "[vsyscall]") || !strcmp(proc_maps[i].name, "[vvar]")) {
      continue;
    }

    // Ignore segments that are not read-only
    if (proc_maps[i].rwxp[0] != 'r') {
      continue;
    }

    int data_size = proc_maps[i].end - proc_maps[i].start;
    save_header(proc_maps[i], fd_h);
    save_section_separator(fd_h);
    // printf("saved proc header with name:%s\n", proc_maps[i].name);
    save_data(fd, proc_maps[i].start, data_size);
    save_section_separator(fd);
    // printf("saved proc data\n");
  }

  close(fd);
  close(fd_h);
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



// Signal Handler
void signal_handler(int signal) {
  is_in_signal_handler = 1;
  fprintf(stderr, "\n\n**** CHECKPOINTING ****\n");

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

  // Read /proc/self/maps and put into proc_maps
  struct ckpt_segment proc_maps[1000];
  assert( proc_self_maps(proc_maps) == 0 );
  // calling this function twice creates issues sometimes and we will not be able to . So I just call it once. 
  // assert( proc_self_maps(proc_maps) == 0 );
  

  // saving each memory segment into mycckpt
  char *ckpt_name = "myckpt";
  char *ckpt_header_name = "myckpt.header";
  save_into_checkpoint_file(proc_maps, cntx, &context, ckpt_name, ckpt_header_name);

  // call constructor again
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
