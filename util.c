#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"

void
cspawn(char *const *arg)
{
        setsid();
        execvp(arg[0], arg);
        perror("cspawn - execvp");
        _exit(127);
}

/* sigdsblocks from funcc */
void
csigself(int sig, int sigval)
{
        union sigval sv;

        sig += SIGRTMIN;
        sv.sival_int = sigval;
        if (sigqueue(pid, sig, sv) == -1) {
                perror("csigself - sigqueue");
                exit(1);
        }
}

/**
 * Returns a pointer to the original path (no expansion) or internal static
 * buffer when expansion occurs. The internal static buffer is overwritten
 * every time the function is called.
 */
char *
expanduser(char *path)
{
        static char fullpath[256];
        static int n;
        struct passwd *pw;
        char *home;

        if (!n) {
                if (!(home = getenv("HOME")) || home[0] == '\0') {
                        pw = getpwuid(geteuid());
                        home = pw ? pw->pw_dir : NULL;
                }
                if (home) {
                        n = snprintf(fullpath, sizeof fullpath, "%s", home);
                        n = (n < (int)(sizeof fullpath)) ? n : (int)(sizeof fullpath);
                }
                if (n <= 0) {
                        fullpath[0] = '.';
                        n = 1;
                }
        }

        if (path[0] != '~' || path[1] != '/')
                return path;

        snprintf(fullpath + n, (int)(sizeof fullpath) - n, "/%s", &path[2]);
        return fullpath;
}

/* getcmdout doesn't null terminate cmdout
 * make sure that terminal newline character is handled if the command spits one */
size_t
getcmdout(char *const *arg, char *cmdout, size_t cmdoutlen)
{
        int fd[2];
        size_t trd;
        ssize_t rd;

        if (pipe(fd) == -1) {
                perror("getcmdout - pipe");
                cleanup();
                exit(1);
        }
        switch (fork()) {
                case -1:
                        perror("getcmdout - fork");
                        close(fd[0]);
                        close(fd[1]);
                        cleanup();
                        exit(1);
                case 0:
                        close(fd[0]);
                        if (fd[1] != STDOUT_FILENO) {
                                if (dup2(fd[1], STDOUT_FILENO) != STDOUT_FILENO) {
                                        perror("getcmdout - child - dup2");
                                        close(fd[1]);
                                        exit(1);
                                }
                                close(fd[1]);
                        }
                        execvp(arg[0], arg);
                        perror("getcmdout - child - execvp");
                        _exit(127);
                default:
                        close(fd[1]);
                        trd = 0;
                        do
                                rd = read(fd[0], cmdout + trd, cmdoutlen - trd);
                        while (rd > 0 && (trd += rd) < cmdoutlen);
                        if (rd == -1) {
                                perror("getcmdout - read");
                                cleanup();
                                close(fd[0]);
                                exit(1);
                        }
                        close(fd[0]);
        }
        return trd;
}

int
readint(const char *path, int *var) {
        FILE *fp;

        if (!(fp = fopen(path, "r")))
                return 0;
        if (fscanf(fp, "%d", var) != 1) {
                fclose(fp);
                return 0;
        }
        fclose(fp);
        return 1;
}

int
readstr(const char *path, char *str, int maxlen) {
        FILE *fp;

        if (!(fp = fopen(path, "r")))
                return 0;
        if (fgets(str, maxlen, fp) == NULL) {
                fclose(fp);
                return 0;
        }
        fclose(fp);
        return 1;
}

void
uspawn(char *const *arg)
{
        switch (fork()) {
                case -1:
                        perror("uspawn - fork");
                        cleanup();
                        exit(1);
                case 0:
                        setsid();
                        execvp(arg[0], arg);
                        perror("uspawn - child - execvp");
                        _exit(127);
        }
}
