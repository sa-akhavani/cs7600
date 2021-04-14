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

struct ckpt_segment {
  void *start;
  void *end;
  char rwxp[4];
  int is_register_context; // Is this a context from sigsetjmp? If so, ignore the start_address, end_address, and read/write/execute.
  int data_size;  // Size of DATA after this HEADER.
  char name[NAME_LEN]; // for debugging only
};

// Same process that is done in reading /proc/self/maps but this time for myckpt file
// This is the format:
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
    // printf("is_register:%d, start:%ld, end:%ld, rwxp:%c%c%c%c, size:%d, filename:%s\n", is_register_context, start, end, rwxp[0], rwxp[1],rwxp[2],rwxp[3], data_size, filename);

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

int read_data(int fd, struct ckpt_segment segment) {
    char addr[5000000] = "";
    int rc = read(fd, addr, segment.data_size);
    if (rc == -1) {
        perror("read");
        exit(1);
    }
    while (rc < segment.data_size) {
        rc += read(fd, addr + rc, segment.data_size - rc);
    }
    assert(rc == segment.data_size);

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

int main() {
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

    i = 0;
    for (i = 0; proc_maps[i].start != NULL || proc_maps[i].is_register_context; i++) {
        if (proc_maps[i].is_register_context) {
            printf("Register context data_size: %d\n", proc_maps[i].data_size);

        } else {
            printf("%s (%c%c%c%c)\n"
                "  Address-range: %p - %p\n",
                proc_maps[i].name,
                proc_maps[i].rwxp[0], proc_maps[i].rwxp[1], proc_maps[i].rwxp[2], proc_maps[i].rwxp[3],
                proc_maps[i].start, proc_maps[i].end);
        }
    }

    return 0;
}