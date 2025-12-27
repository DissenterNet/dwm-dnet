#ifndef KEYMODE_VIM_INSERT_H
#define KEYMODE_VIM_INSERT_H

/* utils/keys/keymode_vim_insert.h */
#include "../helpers/vim_modes.h"

static const Key keymode_vim_insert[] = {
    /* In insert mode, most keys should pass through to applications */
    /* Only mode switching keys are captured */
    
    /* Escape to return to normal mode */
    { 0, XK_Escape, vim_escape_to_normal, {0}, KEYMODE_VIM_INSERT, KeyPressOnly },
    { 0, XK_Control_L, vim_escape_to_normal, {0}, KEYMODE_VIM_INSERT, KeyPressOnly },
    { 0, XK_Control_R, vim_escape_to_normal, {0}, KEYMODE_VIM_INSERT, KeyPressOnly },
    
    /* Ctrl+[ also acts as escape */
    { ControlMask, XK_bracketleft, vim_escape_to_normal, {0}, KEYMODE_VIM_INSERT, KeyPressOnly },
    
    /* Mode switching */
    { 0, XK_Tab, vim_toggle_mode, {0}, KEYMODE_VIM_INSERT, KeyPressOnly },
};

#endif /* KEYMODE_VIM_INSERT_H */
