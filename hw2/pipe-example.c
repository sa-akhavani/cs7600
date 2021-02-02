/* For those who would like to continue to learn more about UNIX programming,
 *  see the YoLinux tutorial (e.g. Fork/Exec):
 *                    http://www.yolinux.com/TUTORIALS/ForkExecProcesses.html
 *  for the many details of UNIX programming.
 *
 * This version includes error checking.  See man pages for details.
 */

#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>    /* 'man fprintf' says we need this. */
#include <sys/wait.h> /* 'man waitpid' says we need this. */

#define PIPE_READ 0
#define PIPE_WRITE 1

int main() {
    int pipe_fd[2];       /* 'man pipe' says its arg is this type */
    int fd;               /* 'man dup' says its arg is this type */
    pid_t child1, child2; /* 'man fork' says it returns type 'pid_t' */
    char * argvChild[2];

    printf("Executing \"ls | wc\";\n");
    printf("Number of files in curr dir is (first # is answer): ");
    fflush(stdout);  /* Force printing to complete, before continuing. */

    if (-1 == pipe(pipe_fd)) {
      perror("pipe");
    }

    child1 = fork();
    /* child1 > 0 implies that we're still the parent. */
    if (child1 > 0) {
      child2 = fork();
    }

    if (child1 == 0) { /* if we are child1, do:  "ls | ..." */
        /* close  */
        if (-1 == close(STDOUT_FILENO)) {
          perror("close");
        }

        fd = dup(pipe_fd[PIPE_WRITE]); /* set up empty STDOUT to be pipe_fd[1] */
        if (-1 == fd) {
            perror("dup");
        }

        if (fd != STDOUT_FILENO) {
            fprintf(stderr, "Pipe output not at STDOUT.\n");
        }

        close(pipe_fd[PIPE_READ]); /* never used by child1 */
        close(pipe_fd[PIPE_WRITE]); /* not needed any more */

        argvChild[0] = "ls";
        argvChild[1] = NULL;

        if (-1 == execvp(argvChild[0], argvChild)) {
            perror("execvp");
        }

    } else if (child2 == 0) { /* if we are child2, do:  "... | wc" */

        /* close  */
        if (-1 == close(STDIN_FILENO)) {
            perror("close");
        }

        /* set up empty STDIN to be pipe_fd[0] */
        fd = dup(pipe_fd[PIPE_READ]);
        if (-1 == fd) {
            perror("dup");
        }

        if (fd != STDIN_FILENO) {
            fprintf(stderr, "Pipe input not at STDIN.\n");
        }

        close(pipe_fd[PIPE_READ]); /* not needed any more */
        close(pipe_fd[PIPE_WRITE]); /* never used by child2 */

        argvChild[0] = "wc";
        argvChild[1] = NULL;

        if (-1 == execvp(argvChild[0], argvChild)) {
            perror("execvp");
        }

    } else { /* else we're parent */
        int status;
        /* Close parent copy of pipes;
         * In particular, if pipe_fd[1] not closed, child2 will hang
         *   forever waiting since parent could also write to pipe_fd[1]
         */
        close(pipe_fd[PIPE_READ]);
        close(pipe_fd[PIPE_WRITE]);

        if (-1 == waitpid(child1, &status, 0)) {
            perror("waitpid");
        }

        /* Optionally, check return status.  This is what main() returns. */
        if (WIFEXITED(status) == 0) {
            printf("child1 returned w/ error code %d\n", WEXITSTATUS(status));
        }

        if (-1 == waitpid(child2, &status, 0)) {
            perror("waitpid");
        }

        /* Optionally, check return status.  This is what main() returns. */
        if (WIFEXITED(status) == 0) {
            printf("child2 returned w/ error code %d\n", WEXITSTATUS(status));
        }
    }
    return 0;  /* returning 0 from main() means success. */
}