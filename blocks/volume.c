#include <stdio.h>

#include "../util.h"
#include "volume.h"

#define ICONsn                          COL1 "" COL0
#define ICONsm                          COL2 "" COL0
#define ICONhn                          COL1 "" COL0
#define ICONhm                          COL2 "" COL0

#define PULSEINFO                       (char *[]){ SCRIPT("pulse_info.sh"), NULL }

#define PAVUCONTROL                     (char *[]){ "/usr/bin/pavucontrol-qt", NULL }
#define NORMALIZEVOLUME                 (char *[]){ SCRIPT("pulse_normalize.sh"), NULL }
#define TOGGLEMUTE                      (char *[]){ "/usr/bin/pactl", "set-sink-mute", "@DEFAULT_SINK@", "toggle", NULL }

void
volumeu(char *str, int sigval)
{
        static char *icons[] = { ICONsn, ICONsm, ICONhn, ICONhm };
        char buf[15];
        size_t l;

        if ((l = getcmdout(PULSEINFO, buf, sizeof buf - 1))) {
                buf[l] = '\0';
                snprintf(str, BLOCKLENGTH, "%s%s", icons[buf[0] - '0'], buf + 1);
        } else
                *str = '\0';
}

void
volumec(int button)
{
        switch(button) {
                case 1:
                        cspawn(TOGGLEMUTE);
                        break;
                case 2:
                        cspawn(NORMALIZEVOLUME);
                        break;
                case 3:
                        cspawn(PAVUCONTROL);
                        break;
        }
}
