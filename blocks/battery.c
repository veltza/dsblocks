/* Todo: add support for multiple batteries */

#include <stdio.h>
#include <string.h>
#include <dirent.h>

#include "../util.h"
#include "battery.h"

#define ICONe                   COL1 ""  COL0 /* unexpected error */
#define ICONa                   COL1 " " COL0 /* no battery */

#define ICONC0                  " "
#define ICONC1                  " "
#define ICONC2                  " "
#define ICONC3                  " "
#define ICONC4                  " "
#define ICONC5                  " "
#define ICONC6                  " "

#define ICOND0                  ""
#define ICOND1                  ""
#define ICOND2                  ""
#define ICOND3                  ""
#define ICOND4                  ""
#define ICOND5                  ""
#define ICOND6                  ""
#define ICOND7                  ""
#define ICOND8                  ""
#define ICOND9                  ""
#define ICOND10                 ""

#define ICONS(icons, cap)       icons[((LENGTH(icons)-1) * cap + 50) / 100]

/* AUTODETECT:
 * 0 = off, AC_DEVICE and BAT_DEVICE must be set manually
 * 1 = on,  AC_DEVICE and BAT_DEVICE are detected automatically
 * Note: if the autodetection doesn't detect the correct devices,
 * turn it off and set the devices manually.
 */
#define AUTODETECT              1
#define AC_DEVICE               "/sys/class/power_supply/ACAD"
#define BAT_DEVICE              "/sys/class/power_supply/BAT1"

#define CAPC                    7  /* critical level */
#define CAPL                    15 /* low level */
#define CAPM                    59 /* mid level */

#define TIMEINTERVAL            4 /* update interval for remaining time */
#define MAXRATESAMPLES          4 /* for calculating average charging and discharging rate */

#define NOTIFYID                "51000"

#define CNOTIFY(t, msg)         (char *[]){ "/usr/bin/dunstify", \
                                            "-u", "critical", \
                                            "-t", t, \
                                            "-r", NOTIFYID, \
                                            msg, NULL, NULL }

#define NNOTIFY(t, msg)         (char *[]){ "/usr/bin/dunstify", \
                                            "-t", t, \
                                            "-r", NOTIFYID, \
                                            msg, NULL, NULL }

#define RNOTIFY()               (char *[]){ "/usr/bin/dunstify", \
                                            "-C", NOTIFYID, NULL }

#define UCNOTIFY(t, msg)        uspawn(CNOTIFY(t, msg))
#define UNNOTIFY(t, msg)        uspawn(NNOTIFY(t, msg))
#define URNOTIFY()              uspawn(RNOTIFY())

static int showtime = 0;        /* show remaining time: 0 = off, 1 or greater = on */

static int ac, batdevlen;
static char batdevice[512];

int
detectbattery()
{
    char path[512], buffer[80];
    int online;

    ac = -1;
    *batdevice = '\0';

    /* try to detect automatically or fall back to manual mode */
    #if AUTODETECT
    DIR *dir;
    struct dirent *dirent;
    char *dev, devices[] = "/sys/class/power_supply";

    if ((dir = opendir(devices))) {
        while ((dirent = readdir(dir))) {
            dev = dirent->d_name;
            if (dev[0] == '.')
                continue;

            sprintf(path, "%s/%s/online", devices, dev);
            if (ac < 1 && readint(path, &online))
                ac = online;

            sprintf(path, "%s/%s/type", devices, dev);
            if (readstr(path, buffer, sizeof buffer) && strncmp("Battery", buffer, 7) == 0)
                batdevlen = sprintf(batdevice, "%s/%s/", devices, dev);
        }
        closedir(dir);
    }
    #endif

    if (ac < 0) {
        sprintf(path, "%s/online", AC_DEVICE);
        if (readint(path, &online))
            ac = online;
    }

    if (!*batdevice) {
        sprintf(path, "%s/type", BAT_DEVICE);
        if (readstr(path, buffer, sizeof buffer) && strncmp("Battery", buffer, 7) == 0)
            batdevlen = sprintf(batdevice, "%s/", BAT_DEVICE);
    }

    return (*batdevice) ? 1 : 0;
}

int
readbattery(char *property, int *var)
{
    sprintf(batdevice + batdevlen, "%s", property);
    return readint(batdevice, var);
}

