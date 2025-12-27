#ifndef BUTTONS_RESIZE_H
#define BUTTONS_RESIZE_H
/*
 * buttons/buttonmode-resize.h
 *
 * A "resize" button mode: when the button mode is set to ButtonModeResize,
 * these button bindings are used by grabbuttons().
 */

static const Button resizebuttons[] = {
    /* Left-click on client -> start resizemouse (no modifier) */
    { ClkClientWin, 0, Button1, resizemouse, {0}, ButtonModeResize },

    /* Middle-click -> reset gaps (example) */
    { ClkClientWin, 0, Button2, defaultgaps, {0}, ButtonModeResize },
};

#endif /* BUTTONS_RESIZE_H */