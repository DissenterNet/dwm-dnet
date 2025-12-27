#ifndef CHORD_TEST_H
#define CHORD_TEST_H

static Keychord chord_test = {
    .n = 2,
    .keys = { { .mod = 0, .keysym = XK_t }, { .mod = 0, .keysym = XK_t } },
    .func = focusmaster,
    .arg = {0},
    .mode = KEYMODE_TESTING
};

#endif /* CHORD_TEST_H */