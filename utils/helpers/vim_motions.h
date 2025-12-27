#ifndef VIM_MOTIONS_H
#define VIM_MOTIONS_H

/* utils/helpers/vim_motions.h */
#include "../api/dwm_api.h"

/* Basic vim motion helpers */

/* Move to next word - adapted for window management */
static void
vim_motion_w(const Arg *arg)
{
    (void)arg;
    /* Focus next window, similar to vim's 'w' (next word) */
    Arg a = { .i = +1 };
    focusstack(&a);
}

/* Move to previous word - adapted for window management */
static void
vim_motion_b(const Arg *arg)
{
    (void)arg;
    /* Focus previous window, similar to vim's 'b' (previous word) */
    Arg a = { .i = -1 };
    focusstack(&a);
}

/* Move to end of word - focus next window in stack */
static void
vim_motion_e(const Arg *arg)
{
    (void)arg;
    /* Focus next window, similar to vim's 'e' (end of word) */
    Arg a = { .i = +1 };
    focusstack(&a);
}

/* Go to beginning of line - focus first window */
static void
vim_motion_0(const Arg *arg)
{
    (void)arg;
    /* Focus first window in stack, similar to vim's '0' (start of line) */
    Client *c;
    int i = 0;
    for (c = selmon->clients; c && (!ISVISIBLE(c) || i < 1); c = c->next)
        if (ISVISIBLE(c))
            i++;
    
    if (c) {
        focus(c);
        restack(selmon);
    }
}

/* Go to end of line - focus last window */
static void
vim_motion_dollar(const Arg *arg)
{
    (void)arg;
    /* Focus last visible window, similar to vim's '$' (end of line) */
    Client *c, *last = NULL;
    for (c = selmon->clients; c; c = c->next)
        if (ISVISIBLE(c))
            last = c;
    
    if (last) {
        focus(last);
        restack(selmon);
    }
}

/* Go to top of file - first window on first tag */
static void
vim_motion_gg(const Arg *arg)
{
    (void)arg;
    /* Go to first tag and focus first window */
    Arg a = { .ui = 1 << 0 };
    view(&a);
    
    /* Focus first window */
    Client *c = nexttiled(selmon->clients);
    if (c) {
        focus(c);
        restack(selmon);
    }
}

/* Go to bottom of file - last window on last tag */
static void
vim_motion_G(const Arg *arg)
{
    (void)arg;
    /* Go to last tag and focus last window */
    Arg a = { .ui = 1 << (ntags - 1) };
    view(&a);
    
    /* Focus last window */
    Client *c, *last = NULL;
    for (c = selmon->clients; c; c = c->next)
        if (ISVISIBLE(c))
            last = c;
    
    if (last) {
        focus(last);
        restack(selmon);
    }
}

/* Go to specific line - jump to specific tag number */
static void
vim_motion_number_G(const Arg *arg)
{
    if (arg->i > 0 && arg->i <= ntags) {
        Arg a = { .ui = 1 << (arg->i - 1) };
        view(&a);
    }
}

/* Move to beginning of line - first window in current tag */
static void
vim_motion_caret(const Arg *arg)
{
    (void)arg;
    /* Focus first visible window */
    Client *c = nexttiled(selmon->clients);
    if (c) {
        focus(c);
        restack(selmon);
    }
}

/* Move to middle of line - middle window in stack */
static void
vim_motion_middle(const Arg *arg)
{
    (void)arg;
    /* Count visible windows and go to middle */
    Client *c;
    int count = 0;
    int target = 0;
    
    /* Count visible windows */
    for (c = selmon->clients; c; c = c->next)
        if (ISVISIBLE(c))
            count++;
    
    if (count == 0) return;
    
    target = count / 2;
    
    /* Go to middle window */
    count = 0;
    for (c = selmon->clients; c; c = c->next) {
        if (ISVISIBLE(c)) {
            if (count == target) {
                focus(c);
                restack(selmon);
                break;
            }
            count++;
        }
    }
}

/* Repeat last motion - store and repeat last motion */
static void (*last_motion)(const Arg *) = NULL;
static Arg last_motion_arg = {0};

static void
vim_motion_repeat(const Arg *arg)
{
    (void)arg;
    if (last_motion) {
        last_motion(&last_motion_arg);
    }
}

/* Set motion for repetition */
static void
vim_set_motion(void (*motion)(const Arg *), const Arg *arg)
{
    last_motion = motion;
    if (arg)
        last_motion_arg = *arg;
    else
        last_motion_arg = (Arg){0};
}

/* Enhanced word motion with count support */
static void
vim_motion_w_count(const Arg *arg)
{
    int count = arg->i > 0 ? arg->i : 1;
    for (int i = 0; i < count; i++) {
        Arg a = { .i = +1 };
        focusstack(&a);
    }
    vim_set_motion(vim_motion_w_count, arg);
}

static void
vim_motion_b_count(const Arg *arg)
{
    int count = arg->i > 0 ? arg->i : 1;
    for (int i = 0; i < count; i++) {
        Arg a = { .i = -1 };
        focusstack(&a);
    }
    vim_set_motion(vim_motion_b_count, arg);
}

/* Search forward for window - simplified find */
static void
vim_motion_slash(const Arg *arg)
{
    (void)arg;
    /* For now, just cycle to next tag */
    view_next_tag(NULL);
}

/* Search backward for window */
static void
vim_motion_question(const Arg *arg)
{
    (void)arg;
    /* For now, just cycle to previous tag */
    view_prev_tag(NULL);
}

/* Next matching window - like n in vim */
static void
vim_motion_n(const Arg *arg)
{
    (void)arg;
    /* Cycle to next window in current tag */
    Arg a = { .i = +1 };
    focusstack(&a);
}

/* Previous matching window - like N in vim */
static void
vim_motion_N(const Arg *arg)
{
    (void)arg;
    /* Cycle to previous window in current tag */
    Arg a = { .i = -1 };
    focusstack(&a);
}

/* Jump to matching window (like vim's % ) */
static void
vim_motion_percent(const Arg *arg)
{
    (void)arg;
    /* Toggle between master and stack */
    zoom(NULL);
}

/* Find window forward - simplified version of vim's 'f' */
static void
vim_motion_f(const Arg *arg)
{
    (void)arg;
    /* For now, just focus next window */
    Arg a = { .i = +1 };
    focusstack(&a);
}

/* Find window backward - simplified version of vim's 'F' */
static void
vim_motion_F(const Arg *arg)
{
    (void)arg;
    /* For now, just focus previous window */
    Arg a = { .i = -1 };
    focusstack(&a);
}

/* Till forward - simplified version of vim's 't' */
static void
vim_motion_t(const Arg *arg)
{
    (void)arg;
    /* For now, focus next window */
    Arg a = { .i = +1 };
    focusstack(&a);
}

/* Till backward - simplified version of vim's 'T' */
static void
vim_motion_T(const Arg *arg)
{
    (void)arg;
    /* For now, focus previous window */
    Arg a = { .i = -1 };
    focusstack(&a);
}

#endif /* VIM_MOTIONS_H */
