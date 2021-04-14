#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <ucontext.h>

#define NAME_LEN 80
ucontext_t context;

struct ckpt_segment {
  void *start;
  void *end;
  char rwxp[4];
  int is_register_context; // Is this a context from sigsetjmp? If so, ignore the start_address, end_address, and read/write/execute.
  int data_size;  // Size of DATA after this HEADER.
  char name[NAME_LEN]; // for debugging only
};


// // Read header segment from myckpt.header file
// This is the header format:
// iscontext|start|end|rwxp|datasize|name
int read_header(int fd, struct ckpt_segment *ckpt_segment, char *filename) {
    unsigned long int start, end;
    char rwxp[4];
    int is_register_context;
    int data_size;
    int rc;
    char tmp[10];
    int tmp_stdin = dup(0);
    dup2(fd, 0);

    rc = scanf("%d|%lx|%lx|%4c|%d|%s[^\n]",
        &is_register_context, &start, &end, rwxp, &data_size, filename);
    assert(fseek(stdin, 0, SEEK_CUR) == 0);

    dup2(tmp_stdin, 0); // Restore original stdin; file offset was advanced.
    close(tmp_stdin);


    if (rc == 0) {
        return 0;
    }
    if (rc == EOF) {
        return EOF;
    }

    ckpt_segment->is_register_context = is_register_context;
    ckpt_segment->data_size = data_size;
    if (!is_register_context) {
        ckpt_segment -> start = (void *)start;
        ckpt_segment -> end = (void *)end;
        strncpy(ckpt_segment->name, filename, NAME_LEN-1);
        memcpy(ckpt_segment->rwxp, rwxp, 4);
    }

    return rc;
}

// Read data segment from myckpt file
int read_data(int fd, struct ckpt_segment segment) {
    void *addr;

    // Read Context if we see is_register_context is 1
    if (segment.is_register_context) {
        read(fd, &context, segment.data_size);
        

    } else { // Restore memory data segment if is_register_context is 0

        // Setting flags and prot for mmap
        int flags = 0;
        if (!strcmp(segment.name, "ANONYMOUS_SEGMENT")) {
            flags = flags | MAP_ANONYMOUS;
        }

        int prot = PROT_READ;
        if (segment.rwxp[1] == 'w')
            prot = prot | PROT_WRITE;
        if (segment.rwxp[1] == 'x')
            prot = prot | PROT_EXEC;
        if (segment.rwxp[1] == 'p')
            flags = flags | MAP_PRIVATE;

        // Calling mmap after setting prot and flags
        addr = mmap(segment.start, segment.data_size,
                            prot, flags, -1, 0);
        
        int rc = read(fd, addr, segment.data_size);
        if (rc == -1) {
            perror("read");
            exit(1);
        }
        while (rc < segment.data_size) {
            rc += read(fd, addr + rc, segment.data_size - rc);
        }
        assert(rc == segment.data_size);
    }

    printf("Memory segment of length %d mmap'ed in at address %p.\n",
            segment.data_size, segment.start);

    // TO DETECT CURRENT MEMORY SEGMENTS, DO:  (gdb) info proc mappings
    // // But if you need to see the permissions for the segment, then do:
    // (gdb) info proc  # This shows the process PID
    // (gdb) shell cat /proc/PID/maps

    // Read newline in order to reach next data segment in the next call to this function
    char junk[1];
    int new_rc = read(fd, junk, 1);
    if (new_rc == -1) {
        perror("read");
        exit(1);
    }
    assert(new_rc == 1);
    return 1;
}


void do_work() {
    //  We will
    //  map in each memory segment in 'myckpt'.  We will use the MAP_FIXED
    //  flag from 'man mmap' to specify the exact address to restore the segment.

    struct ckpt_segment proc_maps[1000];
    char filename [200];

    // Open checkpoint data file
    int fd = open("myckpt", O_RDONLY);
    if (fd == -1) {perror("open");}

    // Open checkpoint header file
    int fd_h = open("myckpt.header", O_RDONLY);
    if (fd_h == -1) {perror("open");}

    int i = 0;
    int rc = -2; // any value that will not terminate the 'for' loop.
    for (i = 0; rc != EOF; i++) {
        rc = read_header(fd_h, &proc_maps[i], filename);
        if (rc == EOF)
            break;
        read_data(fd, proc_maps[i]);
    }
    close(fd);
    close(fd_h);

    // Pulling the trigger and restoring context after restoring all memory segments
    setcontext(&context);
}

int main() {
    mmap(0x6000000, 0x1000000, PROT_READ | PROT_WRITE,
            MAP_FIXED | MAP_ANONYMOUS, -1, 0);
    void *stack_ptr = 0x6000000 + 0x1000000 - 16;
    asm volatile ("mov %0,%%rsp;" : : "g" (stack_ptr) : "memory");
    do_work();

    return 0;
}