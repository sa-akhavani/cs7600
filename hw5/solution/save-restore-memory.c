#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <assert.h>

typedef void (*printf_ptr_t)();
typedef void (*fnc_ptr_p)(const char *, void *, printf_ptr_t);

void fnc(const char *string, void *fnc, printf_ptr_t myprintf) {
  // NOTE:  The original function fnc() can make a call to the address
  //        or printf().  Since we compiled this program with -fPIC
  //        (Position-Independent Code), the code can be copied to
  //        a new address.   But there is no guarantee that the call
  //        to printf() in libc.so will continue to work.  So, we pass
  //        in myprintf, a pointer to printf() in the current libc.so.
  //        (In your checkpoint program, you will restore the text
  //        segment _and_ libc.so at their original addresses.  So,
  //        you will not see this problem in your own checkpoint program.)
  (*myprintf)("%s %p\n", string, fnc);
}

# define FNC_START fnc
# define FNC_LEN 1000

void save(const char *filename, fnc_ptr_p fnc) {
  int fd = open(filename, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
  if (fd == -1) {perror("open");}
  int rc = write(fd, (void *)FNC_START, FNC_LEN);
  if (rc == -1) {
    perror("write");
    exit(1);
  }
  while (rc < FNC_LEN) {
    rc += write(fd, (char *)FNC_START + rc, FNC_LEN - rc);
  }
  assert(rc == FNC_LEN);
  close(fd);
}

void *restore(char *filename) {
  // MAP_PRIVATE corresponds to the 'p' in things like 'r-xp' in /proc/*/maps
  // MAP_ANONYMOUS means that this memory has no backing file.  So, in
  //   /proc/*/maps, there will be no filename associated with this segment.
  // We will need execute permission if we want to load code at this address.
  void *addr = mmap(NULL, FNC_LEN,
                    PROT_READ|PROT_WRITE|PROT_EXEC,
                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  printf("Memory segment of length %d mmap'ed in at address %p.\n",
         FNC_LEN, addr);
  // TO DETECT CURRENT MEMORY SEGMENTS, DO:  (gdb) info proc mappings
  // // But if you need to see the permissions for the segment, then do:
  // (gdb) info proc  # This shows the process PID
  // (gdb) shell cat /proc/PID/maps
  int fd = open(filename, O_RDONLY);
  if (fd == -1) {perror("open");}
  int rc = read(fd, addr, FNC_LEN);
  if (rc == -1) {
    perror("read");
    exit(1);
  }
  while (rc < FNC_LEN) {
    rc += read(fd, (char *)addr + rc, FNC_LEN - rc);
  }
  assert(rc == FNC_LEN);
  close(fd);
  return addr;
}

int main(int argc, char *argv[]) {
  fnc("This is the original function at address: ",
      &fnc, (printf_ptr_t)&printf);
  char *filename = "save-restore.dat";
  save(filename, &fnc);
  void *addr = restore(filename);
  // To verify that fnc and new_fnc are the same, look at their assembly:
  // (gdb) x/10i fnc
  // (gdb) x/10i new_fnc
  fnc_ptr_p new_fnc = addr;
  // Verify that new_fnc has execute permission
  // (gdb) p/x new_fnc
  // (gdb) info proc  # This shows the process PID
  // (gdb) shell cat /proc/PID/maps
  // Look at our attempt to execute new_fnc, within assembly:
  // (gdb) x/10i $pc
  // (gdb) si  # step instruction at assembly level
  (*new_fnc)("This is the restored function at a new address: ",
             new_fnc, (printf_ptr_t)&printf);
  return 0;
}
