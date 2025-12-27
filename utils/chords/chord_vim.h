#ifndef CHORD_VIM_H
#define CHORD_VIM_H

/* utils/chords/chord_vim.h */
#include "../helpers/vim_motions.h"
#include "../helpers/helpers_vim.h"

/* Vim chords for enhanced navigation */

/* gg - go to top (first tag, first window) */
static Keychord chord_vim_gg = {
    .n = 2,
    .keys = { 
        { .mod = 0, .keysym = XK_g, .func = NULL, .arg = {0}, .mode = KEYMODE_VIM, .trigger = KeyPressOnly },
        { .mod = 0, .keysym = XK_g, .func = NULL, .arg = {0}, .mode = KEYMODE_VIM, .trigger = KeyPressOnly }
    },
    .func = vim_motion_gg,
    .arg = {0},
    .mode = KEYMODE_VIM
};

/* gt - go to next tag */
static Keychord chord_vim_gt = {
    .n = 2,
    .keys = { 
        { .mod = 0, .keysym = XK_g, .func = NULL, .arg = {0}, .mode = KEYMODE_VIM, .trigger = KeyPressOnly },
        { .mod = 0, .keysym = XK_t, .func = NULL, .arg = {0}, .mode = KEYMODE_VIM, .trigger = KeyPressOnly }
    },
    .func = view_next_tag,
    .arg = {0},
    .mode = KEYMODE_VIM
};

/* gT - go to previous tag */
static Keychord chord_vim_gT = {
    .n = 2,
    .keys = { 
        { .mod = 0, .keysym = XK_g, .func = NULL, .arg = {0}, .mode = KEYMODE_VIM, .trigger = KeyPressOnly },
        { .mod = ShiftMask, .keysym = XK_t, .func = NULL, .arg = {0}, .mode = KEYMODE_VIM, .trigger = KeyPressOnly }
    },
    .func = view_prev_tag,
    .arg = {0},
    .mode = KEYMODE_VIM
};

/* yy - yank/copy current window info */
static Keychord chord_vim_yy = {
    .n = 2,
    .keys = { 
        { .mod = 0, .keysym = XK_y, .func = NULL, .arg = {0}, .mode = KEYMODE_VIM, .trigger = KeyPressOnly },
        { .mod = 0, .keysym = XK_y, .func = NULL, .arg = {0}, .mode = KEYMODE_VIM, .trigger = KeyPressOnly }
    },
    .func = spawn,
    .arg = SHCMD("xclip -selection clipboard"),
    .mode = KEYMODE_VIM
};

/* dd - delete current window */
static Keychord chord_vim_dd = {
    .n = 2,
    .keys = { 
        { .mod = 0, .keysym = XK_d, .func = NULL, .arg = {0}, .mode = KEYMODE_VIM, .trigger = KeyPressOnly },
        { .mod = 0, .keysym = XK_d, .func = NULL, .arg = {0}, .mode = KEYMODE_VIM, .trigger = KeyPressOnly }
    },
    .func = killclient,
    .arg = {0},
    .mode = KEYMODE_VIM
};

/* ww - focus and zoom current window */
static Keychord chord_vim_ww = {
    .n = 2,
    .keys = { 
        { .mod = 0, .keysym = XK_w, .func = NULL, .arg = {0}, .mode = KEYMODE_VIM, .trigger = KeyPressOnly },
        { .mod = 0, .keysym = XK_w, .func = NULL, .arg = {0}, .mode = KEYMODE_VIM, .trigger = KeyPressOnly }
    },
    .func = zoom,
    .arg = {0},
    .mode = KEYMODE_VIM
};

/* G - go to bottom (last tag, last window) */
static Keychord chord_vim_G = {
    .n = 1,
    .keys = { 
        { .mod = ShiftMask, .keysym = XK_g, .func = NULL, .arg = {0}, .mode = KEYMODE_VIM, .trigger = KeyPressOnly }
    },
    .func = vim_motion_G,
    .arg = {0},
    .mode = KEYMODE_VIM
};

#endif /* CHORD_VIM_H */
