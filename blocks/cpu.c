#include <stdio.h>
#include <string.h>

#include "../util.h"
#include "cpu.h"

#define ICON                    COL4 " " COL0

size_t
cpuu(char *str, int sigval)
{
    char buffer[256];
    FILE *file;
    static unsigned long long int s[10];
    static unsigned long long int prevTotal, prevIdle;
    unsigned long long int currTotal = 0, currIdle = 0, usage = 0;
    unsigned long long int diffTotal, diffIdle;

    if ((file = fopen("/proc/stat", "r")) == NULL) {
        *str = '\0';
        return 1;
    }

    while (fgets(buffer, sizeof buffer - 1, file)) {
        if (strncmp(buffer, "cpu ", 4) == 0) {
            sscanf(buffer + 4, " %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                    &s[0], &s[1], &s[2], &s[3], &s[4], &s[5], &s[6], &s[7], &s[8], &s[9]);
            currTotal = s[0] + s[1] + s[2] + s[3] + s[4] + s[5] + s[6] + s[7] + s[8] + s[9];
            currIdle  = s[3];
            break;
        }
    }

    fclose(file);

    if (currTotal > prevTotal && prevTotal) {
        diffTotal = currTotal - prevTotal;
        diffIdle  = currIdle - prevIdle;
        if (diffTotal >= diffIdle)
            usage = (1000 * (diffTotal - diffIdle) / diffTotal + 5) / 10;
    }

    prevTotal = currTotal;
    prevIdle  = currIdle;
    return SPRINTF(str, ICON "%3d%%", (int)usage);
}

void
cpuc(int button)
{
    switch (button) {
    case 1:
        TERMCMD("htop", "-s", "PERCENT_CPU");
        break;
    default:
        break;
    }
}
