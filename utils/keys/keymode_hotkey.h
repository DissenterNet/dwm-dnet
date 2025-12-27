#ifndef KEYMODE_HOTKEY_H
#define KEYMODE_HOTKEY_H

/* utils/keys/keymode_hotkey.h */
#include "../helpers/vim_hotkey.h"

/* Hotkey mode - works only when trigger is held or mode is toggled on */
static const Key keymode_hotkey[] = {
    /* Hotkey trigger key - press to activate, release to deactivate */
    { ControlMask, XK_F12, vim_hotkey_trigger_press, {0}, KEYMODE_ANY, KeyPressOnly },
    { ControlMask, XK_F12, vim_hotkey_trigger_release, {0}, KEYMODE_ANY, KeyReleaseOnly },
    
    /* Toggle hotkey mode on/off */
    { Mod4Mask|ShiftMask, XK_F12, vim_toggle_hotkey_mode, {0}, KEYMODE_ANY, KeyPressOnly },
    { Mod4Mask|ShiftMask, XK_h, vim_toggle_hotkey_mode, {0}, KEYMODE_ANY, KeyPressOnly },
    
    /* Navigation (arrow keys or hjkl) */
    { 0, XK_Up, vim_hotkey_up, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    { 0, XK_Down, vim_hotkey_down, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    { 0, XK_Left, vim_hotkey_left, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    { 0, XK_Right, vim_hotkey_right, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    { 0, XK_k, vim_hotkey_up, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    { 0, XK_j, vim_hotkey_down, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    { 0, XK_h, vim_hotkey_left, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    { 0, XK_l, vim_hotkey_right, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    
    /* Window management */
    { 0, XK_q, vim_hotkey_close, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    { 0, XK_x, vim_hotkey_close, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    { 0, XK_space, vim_hotkey_zoom, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    { 0, XK_Return, vim_hotkey_zoom, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    { 0, XK_z, vim_hotkey_zoom, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    
    /* Layout management */
    { 0, XK_t, vim_hotkey_tile, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    { 0, XK_m, vim_hotkey_monocle, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    { 0, XK_f, vim_hotkey_float, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    
    /* Tag navigation */
    { 0, XK_comma, vim_hotkey_prev_tag, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    { 0, XK_period, vim_hotkey_next_tag, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    { 0, XK_bracketleft, vim_hotkey_prev_tag, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    { 0, XK_bracketright, vim_hotkey_next_tag, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    
    /* Direct tag switching */
    { 0, XK_1, vim_hotkey_tag_1, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    { 0, XK_2, vim_hotkey_tag_2, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    { 0, XK_3, vim_hotkey_tag_3, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    { 0, XK_4, vim_hotkey_tag_4, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    { 0, XK_5, vim_hotkey_tag_5, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    { 0, XK_6, vim_hotkey_tag_6, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    { 0, XK_7, vim_hotkey_tag_7, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    { 0, XK_8, vim_hotkey_tag_8, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    { 0, XK_9, vim_hotkey_tag_9, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    
    /* Window resizing */
    { 0, XK_equal, vim_hotkey_increase_mfact, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    { 0, XK_minus, vim_hotkey_decrease_mfact, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    { 0, XK_plus, vim_hotkey_increase_master, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    { 0, XK_underscore, vim_hotkey_decrease_master, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    
    /* Gaps control */
    { 0, XK_g, vim_hotkey_increase_gaps, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    { ShiftMask, XK_g, vim_hotkey_decrease_gaps, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    
    /* Application launching */
    { 0, XK_e, vim_hotkey_terminal, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    { 0, XK_d, vim_hotkey_dmenu, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    { 0, XK_r, vim_hotkey_terminal, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    
    /* System control */
    { Mod4Mask, XK_q, vim_hotkey_quit, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    { Mod4Mask, XK_r, vim_hotkey_restart, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    { Mod4Mask|ShiftMask, XK_r, vim_hotkey_restart, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    
    /* Mode control */
    { 0, XK_Escape, vim_disable_hotkey_mode, {0}, KEYMODE_HOTKEY, KeyPressOnly },
    { 0, XK_BackSpace, vim_disable_hotkey_mode, {0}, KEYMODE_HOTKEY, KeyPressOnly },
};

#endif /* KEYMODE_HOTKEY_H */
