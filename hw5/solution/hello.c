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

int main() {
    int fd = open("behx.txt", O_WRONLY);
    if (fd == -1) {perror("open");}
    char buf[1000];
    buf[sizeof(buf)-1] = '\0';
    snprintf(buf, sizeof buf, "askfdhsakfhsajkfh");
    int rc = write(fd, buf, 1000);
    
  if (rc == -1) {
    perror("write");
    exit(1);
  }
  while (rc < 1000) {
    rc += write(fd, buf, 1000);
  }
  assert(rc == 1000);
  close(fd);
}