int
calc_avg_rate(int rate, int ac)
{
    static int samples[MAXRATESAMPLES][2];
    static int numsamples[2];
    int i;
    ac = ac ? 1 : 0;

    if (!rate)
        return rate;

    if (numsamples[ac] < MAXRATESAMPLES)
        numsamples[ac]++;

    for (i = MAXRATESAMPLES-1; i > 0; i--)
        samples[i][ac] = samples[i-1][ac];
    samples[0][ac] = rate;

    for (i = 1; i < numsamples[ac]; i++)
        rate += samples[i][ac];

    return rate / numsamples[ac];
}

size_t
batteryu(char *str, int sigval)
{
    static char *chargeicons[] = { ICONC0, ICONC1, ICONC2, ICONC3, ICONC4,
                                   ICONC5, ICONC6 };
    static char *dischargeicons[]  = { ICOND0, ICOND1, ICOND2, ICOND3, ICOND4,
                                       ICOND5, ICOND6, ICOND7, ICOND8, ICOND9,
                                       ICOND10 };
    static int cap, oldac = -1;
    static int critwarning, lowwarning, fullwarning;
    static int cfull, cnow, ccur, rate;
    static int hr, mn, timeout;
    int err = 0;
    size_t l;

    if (!detectbattery()) {
        *str = '\0';
        return 1;
    }

    if (!readbattery("capacity", &cap))
        return SPRINTF(str, ICONa);

    /* toggle remaining time on/off (signal value 100) */
    if (sigval == 100) {
        showtime = showtime ? 0 : 1;
        timeout = 0;
    }

    if (oldac != ac)
        timeout = 0;

    /* remove all warnings when battery is being plugged in or out */
    if (oldac != ac)
        URNOTIFY();

    /* clear certain warning triggers when battery is plugged in or out */
    if (ac > 0) {
        critwarning = lowwarning = 0;
        showtime = showtime & 1;
    } else {
        fullwarning = 0;
    }

    /* send warning, if battery is critical */
    if (ac == 0 && cap <= CAPC && !critwarning) {
        UCNOTIFY("0", "Battery level is critical!");
        showtime |= 2;
        timeout = 0;
        critwarning = 1;
    }

    /* send warning, if battery is low */
    if (ac == 0 && cap <= CAPL && !critwarning && !lowwarning) {
        UNNOTIFY("0", "Battery level is low!");
        showtime |= 2;
        timeout = 0;
        lowwarning = 1;
    }

    /* send notification, if battery is fully charged */
    if (ac > 0 && cap >= 100 && !fullwarning) {
        UNNOTIFY("0", "Battery is fully charged");
        fullwarning = 1;
    }

    if (ac > 0) {
        l = snprintf(str, BLOCKLENGTH, "%s%s" COL0 " %d%%",
                (cap < 100) ? COL9 : COL10,
                ICONS(chargeicons, cap), cap);
    } else {
        l = snprintf(str, BLOCKLENGTH, "%s%s%s %d%%",
                (cap <= CAPL) ? COL6 : (cap <= CAPM) ? COL7 : COL8,
                (ac < 0) ? ICONe : ICONS(dischargeicons, cap),
                (cap < 10) ? COL6 : COL0, cap);
    }

    err |= !readbattery("current_now", &rate);
    rate = calc_avg_rate(rate, ac);

    if (showtime) {
        if (--timeout <= 0) {
            err |= !readbattery("charge_full", &cfull);
            err |= !readbattery("charge_now", &cnow);
            ccur = (ac > 0) ? cfull - cnow : cnow;
            if (oldac == ac && rate) {
                hr = ccur / rate;
                mn = (ccur * 60 / rate) - hr * 60;
            } else {
                hr = 0;
                mn = 0;
            }
            timeout = TIMEINTERVAL;
        }
        if (err || !cfull || (ac <= 0 && !rate))
            l += snprintf(str + l, BLOCKLENGTH - l, " (?)");
        else if (ac > 0 && (!ccur || cap >= 100))
            l += snprintf(str + l, BLOCKLENGTH - l, " (full)");
        else
            l += snprintf(str + l, BLOCKLENGTH - l, " (%d:%02d)", hr, mn);
    }
    l += snprintf(str + l, BLOCKLENGTH - l, COL0);

    oldac = ac;

    return l < BLOCKLENGTH ? l + 1 : BLOCKLENGTH;
}

void
batteryc(int button)
{
    switch (button) {
    case 1:
        csigself(2, 100); /* toggle remaining time on/off */
        break;
    default:
        break;
    }
}
