#include <stdio.h>
#include <dirent.h>

#include "../util.h"
#include "backlight.h"

#define ICON                            " "
#define ICONcol                         COL1

/* AUTODETECT:
 * 0 = off, BACKLIGHTDEVICE must be set manually
 * 1 = on,  BACKLIGHTDEVICE is detected automatically
 * Note: if there are multiple backlight devices, the device with the highest
 * resolution (max_brightness) is selected. If the autodetection doesn't
 * select the correct device, turn it off and select the device manually.
 */
#define AUTODETECT                      1
#define BACKLIGHTDEVICE                 "/sys/class/backlight/amdgpu_bl0"

#define NOTIFYFONT                      "<span font='Mononoki Nerd Font 15'>"
#define NOTIFYTIME                      "1500"
#define NOTIFYID                        "50000"
#define NOTIFYENABLED                   1

/*
 * Hide block after timeout.
 * If timeout is 0, then the block is always visible unless showblock is 0 too.
 */
static int timeout = 2;
static int showblock = 0;

static int maxbrightness, backlightdevlen;
static char backlightdevice[512];

#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

int
detectbacklight(void)
{
    /* only detect once */
    if (*backlightdevice)
        return 1;

    /* try to detect automatically or fall back to manual mode */
    #if AUTODETECT
    DIR *dir;
    struct dirent *dirent;
    char path[512], *dev, devices[] = "/sys/class/backlight";
    int maxbr;

    if ((dir = opendir(devices))) {
        while ((dirent = readdir(dir))) {
            dev = dirent->d_name;
            if (dev[0] == '.')
                continue;

            sprintf(path, "%s/%s/max_brightness", devices, dev);
            if (!readint(path, &maxbr))
                continue;

            if (maxbr >= maxbrightness) {
                maxbrightness = maxbr;
                backlightdevlen = sprintf(backlightdevice, "%s/%s/", devices, dev);
            }
        }
        closedir(dir);
    }
    #endif

    if (!*backlightdevice) {
        if (readint(BACKLIGHTDEVICE "/max_brightness", &maxbrightness))
            backlightdevlen = sprintf(backlightdevice, "%s/", BACKLIGHTDEVICE);
    }

    return (*backlightdevice) ? 1 : 0;
}

int
readbacklight(char *property, int *var)
{
    sprintf(backlightdevice + backlightdevlen, "%s", property);
    return readint(backlightdevice, var);
}

int
setbrightness(int percentage)
{
    FILE *f;
    char buffer[20];
    int value = CLAMP(maxbrightness * percentage / 100, 0, maxbrightness);
    size_t s = sprintf(buffer, "%d", value);

    sprintf(backlightdevice + backlightdevlen, "brightness");
    if ((f = fopen(backlightdevice, "w"))) {
        if (fwrite(buffer, 1, s, f) < s)
            value = -1; /* error */
        fclose(f);
    } else {
        value = -1; /* error */
    }
    return value;
}

size_t
backlightu(char *str, int sigval)
{
    static int init, timer;
    static int brightness, percentage;
    int brightness_new;
    #if NOTIFYENABLED
    char buf[256];
    char *cmd[] = { "/usr/bin/dunstify", "-t", NOTIFYTIME, "-r", NOTIFYID, "", buf, NULL };
    #endif

    if (!detectbacklight()) {
        *str = '\0';
        return 1;
    }

    /* first launch */
    if (!init) {
        if (!readbacklight("brightness", &brightness))
            return SPRINTF(str, ICONcol ICON COL0 " ERR1");

        percentage = CLAMP(100 * brightness / maxbrightness, 0, 100);
        percentage = (percentage + 3) / 5 * 5;  /* round to nearest 5 */
        if ((brightness = setbrightness(percentage)) < 0)
            return SPRINTF(str, ICONcol ICON COL0 " ERR2");

        init = 1;
    }

    /* routine update call and brightness is unchanged */
    readbacklight("brightness", &brightness_new);
    if (ISSPLSIGVAL(sigval) && brightness == brightness_new) {
        if (!showblock || (timeout > 0 && --timer <= 0)) {
            *str = '\0';
            timer = 0;
            return 1;
        }
        return SPRINTF(str, ICONcol ICON COL0 " %d%%", percentage);
    }

    /* actual update call or brightness is changed */
    if (!ISSPLSIGVAL(sigval))
        percentage = CLAMP(percentage + sigval, 0, 100);
    else
        percentage = CLAMP(100 * brightness_new / maxbrightness, 0, 100);

    brightness = setbrightness(percentage);

    #if NOTIFYENABLED
    snprintf(buf, sizeof buf, NOTIFYFONT " %s %d%% \n</span>", ICON, percentage);
    uspawn(cmd);
    #endif

    timer = timeout;
    if (!showblock) {
        *str = '\0';
        return 1;
    }
    return SPRINTF(str, ICONcol ICON COL0 " %d%%", percentage);
}

/*
void
backlightc(int button)
{
}
*/
