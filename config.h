#include "blocks/backlight.h"
#include "blocks/battery.h"
#include "blocks/calendar.h"
#include "blocks/capslock.h"
#include "blocks/cpu.h"
#include "blocks/mem.h"
#include "blocks/network.h"
#include "blocks/volume.h"

/* DELIMITERENDCHAR must be less than 32.
 * At max, DELIMITERENDCHAR - 1 number of clickable blocks are allowed.
 * Raw characters larger than DELIMITERENDCHAR and smaller than ' ' in ASCII
   character set can be used for signaling color change in status.
 * The character corresponding to DELIMITERENDCHAR + 1 ('\x0b' when
   DELIMITERENDCHAR is 10) will switch the active colorscheme to the first one
   defined in colors array in dwm's config.h and so on.
 * If you wish to change DELIMITERENDCHAR, don't forget to update its value in
   dwm.c and color codes in util.h. */
#define DELIMITERENDCHAR                10

static const char delimiter[] = { ' ', '|', ' ', DELIMITERENDCHAR };

#include "block.h"

/* If interval of a block is set to 0, the block will only be updated once at
   startup.
 * If interval is set to a negative value, the block will never be updated in
   the main loop.
 * Set funcc to NULL if clickability is not required for the block.
 * Set signal to 0 if both clickability and signaling are not required for the
   block.
 * Signal must be less than DELIMITERENDCHAR for clickable blocks.
 * If multiple signals are pending, then the lowest numbered one will be
   delivered first. */

/* funcu - function responsible for updating status text of the block
           (it should return the length of the text (including the terminating
            null byte), if the text was updated and 0 otherwise)
 * funcc - function responsible for handling clicks on the block */

/* 1 interval = INTERVALs seconds, INTERVALn nanoseconds */
#define INTERVALs                       1
#define INTERVALn                       0

static Block blocks[] = {
/*      funcu                   funcc                   interval        signal */
        { capslocku,            NULL,                   0,              9 },
        { backlightu,           NULL,                   2,              7 },
        { volumeu,              volumec,                0,              1 },
        { cpuu,                 cpuc,                   1,              3 },
        { memu,                 memc,                   3,              4 },
        { batteryu,             batteryc,               7,              2 },
        { calendaru,            calendarc,              5,              5 },
        { networku,             networkc,               4,              8 },
        { NULL } /* just to mark the end of the array */
};
