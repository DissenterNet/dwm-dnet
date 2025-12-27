#ifndef KEYMODE_TEST_H
#define KEYMODE_TEST_H

// * keys/keymode-test.h
// *
// * Test keys demonstrating Press/Release/Hold triggers. Useful to ensure the
// * press/hold/release semantics are wired correctly.
// *
// * NOTE: the Key struct in your modified dwm probably contains a `trigger`
// *       field controlling Press/Release/Hold. If your Key typedef hasn't got it,
// *       compile errors will point that out; adjust as needed.

static const Key testkeys[] = {
    /* F9 press -> notify (Press) */
    { MODKEY, XK_F9, spawn, SHCMD("notify-send 'Press F9'"), KeyModeTest, Press },

    /* F10 hold -> notify (Hold) (must hold longer than HOLD_DELAY to trigger) */
    { MODKEY, XK_F10, spawn, SHCMD("notify-send 'Held F10'"), KeyModeTest, Hold },

    /* F11 release -> notify (Release) */
    { MODKEY, XK_F11, spawn, SHCMD("notify-send 'Released F11'"), KeyModeTest, Release },
};

#endif /* KEYMODE_TEST_H */