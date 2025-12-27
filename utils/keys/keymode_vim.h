#ifndef KEYMODE_VIM_H
#define KEYMODE_VIM_H

/* utils/keys/keymode_vim.h */
#include "../helpers/vim_modes.h"
#include "../helpers/vim_motions.h"
#include "../helpers/helpers_vim.h"
#include "../helpers/vim_commands.h"

static const Key keymode_vim[] = {
    /* Basic navigation - vim style */
    { 0, XK_h, focusstack, {.i = -1}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_l, focusstack, {.i = +1}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_j, focusstack, {.i = +1}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_k, focusstack, {.i = -1}, KEYMODE_VIM, KeyPressOnly },

    /* Vim motions - enhanced */
    { 0, XK_w, vim_motion_w, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_b, vim_motion_b, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_e, vim_motion_e, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_0, vim_motion_0, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_dollar, vim_motion_dollar, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_caret, vim_motion_caret, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_percent, vim_motion_percent, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_f, vim_motion_f, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_F, vim_motion_F, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_t, vim_motion_t, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_T, vim_motion_T, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_g, vim_motion_gg, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_G, vim_motion_G, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_slash, vim_motion_slash, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_question, vim_motion_question, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_n, vim_motion_n, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_N, vim_motion_N, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_period, vim_motion_repeat, {0}, KEYMODE_VIM, KeyPressOnly },

    /* Mode switching */
    { 0, XK_i, vim_enter_insert_mode, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_v, vim_enter_visual_mode, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_V, vim_enter_visual_mode, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_Tab, vim_toggle_mode, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_Escape, vim_escape_to_normal, {0}, KEYMODE_VIM, KeyPressOnly },
    { ControlMask, XK_bracketleft, vim_escape_to_normal, {0}, KEYMODE_VIM, KeyPressOnly },

    /* Window management - vim style */
    { 0, XK_space, zoom, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_Return, zoom, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_q, vim_quit, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_x, vim_quit, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_Z, vim_force_quit, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_Q, vim_force_quit, {0}, KEYMODE_VIM, KeyPressOnly },

    /* Vim-style splits */
    { 0, XK_s, vim_split_horizontal, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_S, vim_split_vertical, {0}, KEYMODE_VIM, KeyPressOnly },

    /* Tag navigation - vim buffer equivalents */
    { 0, XK_comma, view_prev_tag, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_period, view_next_tag, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_bracketleft, vim_prev_buffer, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_bracketright, vim_next_buffer, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_b, vim_prev_buffer, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_B, vim_next_buffer, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_l, vim_list_buffers, {0}, KEYMODE_VIM, KeyPressOnly },

    /* Number keys for direct tag access */
    { 0, XK_1, view, {.ui = 1 << 0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_2, view, {.ui = 1 << 1}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_3, view, {.ui = 1 << 2}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_4, view, {.ui = 1 << 3}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_5, view, {.ui = 1 << 4}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_6, view, {.ui = 1 << 5}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_7, view, {.ui = 1 << 6}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_8, view, {.ui = 1 << 7}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_9, view, {.ui = 1 << 8}, KEYMODE_VIM, KeyPressOnly },

    /* Layout management - vim style */
    { 0, XK_m, setlayout, {.v = &layouts[2]}, KEYMODE_VIM, KeyPressOnly }, /* monocle */
    { 0, XK_t, setlayout, {.v = &layouts[0]}, KEYMODE_VIM, KeyPressOnly }, /* tile */
    { 0, XK_f, togglefloating, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_F11, togglefullscreen, {0}, KEYMODE_VIM, KeyPressOnly },

    /* Window resizing - vim style */
    { 0, XK_equal, vim_increase_height, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_minus, vim_decrease_height, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_plus, vim_increase_master, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_underscore, vim_decrease_master, {0}, KEYMODE_VIM, KeyPressOnly },

    /* Tab operations - vim tab equivalents */
    { ControlMask, XK_t, vim_new_tab, {0}, KEYMODE_VIM, KeyPressOnly },
    { ControlMask, XK_w, vim_close_tab, {0}, KEYMODE_VIM, KeyPressOnly },
    { ControlMask, XK_Tab, vim_next_tab, {0}, KEYMODE_VIM, KeyPressOnly },
    { ControlMask|ShiftMask, XK_Tab, vim_prev_tab, {0}, KEYMODE_VIM, KeyPressOnly },
    { ControlMask, XK_1, vim_first_tab, {0}, KEYMODE_VIM, KeyPressOnly },
    { ControlMask, XK_9, vim_last_tab, {0}, KEYMODE_VIM, KeyPressOnly },

    /* Command mode - vim style */
    { 0, XK_colon, vim_enter_command_mode, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_slash, vim_enter_search_mode, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_question, vim_enter_backward_search_mode, {0}, KEYMODE_VIM, KeyPressOnly },

    /* Vim-style commands with colon prefix (using Control+Alt as modifier) */
    { ControlMask|Mod1Mask, XK_q, vim_quit, {0}, KEYMODE_VIM, KeyPressOnly }, /* :q */
    { ControlMask|Mod1Mask, XK_w, vim_write, {0}, KEYMODE_VIM, KeyPressOnly }, /* :w */
    { ControlMask|Mod1Mask, XK_x, vim_save_quit, {0}, KEYMODE_VIM, KeyPressOnly }, /* :x */
    { ControlMask|Mod1Mask, XK_e, vim_edit, {.v = (const char*[]){NULL}}, KEYMODE_VIM, KeyPressOnly }, /* :e */

    /* Command mode keybindings */
    { 0, XK_Return, vim_command_add_char, {.i = XK_Return}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_Escape, vim_command_add_char, {.i = XK_Escape}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_BackSpace, vim_command_add_char, {.i = XK_BackSpace}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_Tab, vim_command_tab_complete, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_Up, vim_command_history_up, {0}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_Down, vim_command_history_down, {0}, KEYMODE_VIM, KeyPressOnly },

    /* Search mode keybindings */
    { 0, XK_Return, vim_search_add_char, {.i = XK_Return}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_Escape, vim_search_add_char, {.i = XK_Escape}, KEYMODE_VIM, KeyPressOnly },
    { 0, XK_BackSpace, vim_search_add_char, {.i = XK_BackSpace}, KEYMODE_VIM, KeyPressOnly },

    /* Return to normal mode */
    { 0, XK_BackSpace, setkeymode, {.i = KEYMODE_NORMAL}, KEYMODE_VIM, KeyPressOnly },
};

#endif /* KEYMODE_VIM_H */
