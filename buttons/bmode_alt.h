#ifndef BUTTONS_ALT_H
#define BUTTONS_ALT_H

/* buttons/buttons_alt.h */
static const Button buttons_alt[] = {
    /* click, mask, button, func, arg, bmode */
    { ClkClientWin, MODKEY, Button1, zoom, {0}, BMODE_NORMAL },
    { ClkClientWin, MODKEY, Button3, resizemouse, {0}, BMODE_NORMAL },
    { ClkClientWin, MODKEY, Button4, view, {.ui = 1 << 1}, BMODE_NORMAL },
    { ClkClientWin, MODKEY, Button5, view, {.ui = 1 << 0}, BMODE_NORMAL },
};


#endif /* BUTTONS_ALT_H */
