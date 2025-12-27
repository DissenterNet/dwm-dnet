#ifndef BUTTONS_NORMAL_H
#define BUTTONS_NORMAL_H

/* buttons/buttons_normal.h */
static const Button buttons_normal[] = {
    /* click, mask, button, func, arg, bmode */
    { ClkClientWin, 0, Button1, resizemouse, {0}, BMODE_NORMAL },
    { ClkClientWin, 0, Button2, defaultgaps, {0}, BMODE_NORMAL },
};

#endif /* BUTTONS_NORMAL_H */