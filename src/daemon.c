#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include "cache.h"

// daemonize the server process 
int daemonize(int nochdir) {
    pid_t pid;
    int fd;
    // create new process 
    pid = fork();
    if (pid == -1) {
        return -1;
    } else if (pid != 0) {
        // use _exit instead of exit to avoid flush buffers 
        _exit(EXIT_SUCCESS);
    }
    // create new session and process group 
    if (setsid() == -1) {
        return -1;
    }
    // set the working directory to the root directory 
    if (nochdir == 0) {
        if (chdir("/") == -1) {
            perror("chdir");
            return -1;
        }
    }

    if ((fd = open("/dev/null", O_RDWR, 0)) != -1) {
        if (dup2(fd, STDIN_FILENO) < 0) {
            perror("dup2 stdin");
            return -1;
        }
        if (dup2(fd, STDOUT_FILENO) < 0) {
            perror("dup2 stdout");
            return -1;
        }
        if (dup2(fd, STDERR_FILENO) < 0) {
            perror("dup2 stderr");
            return -1;
        }
        if (fd > STDERR_FILENO) {
            if (close(fd) < 0) {
                perror("close");
                return -1;
            }
        }
    }
    return 0;

}