/* keymode-normal.h - normal keymode definitions */

#ifndef KEYMODE_NORMAL_H
#define KEYMODE_NORMAL_H

/* Key mode enum */
enum {
	KEYMODE_ANY = -1,
	KEYMODE_NORMAL = 0,
	KEYMODE_ALT = 1,
	KEYMODE_VIM = 2,
	KEYMODE_TESTING = 3,
	KEYMODE_ADMIN = 4
};

/* Key functions */
static void setkeymode(const Arg *arg);
static void cyckeymode(const Arg *arg);

/* Key bindings for normal mode */
static const Key keys_normal[] = {
	/* modifier keysym function argument */
	{ MODKEY, XK_Tab, focusstack, {.i = +1 }, KEYMODE_NORMAL, KeyPressOnly },
	{ MODKEY, XK_Tab, focusstack, {.i = -1 }, KEYMODE_NORMAL, KeyPressOnly },
	{ MODKEY, XK_x, killclient, {0}, KEYMODE_NORMAL, KeyPressOnly },
	{ MODKEY, XK_q, quit, {0}, KEYMODE_NORMAL, KeyPressOnly },
	{ MODKEY, XK_Return, spawn, {.v = termcmd }, KEYMODE_NORMAL, KeyPressOnly },
	{ MODKEY, XK_space, setlayout, {.v = &layouts[0]}, KEYMODE_NORMAL, KeyPressOnly },
	{ MODKEY, XK_space, setlayout, {.v = &layouts[1]}, KEYMODE_NORMAL, KeyPressOnly },
	{ MODKEY, XK_space, setlayout, {.v = &layouts[2]}, KEYMODE_NORMAL, KeyPressOnly },
	{ MODKEY, XK_space, setlayout, {.v = &layouts[3]}, KEYMODE_NORMAL, KeyPressOnly },
	{ MODKEY, XK_space, setlayout, {.v = &layouts[4]}, KEYMODE_NORMAL, KeyPressOnly },
};

#endif
