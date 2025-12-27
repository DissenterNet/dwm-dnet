#ifndef VIM_HOTKEY_H
#define VIM_HOTKEY_H

/* utils/helpers/vim_hotkey.h */
#include "../api/dwm_api.h"

/* Hotkey mode state management */
static int hotkey_mode_active = 0;
static int hotkey_trigger_key_pressed = 0;
static KeySym hotkey_trigger_sym = XK_F12; /* Default trigger key */
static unsigned int hotkey_trigger_mod = ControlMask; /* Default trigger modifier */

/* Toggle hotkey mode on/off */
static void
vim_toggle_hotkey_mode(const Arg *arg)
{
    (void)arg;
    hotkey_mode_active = !hotkey_mode_active;
}

/* Enable hotkey mode */
static void
vim_enable_hotkey_mode(const Arg *arg)
{
    (void)arg;
    hotkey_mode_active = 1;
}

/* Disable hotkey mode */
static void
vim_disable_hotkey_mode(const Arg *arg)
{
    (void)arg;
    hotkey_mode_active = 0;
    hotkey_trigger_key_pressed = 0;
}

/* Set hotkey trigger key */
static void
vim_set_hotkey_trigger(const Arg *arg)
{
    if (arg && arg->i) {
        hotkey_trigger_sym = arg->i;
    }
}

/* Handle hotkey trigger press */
static void
vim_hotkey_trigger_press(const Arg *arg)
{
    (void)arg;
    hotkey_trigger_key_pressed = 1;
    hotkey_mode_active = 1;
}

/* Handle hotkey trigger release */
static void
vim_hotkey_trigger_release(const Arg *arg)
{
    (void)arg;
    hotkey_trigger_key_pressed = 0;
    hotkey_mode_active = 0;
}

/* Check if hotkey mode is active */
static int
vim_is_hotkey_active(void)
{
    return hotkey_mode_active || hotkey_trigger_key_pressed;
}

/* Check if hotkey trigger is currently pressed */
static int
vim_is_hotkey_trigger_pressed(void)
{
    return hotkey_trigger_key_pressed;
}

/* Hotkey mode functions - these only work when hotkey mode is active */

/* Hotkey navigation */
static void
vim_hotkey_up(const Arg *arg)
{
    if (!vim_is_hotkey_active()) return;
    Arg a = { .i = -1 };
    focusstack(&a);
}

static void
vim_hotkey_down(const Arg *arg)
{
    if (!vim_is_hotkey_active()) return;
    Arg a = { .i = +1 };
    focusstack(&a);
}

static void
vim_hotkey_left(const Arg *arg)
{
    if (!vim_is_hotkey_active()) return;
    Arg a = { .i = -1 };
    focusstack(&a);
}

static void
vim_hotkey_right(const Arg *arg)
{
    if (!vim_is_hotkey_active()) return;
    Arg a = { .i = +1 };
    focusstack(&a);
}

/* Hotkey window management */
static void
vim_hotkey_close(const Arg *arg)
{
    if (!vim_is_hotkey_active()) return;
    killclient(NULL);
}

static void
vim_hotkey_zoom(const Arg *arg)
{
    if (!vim_is_hotkey_active()) return;
    zoom(NULL);
}

/* Hotkey layout management */
static void
vim_hotkey_tile(const Arg *arg)
{
    if (!vim_is_hotkey_active()) return;
    Arg a = { .v = &layouts[0] };
    setlayout(&a);
}

static void
vim_hotkey_monocle(const Arg *arg)
{
    if (!vim_is_hotkey_active()) return;
    Arg a = { .v = &layouts[2] };
    setlayout(&a);
}

static void
vim_hotkey_float(const Arg *arg)
{
    if (!vim_is_hotkey_active()) return;
    togglefloating(NULL);
}

/* Hotkey tag navigation */
static void
vim_hotkey_next_tag(const Arg *arg)
{
    if (!vim_is_hotkey_active()) return;
    view_next_tag(NULL);
}

