#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <assert.h>


int main() {
    int fd = open("myckpt", O_RDONLY);
    if (fd == -1) {perror("open");}
    // int rc = read(fd, addr, FNC_LEN);
    // if (rc == -1) {
    //     perror("read");
    //     exit(1);
    // }
    // while (rc < FNC_LEN) {
    //     rc += read(fd, (char *)addr + rc, FNC_LEN - rc);
    // }
    // assert(rc == FNC_LEN);
    close(fd);
    return 0;
}