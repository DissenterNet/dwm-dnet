#ifndef HELPERS_VIM_H
#define HELPERS_VIM_H

/* utils/helpers/helpers_vim.h */
#include "../api/dwm_api.h"

/* helper: view next tag */
static void
view_next_tag(const Arg *arg)
{
   unsigned int cur = selmon->tagset[selmon->seltags];
    int idx = 0;
    if (cur)
        idx = __builtin_ctz(cur); /* first set bit */
    int next = (idx + 1) % ntags;
    Arg a = { .ui = 1 << next };
    view(&a);
}

/* helper: view previous tag */
static void
view_prev_tag(const Arg *arg)
{
     unsigned int cur = selmon->tagset[selmon->seltags];
     int idx = 0;
     if (cur)
         idx = __builtin_ctz(cur);
     int prev = (idx - 1 + ntags) % ntags;
     Arg a = { .ui = 1 << prev };
     view(&a);
}

/* Toggle helper (example) */
static void
toggle_to_vim(const Arg *arg)
{
    Arg a = { .i = KEYMODE_VIM };
    setkeymode(&a);
}

/* Vim-style window operations */

/* Split window horizontally - vim :split equivalent */
static void
vim_split_horizontal(const Arg *arg)
{
    (void)arg;
    /* In dwm, we can't really split windows, but we can spawn a new terminal */
    Arg a = { .v = termcmd };
    spawn(&a);
}

/* Split window vertically - vim :vsplit equivalent */
static void
vim_split_vertical(const Arg *arg)
{
    (void)arg;
    /* Similar to horizontal split for dwm */
    Arg a = { .v = termcmd };
    spawn(&a);
}

/* Close current window - vim :q equivalent */
static void
vim_quit(const Arg *arg)
{
    (void)arg;
    killclient(NULL);
}

/* Force quit without saving - vim :q! equivalent */
static void
vim_force_quit(const Arg *arg)
{
    (void)arg;
    killclient(NULL);
}

/* Save and quit - vim :wq equivalent (in dwm context, just quit) */
static void
vim_save_quit(const Arg *arg)
{
    (void)arg;
    killclient(NULL);
}

/* Write file - vim :w equivalent (no-op in dwm context) */
static void
vim_write(const Arg *arg)
{
    (void)arg;
    /* No direct equivalent in dwm - could trigger a save action in focused app */
}

/* Force write - vim :w! equivalent */
static void
vim_force_write(const Arg *arg)
{
    (void)arg;
    /* No direct equivalent in dwm */
}

/* Edit file - vim :e equivalent */
static void
vim_edit(const Arg *arg)
{
    if (arg->v) {
        const char *cmd[] = { "st", "-e", "nvim", NULL };
        if (arg->v && *(const char**)arg->v) {
            cmd[2] = *(const char**)arg->v;
        }
        Arg a = { .v = cmd };
        spawn(&a);
    }
}

/* Search and replace - vim :%s equivalent (no-op in dwm) */
static void
vim_search_replace(const Arg *arg)
{
    (void)arg;
    /* No direct equivalent in window manager context */
}

/* Go to line number - vim :<number> equivalent */
static void
vim_goto_line(const Arg *arg)
{
    if (arg->i > 0 && arg->i <= ntags) {
        Arg a = { .ui = 1 << (arg->i - 1) };
        view(&a);
    }
}

/* Tag operations - vim buffer equivalents */

/* Next buffer/tag - vim :bnext equivalent */
static void
vim_next_buffer(const Arg *arg)
{
    (void)arg;
    view_next_tag(NULL);
}

/* Previous buffer/tag - vim :bprev equivalent */
static void
vim_prev_buffer(const Arg *arg)
{
    (void)arg;
    view_prev_tag(NULL);
}

/* List buffers/tags - vim :ls equivalent */
static void
vim_list_buffers(const Arg *arg)
{
    (void)arg;
    /* Could spawn a script to show current tags and windows */
    Arg a = { .v = (const char*[]){ "st", "-e", "sh", "-c", "echo 'Current tags:'; for i in {1..9}; do echo -n \"$i: \"; test $((1 << (i-1))) -eq $(xprop -root _NET_CURRENT_DESKTOP | cut -d' ' -f3) 2>/dev/null && echo '[current]' || echo; done", NULL } };
    spawn(&a);
}

/* Window resizing - vim equivalent of window resizing */

/* Increase window height - vim :resize +N equivalent */
static void
vim_increase_height(const Arg *arg)
{
    Arg a = { .f = +0.05 };
    setmfact(&a);
}

/* Decrease window height - vim :resize -N equivalent */
static void
vim_decrease_height(const Arg *arg)
{
    Arg a = { .f = -0.05 };
    setmfact(&a);
}

/* Increase master window count - vim equivalent */
static void
vim_increase_master(const Arg *arg)
{
    Arg a = { .i = +1 };
    incnmaster(&a);
}

/* Decrease master window count - vim equivalent */
static void
vim_decrease_master(const Arg *arg)
{
    Arg a = { .i = -1 };
    incnmaster(&a);
}

/* Tab operations - vim tab equivalents */

/* New tab - vim :tabnew equivalent */
static void
vim_new_tab(const Arg *arg)
{
    (void)arg;
    /* Find first empty tag and switch to it */
    for (int i = 0; i < ntags; i++) {
        Arg a = { .ui = 1 << i };
        view(&a);
        /* Check if tag is empty */
        int empty = 1;
        for (Client *c = selmon->clients; c; c = c->next) {
            if (c->tags & (1 << i)) {
                empty = 0;
                break;
            }
        }
        if (empty) {
            Arg spawn_arg = { .v = termcmd };
            spawn(&spawn_arg);
            break;
        }
    }
}

/* Close tab - vim :tabclose equivalent */
static void
vim_close_tab(const Arg *arg)
{
    (void)arg;
    /* Kill all windows on current tag */
    Client *c, *next;
    for (c = selmon->clients; c; c = next) {
        next = c->next;
        if (ISVISIBLE(c)) {
            killclient(NULL);
        }
    }
}

/* Next tab - vim :tabnext equivalent */
static void
vim_next_tab(const Arg *arg)
{
    (void)arg;
    view_next_tag(NULL);
}

/* Previous tab - vim :tabprev equivalent */
static void
vim_prev_tab(const Arg *arg)
{
    (void)arg;
    view_prev_tag(NULL);
}

/* First tab - vim :tabfirst equivalent */
static void
vim_first_tab(const Arg *arg)
{
    (void)arg;
    Arg a = { .ui = 1 << 0 };
    view(&a);
}

/* Last tab - vim :tablast equivalent */
static void
vim_last_tab(const Arg *arg)
{
    (void)arg;
    Arg a = { .ui = 1 << (ntags - 1) };
    view(&a);
}

#endif /* HELPERS_VIM_H */
