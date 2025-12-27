#ifndef KEYMODE_NORMAL_H
#define KEYMODE_NORMAL_H

/* keys/keymode_normal.h */
static const Key keymode_normal[] = {
    /* mod, keysym, func, arg, mode, trigger */
    { MODKEY, XK_b, togglebar, {.i = 0}, KEYMODE_NORMAL, KeyPressOnly },
    { MODKEY, XK_q, killclient, {.i = 0}, KEYMODE_NORMAL, KeyPressOnly },
};

#endif /* KEYMODE_NORMAL_H */