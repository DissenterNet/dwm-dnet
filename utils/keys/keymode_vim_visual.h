#ifndef KEYMODE_VIM_VISUAL_H
#define KEYMODE_VIM_VISUAL_H

/* utils/keys/keymode_vim_visual.h */
#include "../helpers/vim_modes.h"
#include "../helpers/vim_motions.h"

static const Key keymode_vim_visual[] = {
    /* Visual mode - similar to normal mode but for selection */
    
    /* Navigation (same as normal mode) */
    { 0, XK_h, focusstack, {.i = -1}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { 0, XK_l, focusstack, {.i = +1}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { 0, XK_j, focusstack, {.i = +1}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { 0, XK_k, focusstack, {.i = -1}, KEYMODE_VIM_VISUAL, KeyPressOnly },

    /* Vim motions */
    { 0, XK_w, vim_motion_w, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { 0, XK_b, vim_motion_b, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { 0, XK_e, vim_motion_e, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { 0, XK_0, vim_motion_0, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { 0, XK_dollar, vim_motion_dollar, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { 0, XK_caret, vim_motion_caret, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { 0, XK_percent, vim_motion_percent, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { 0, XK_f, vim_motion_f, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { 0, XK_F, vim_motion_F, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { 0, XK_t, vim_motion_t, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { 0, XK_T, vim_motion_T, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { 0, XK_g, vim_motion_gg, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { 0, XK_G, vim_motion_G, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { 0, XK_slash, vim_motion_slash, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { 0, XK_question, vim_motion_question, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { 0, XK_n, vim_motion_n, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { 0, XK_N, vim_motion_N, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },

    /* Visual mode operations - yank/copy */
    { 0, XK_y, spawn, {.v = (const char*[]){ "xclip", "-selection", "clipboard", NULL}}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { 0, XK_Y, spawn, {.v = (const char*[]){ "xclip", "-selection", "clipboard", NULL}}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { ShiftMask, XK_Insert, spawn, {.v = (const char*[]){ "xclip", "-selection", "clipboard", NULL}}, KEYMODE_VIM_VISUAL, KeyPressOnly },

    /* Visual mode operations - delete/cut */
    { 0, XK_d, killclient, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { 0, XK_D, killclient, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { 0, XK_x, killclient, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { 0, XK_X, killclient, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { Delete, XK_Delete, killclient, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },

    /* Visual mode operations - change */
    { 0, XK_c, killclient, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { 0, XK_C, killclient, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { 0, XK_s, killclient, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { 0, XK_S, killclient, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },

    /* Visual mode operations - substitute */
    { 0, XK_r, killclient, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { 0, XK_R, killclient, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },

    /* Visual mode operations - indentation (adapted for dwm) */
    { 0, XK_greater, vim_increase_master, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { 0, XK_less, vim_decrease_master, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },

    /* Visual mode operations - join (no direct equivalent) */
    { 0, XK_J, vim_increase_master, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },

    /* Visual mode operations - format (no direct equivalent) */
    { 0, XK_equal, vim_increase_height, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },

    /* Visual mode operations - toggle case (no direct equivalent) */
    { 0, XK_tilde, togglefloating, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { 0, XK_asciitilde, togglefloating, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },

    /* Visual mode operations - block visual mode */
    { ControlMask, XK_v, vim_enter_visual_mode, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { ControlMask, XK_V, vim_enter_visual_mode, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },

    /* Visual mode operations - line visual mode */
    { ShiftMask, XK_v, vim_enter_visual_mode, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { ShiftMask, XK_V, vim_enter_visual_mode, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },

    /* Visual mode operations - select all */
    { ControlMask, XK_a, vim_last_tab, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },

    /* Visual mode operations - invert selection (no direct equivalent) */
    { ControlMask, XK_i, vim_first_tab, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },

    /* Mode switching */
    { 0, XK_v, vim_enter_normal_mode, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { 0, XK_V, vim_enter_normal_mode, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { 0, XK_Tab, vim_toggle_mode, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { 0, XK_Escape, vim_escape_to_normal, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { ControlMask, XK_bracketleft, vim_escape_to_normal, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },

    /* Return to normal mode */
    { 0, XK_BackSpace, vim_enter_normal_mode, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },
    { 0, XK_Return, vim_enter_normal_mode, {0}, KEYMODE_VIM_VISUAL, KeyPressOnly },
};

#endif /* KEYMODE_VIM_VISUAL_H */
