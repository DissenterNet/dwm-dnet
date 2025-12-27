#ifndef VIM_MODES_H
#define VIM_MODES_H

/* utils/helpers/vim_modes.h */
#include "../api/dwm_api.h"

/* Vim mode state management */
static int vim_submode = KEYMODE_VIM; /* Track vim submode (normal/insert/visual) */

/* Enter vim normal mode */
static void
vim_enter_normal_mode(const Arg *arg)
{
    (void)arg;
    vim_submode = KEYMODE_VIM;
    Arg a = { .i = KEYMODE_VIM };
    setkeymode(&a);
}

/* Enter vim insert mode */
static void
vim_enter_insert_mode(const Arg *arg)
{
    (void)arg;
    vim_submode = KEYMODE_VIM_INSERT;
    Arg a = { .i = KEYMODE_VIM_INSERT };
    setkeymode(&a);
}

/* Enter vim visual mode */
static void
vim_enter_visual_mode(const Arg *arg)
{
    (void)arg;
    vim_submode = KEYMODE_VIM_VISUAL;
    Arg a = { .i = KEYMODE_VIM_VISUAL };
    setkeymode(&a);
}

/* Toggle between vim modes */
static void
vim_toggle_mode(const Arg *arg)
{
    (void)arg;
    switch (vim_submode) {
        case KEYMODE_VIM:
            vim_enter_insert_mode(NULL);
            break;
        case KEYMODE_VIM_INSERT:
            vim_enter_visual_mode(NULL);
            break;
        case KEYMODE_VIM_VISUAL:
            vim_enter_normal_mode(NULL);
            break;
        default:
            vim_enter_normal_mode(NULL);
            break;
    }
}

/* Escape key handler - return to normal mode */
static void
vim_escape_to_normal(const Arg *arg)
{
    (void)arg;
    vim_enter_normal_mode(NULL);
}

#endif /* VIM_MODES_H */
