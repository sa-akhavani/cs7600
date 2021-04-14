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
  int is_register_context; // Is this a context? If so, ignore the start_address, end_address, and read/write/execute.
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
    // Read Context if we see is_register_context is 1
    if (segment.is_register_context) {
        read(fd, &context, segment.data_size);
        
    } else { // Restore memory data segment if is_register_context is 0

        // Setting flags and prot for mmap
        // There is no need to set these so I commented them out
        /*
        int flags = 0;
        if (!strcmp(segment.name, "ANONYMOUS_SEGMENT")) {
            flags = flags | MAP_ANONYMOUS;
        }
        int prot = PROT_READ;
        if (segment.rwxp[1] == 'w')
            prot = prot | PROT_WRITE;
        if (segment.rwxp[2] == 'x')
            prot = prot | PROT_EXEC;
        if (segment.rwxp[3] == 'p')
            flags = flags | MAP_PRIVATE;

        // Calling mmap after setting prot and flags
        void *addr = mmap(segment.start, segment.data_size,
                            prot | PROT_EXEC, flags, -1, 0);
        */
       

        // MAP_PRIVATE corresponds to the 'p' in things like 'r-xp' in /proc/*/maps
        // MAP_ANONYMOUS means that this memory has no backing file.  So, in
        //   /proc/*/maps, there will be no filename associated with this segment.
        // We will need execute permission if we want to load code at this address.
        void *addr = mmap(segment.start, segment.data_size,
                            PROT_READ|PROT_WRITE|PROT_EXEC,
                            MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);        
        
        int rc = read(fd, addr, segment.data_size);
        if (rc == -1) {
            perror("read");
            exit(1);
        }
        while (rc < segment.data_size) {
            rc += read(fd, (char *)addr + rc, segment.data_size - rc);
        }
        assert(rc == segment.data_size);
        if (rc == EOF)
            return EOF;
        // printf("Memory segment of length %d mmap'ed in at address %p.\n", segment.data_size, addr);
    }

    // Read newline in order to reach next data segment in the next call to this function
    char junk[1];
    int new_rc = read(fd, junk, 1);
    if (new_rc == -1) {
        perror("read newline");
        exit(1);
    }
    assert(new_rc == 1);
    return 1;
}


void do_work() {
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

// growing the stack with many call frames, so that the latest call frame will be at an address with no conflict.
void recursive(int levels) {
    if (levels > 0) {
        recursive(levels - 1);
    } else {
        do_work();
    }
}

int main() {
    // mmap(0x6000000, 0x1000000, PROT_READ | PROT_WRITE,
    //         MAP_FIXED | MAP_ANONYMOUS, -1, 0);
    // void *stack_ptr = 0x6000000 + 0x1000000 - 16;
    // asm volatile ("mov %0,%%rsp;" : : "g" (stack_ptr) : "memory");

    // For some reason the mmap method did not work properly so I used the recursive way to grow stack
    recursive(1000);
    return 0;
}