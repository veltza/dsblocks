#include <stdio.h>
#include <string.h>

#include "../util.h"
#include "mem.h"

#define ICON                    COL5 " " COL0

/* Output format: 0 = used percentage, 1 = used mem / total mem */
static int OutputFormat = 0;

size_t
memu(char *str, int sigval)
{
    FILE *file;
    char buffer[256];
    int i;
    unsigned long long int used, usedDiff;
    unsigned long long int total = 0, free = 0, buffers = 0, cached = 0, sreclaimable = 0;

    if ((file = fopen("/proc/meminfo", "r")) == NULL) {
        *str = '\0';
        return 1;
    }

    #define READVALUE(match, var) (!var && strncmp(buffer, match, sizeof match - 1) == 0) \
                                  { sscanf(buffer + sizeof match - 1, " %llu kB", &var); i++; }

    for (i = 0; i < 5 && fgets(buffer, sizeof buffer, file);) {
        if READVALUE("MemTotal:", total)
        else if READVALUE("MemFree:", free)
        else if READVALUE("Buffers:", buffers)
        else if READVALUE("Cached:", cached)
        else if READVALUE("SReclaimable:", sreclaimable)
    }

    fclose(file);

    if (!total)
        return SPRINTF(str, ICON " ERR");

    usedDiff = free + buffers + cached + sreclaimable;
    used = (total >= usedDiff) ? total - usedDiff : total - free;

    /* toggle output format (signal value 1) */
    OutputFormat ^= (sigval == 1) ? 1 : 0;

    if (!OutputFormat) {
        return SPRINTF(str, ICON "%3d%%", (int)(1000 * used / total + 5) / 10);
    } else {
        if (used < 1024000)
            return SPRINTF(str, ICON " %dM/%.1fG", (int)(used / 1024), total / 1048576.0);
        else if (used < 1048576)
            return SPRINTF(str, ICON " %.1fG/%.1fG", 1.0, total / 1048576.0);
        else
            return SPRINTF(str, ICON " %.1fG/%.1fG", used / 1048576.0, total / 1048576.0);
    }
}

void
memc(int button)
{
    switch (button) {
    case 1:
        csigself(4, 1); /* toggle output format */
        break;
    default:
        break;
    }
}
