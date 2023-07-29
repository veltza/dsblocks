#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <X11/XKBlib.h>

#include "../util.h"
#include "capslock.h"

#define NOTIFYFONT                      "<span font='JetBrainsMono Nerd Font 12'>"
#define NOTIFYLOCKTEXT                  " Caps Lock On"
#define NOTIFYUNLOCKTEXT                " Caps Lock Off"
#define NOTIFYTIME                      "2000"
#define NOTIFYID                        "52000"
#define NOTIFYENABLED                   1

size_t
capslocku(char *str, int sigval)
{
    Display *display;
    unsigned int state;
    #if NOTIFYENABLED
    char buf[256];
    char *cmd[] = { "/usr/bin/dunstify", "-t", NOTIFYTIME, "-r", NOTIFYID,
                    "--icon=no-icon", "", buf, NULL };
    #endif

    *str = '\0';

    display = XOpenDisplay(getenv("DISPLAY"));
    if (!display) {
        return 1;
    }

    if (XkbGetIndicatorState(display, XkbUseCoreKbd, &state) != Success) {
        XCloseDisplay(display);
        return 1;
    }
    state &= 1;
    XCloseDisplay(display);

    #if NOTIFYENABLED
    if (!ISSPLSIGVAL(sigval)) {
        snprintf(buf, sizeof buf, NOTIFYFONT " %s \n</span>",
                state ? NOTIFYLOCKTEXT : NOTIFYUNLOCKTEXT);
        uspawn(cmd);
    }
    #endif

    return state ? SPRINTF(str, COL0 "CAPS" COL0) : 1;
}

/*
void
capslockc(int button)
{
}
*/
