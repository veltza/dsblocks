#include <stdio.h>
#include <time.h>

#include "../util.h"
#include "calendar.h"

#define ICON                    COL11 " " COL0

/* Output format: 0 = time, 1 = date and time */
static int OutputFormat = 0;

size_t
calendaru(char *str, int sigval)
{
    static char *days[] = { "su", "ma", "ti", "ke", "to", "pe", "la" };
    static char *months[] = { "tam", "hel", "maa", "huh", "tou", "kes", "hei", "elo", "syy", "lok", "mar", "jou" };
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    /* toggle output format (signal value 1) */
    OutputFormat ^= (sigval == 1) ? 1 : 0;

    if (!OutputFormat)
        return SPRINTF(str, ICON " %d:%02d", tm.tm_hour, tm.tm_min);
    else
        return SPRINTF(str, ICON " %s %d.%s %d:%02d",
                days[tm.tm_wday], tm.tm_mday, months[tm.tm_mon], tm.tm_hour, tm.tm_min);
}

void
calendarc(int button)
{
    switch (button) {
    case 1:
        csigself(5, 1); /* toggle output format */
        break;
    default:
        break;
    }
}
