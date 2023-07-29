#include "shared.h"

#define COL0                            "\x0b" /* default status color */
#define COL1                            "\x0c" /* backlight icon */
#define COL2                            "\x0d" /* volume icon - default */
#define COL3                            "\x0e" /* volume icon - muted */
#define COL4                            "\x0f" /* cpu icon */
#define COL5                            "\x10" /* mem icon */
#define COL6                            "\x11" /* battery icon - charge:  0% .. 15% */
#define COL7                            "\x12" /* battery icon - charge: 16% .. 59% */
#define COL8                            "\x13" /* battery icon - charge: 60% .. 100% */
#define COL9                            "\x14" /* battery icon - charging */
#define COL10                           "\x15" /* battery icon - full and powered */
#define COL11                           "\x16" /* calendar icon */
#define COL12                           "\x17" /* network icon - up */
#define COL13                           "\x18" /* network icon - down */

#define SCRIPT(name)                    "/home/masi/.local/bin/"name
#define TERMCMD(...)                    cspawn((char *[]){ "st", "-e", __VA_ARGS__, NULL })

#define SPRINTF(str, ...)               ({ \
                                                int len = snprintf(str, BLOCKLENGTH, __VA_ARGS__); \
                                                len < BLOCKLENGTH ? len + 1 : BLOCKLENGTH; \
                                        })

void cspawn(char *const *arg);
void csigself(int sig, int sigval);
size_t getcmdout(char *const *arg, char *cmdout, size_t cmdoutlen);
int readint(const char *path, int *var);
int readstr(const char *path, char *str, int maxlen);
void uspawn(char *const *arg);