static void
vim_hotkey_prev_tag(const Arg *arg)
{
    if (!vim_is_hotkey_active()) return;
    view_prev_tag(NULL);
}

/* Hotkey tag switching (1-9) */
static void
vim_hotkey_tag_1(const Arg *arg)
{
    if (!vim_is_hotkey_active()) return;
    Arg a = { .ui = 1 << 0 };
    view(&a);
}

static void
vim_hotkey_tag_2(const Arg *arg)
{
    if (!vim_is_hotkey_active()) return;
    Arg a = { .ui = 1 << 1 };
    view(&a);
}

static void
vim_hotkey_tag_3(const Arg *arg)
{
    if (!vim_is_hotkey_active()) return;
    Arg a = { .ui = 1 << 2 };
    view(&a);
}

static void
vim_hotkey_tag_4(const Arg *arg)
{
    if (!vim_is_hotkey_active()) return;
    Arg a = { .ui = 1 << 3 };
    view(&a);
}

static void
vim_hotkey_tag_5(const Arg *arg)
{
    if (!vim_is_hotkey_active()) return;
    Arg a = { .ui = 1 << 4 };
    view(&a);
}

static void
vim_hotkey_tag_6(const Arg *arg)
{
    if (!vim_is_hotkey_active()) return;
    Arg a = { .ui = 1 << 5 };
    view(&a);
}

static void
vim_hotkey_tag_7(const Arg *arg)
{
    if (!vim_is_hotkey_active()) return;
    Arg a = { .ui = 1 << 6 };
    view(&a);
}

static void
vim_hotkey_tag_8(const Arg *arg)
{
    if (!vim_is_hotkey_active()) return;
    Arg a = { .ui = 1 << 7 };
    view(&a);
}

static void
vim_hotkey_tag_9(const Arg *arg)
{
    if (!vim_is_hotkey_active()) return;
    Arg a = { .ui = 1 << 8 };
    view(&a);
}

/* Hotkey resizing */
static void
vim_hotkey_increase_master(const Arg *arg)
{
    if (!vim_is_hotkey_active()) return;
    Arg a = { .i = +1 };
    incnmaster(&a);
}

static void
vim_hotkey_decrease_master(const Arg *arg)
{
    if (!vim_is_hotkey_active()) return;
    Arg a = { .i = -1 };
    incnmaster(&a);
}

static void
vim_hotkey_increase_mfact(const Arg *arg)
{
    if (!vim_is_hotkey_active()) return;
    Arg a = { .f = +0.05 };
    setmfact(&a);
}

static void
vim_hotkey_decrease_mfact(const Arg *arg)
{
    if (!vim_is_hotkey_active()) return;
    Arg a = { .f = -0.05 };
    setmfact(&a);
}

/* Hotkey gaps control */
static void
vim_hotkey_increase_gaps(const Arg *arg)
{
    if (!vim_is_hotkey_active()) return;
    Arg a = { .i = +3 };
    incrgaps(&a);
}

static void
vim_hotkey_decrease_gaps(const Arg *arg)
{
    if (!vim_is_hotkey_active()) return;
    Arg a = { .i = -3 };
    incrgaps(&a);
}

/* Hotkey application launching */
static void
vim_hotkey_terminal(const Arg *arg)
{
    if (!vim_is_hotkey_active()) return;
    Arg a = { .v = termcmd };
    spawn(&a);
}

static void
vim_hotkey_dmenu(const Arg *arg)
{
    if (!vim_is_hotkey_active()) return;
    Arg a = { .v = dmenucmd };
    spawn(&a);
}

/* Hotkey system control */
static void
vim_hotkey_quit(const Arg *arg)
{
    if (!vim_is_hotkey_active()) return;
    Arg a = { .i = 0 };
    quit(&a);
}

static void
vim_hotkey_restart(const Arg *arg)
{
    if (!vim_is_hotkey_active()) return;
    Arg a = { .i = 1 };
    quit(&a);
}

#endif /* VIM_HOTKEY_H */
