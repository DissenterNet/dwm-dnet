#ifndef KEYMODE_ALT_H
#define KEYMODE_ALT_H

/* Default mode bindings (optional overrides) */
/* Example: none yet */
static const Key keymode_normal[] = {
	{ MODKEY, XK_b, togglebar, {.i = 0}, KEYMODE_NORMAL, KeyPressOnly },
	{ MODKEY, XK_q, killclient, {.i = 0}, KEYMODE_NORMAL, KeyPressOnly },
};

#endif /* KEYMODE_ALT_H */