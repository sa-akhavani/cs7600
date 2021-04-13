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

int in_data_section = 0;

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
// ,
int read_one_line(int fd, struct ckpt_segment *ckpt_segment, char *filename) {
    unsigned long int start, end;
    char rwxp[4];
    int is_register_context;
    int data_size;
    char tmp[10];
    int tmp_stdin = dup(0);
    int rc;
    char data[1000];
    dup2(fd, 0);
    // We are reading a header section
    if (!in_data_section) {
        rc = scanf("%d|%lx|%lx|%4c|%d|%s\n,\n",
            &is_register_context, &start, &end, rwxp, &data_size, filename);
        assert(fseek(stdin, 0, SEEK_CUR) == 0);
        printf("is_register:%d, start:%ld, end:%ld, rwxp:%c%c%c%c, size:%d, filename:%s\n", is_register_context, start, end, rwxp[0], rwxp[1],rwxp[2],rwxp[3], data_size, filename);

    } else { // We are reading a data section
        rc = scanf("%[^\n]\n,\n", data);
        printf("data: --%s--\n", data);
        assert(fseek(stdin, 0, SEEK_CUR) == 0);
    }
    dup2(tmp_stdin, 0); // Restore original stdin; file offset was advanced.
    close(tmp_stdin);
    
    printf("RCC:%d\n", rc);
    if (rc == 0) {
        printf("RC IS ZEROOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\n");
        return 0;
    }
    if (rc == EOF) {
        printf("EOF!\n");
        return EOF;
    }

    if (!in_data_section) {
        ckpt_segment->is_register_context = is_register_context;
        ckpt_segment->data_size = data_size;
        if (!is_register_context) {
            ckpt_segment -> start = (void *)start;
            ckpt_segment -> end = (void *)end;
            strncpy(ckpt_segment->name, filename, NAME_LEN-1);
            memcpy(ckpt_segment->rwxp, rwxp, 4);
        }
    }

    return rc;
}

int main() {
    struct ckpt_segment proc_maps[1000];

    int fd = open("myckpt", O_RDONLY);
    char filename [100];
    if (fd == -1) {perror("open");}
    int i = 0;
    int j = 0;
    int rc = -2; // any value that will not terminate the 'for' loop.
    for (i = 0; rc != EOF; i++) {
        sleep(1);
        rc = read_one_line(fd, &proc_maps[j], filename);
        if (in_data_section) {
            in_data_section = 0;
        } else {
            in_data_section = 1;
            j++;
        }
    }
    close(fd);
    printf("closing\n");
    
    // Debugging
    i = 0;
    for (i = 0; proc_maps[i].start != NULL || proc_maps[i].is_register_context; i++) {
        printf("%d\n", i);
        if (proc_maps[i].is_register_context) {
            printf("data_size: %d\n", proc_maps[i].data_size);
        } else {
            printf("%s (%c%c%c)\n"
                "  Address-range: %p - %p\n",
                proc_maps[i].name,
                proc_maps[i].rwxp[0], proc_maps[i].rwxp[1], proc_maps[i].rwxp[2],
                proc_maps[i].start, proc_maps[i].end);
        }
    }

    return 0;
}