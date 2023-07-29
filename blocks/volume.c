#include <stdio.h>

#include "../util.h"
#include "volume.h"

#define ICONmu                          " "
#define ICONlo                          " "
#define ICONme                          " "
#define ICONhi                          " "

#define VOLlo                           20
#define VOLhi                           75

#define NOTIFYFONT                      "<span font='Mononoki Nerd Font 15'>"
#define NOTIFYTIME                      "1500"
#define NOTIFYID                        "50000"
#define NOTIFYENABLED                   1

#define PULSEINFO                       (char *[]){ SCRIPT("dwm-pulseinfo"), NULL }
#define VOLUMECONTROL                   (char *[]){ "pavucontrol", NULL }

#define BOOSTBIAS                       1000
#define MUTECMD                         -1000

#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

size_t
volumeu(char *str, int sigval)
{
    static int init, prvMute, prvVolumeL, prvVolumeR;
    char buf[256], **cmd, *icon;
    int curMute, curVolumeL, curVolumeR, vol;
    size_t l;

    if (!(l = getcmdout(PULSEINFO, buf, sizeof buf - 1))) {
        *str++ = '\0';
        return 1;
    }
    buf[l] = '\0';
    sscanf(buf, "%d %d %d", &curMute, &curVolumeL, &curVolumeR);

    /* volume control */
    if (!ISSPLSIGVAL(sigval)) {
        if (sigval == MUTECMD) {
            curMute ^= 1;
        } else if (sigval > 100) {
            curVolumeL += sigval - BOOSTBIAS;
            curVolumeR += sigval - BOOSTBIAS;
            curVolumeL = CLAMP(curVolumeL, 0, 150);
            curVolumeR = CLAMP(curVolumeR, 0, 150);
            if (sigval > BOOSTBIAS)
                curMute = 0;
        } else {
            curVolumeL += sigval;
            curVolumeR += sigval;
            curVolumeL = CLAMP(curVolumeL, 0, 100);
            curVolumeR = CLAMP(curVolumeR, 0, 100);
            if (sigval > 0)
                curMute = 0;
        }
    }

    vol = (curVolumeL > curVolumeR) ? curVolumeL : curVolumeR;
    icon = curMute ? ICONmu
                   : (vol <= VOLlo) ? ICONlo : (vol < VOLhi) ? ICONme : ICONhi;

    if (init) {
        #if NOTIFYENABLED
        if (!ISSPLSIGVAL(sigval)) {
            cmd = (char *[]) { "/usr/bin/dunstify", "-t", NOTIFYTIME, "-r", NOTIFYID,
                               "--icon=no-icon", "", buf, NULL };
            if (curMute && !prvMute)
                snprintf(buf, sizeof buf, NOTIFYFONT " %s Mute \n</span>", icon);
            else if (curVolumeL == curVolumeR)
                snprintf(buf, sizeof buf, NOTIFYFONT " %s %d%% \n</span>", icon, curVolumeL);
            else
                snprintf(buf, sizeof buf, NOTIFYFONT " %s L: %d%% R: %d%% \n</span>", icon, curVolumeL, curVolumeR);
            uspawn(cmd);
        }
        #endif

        if (prvVolumeL != curVolumeL || prvVolumeR != curVolumeR) {
            cmd = (char *[]) { "pactl", "set-sink-volume", "@DEFAULT_SINK@",
                               buf, buf+5, NULL };
            snprintf(buf,   5, "%d%%", curVolumeL);
            snprintf(buf+5, 5, "%d%%", curVolumeR);
            uspawn(cmd);
        }

        if (prvMute != curMute) {
            cmd = (char *[]) { "pactl", "set-sink-mute", "@DEFAULT_SINK@",
                               buf, NULL };
            snprintf(buf, 2, "%d", curMute & 1);
            uspawn(cmd);
        }
    }

    init = 1;
    prvVolumeL = curVolumeL;
    prvVolumeR = curVolumeR;
    prvMute = curMute;

    if (curMute)
        return SPRINTF(str, COL3 "%s" COL0, icon);
    else if (curVolumeL == curVolumeR)
        return SPRINTF(str, COL2 "%s" COL0 " %d%%", icon, curVolumeL);
    else
        return SPRINTF(str, COL2 "%s" COL0 " %d%% %d%%", icon, curVolumeL, curVolumeR);
}

void
volumec(int button)
{
    switch(button) {
    case 1:
        cspawn(VOLUMECONTROL);
        break;
    case 2:
        csigself(1, MUTECMD);   /* toggle mute */
        break;
    case 4:
        csigself(1, +5);        /* volume up */
        break;
    case 5:
        csigself(1, -5);        /* volume dowm */
        break;
    default:
        break;
    }
}
