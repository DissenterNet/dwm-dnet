#ifndef CHORDS_CHORD_H
#define CHORDS_CHORD_H

/* utils/chords/chord.h */
#include "chords_decl.h"

/* Compile-time array of chord pointers used by dwm.c */
static Keychord *keychords[] = {
    &chord_vim_gg,
    &chord_vim_gt,
    &chord_vim_gT,
    &chord_vim_yy,
    &chord_vim_dd,
    &chord_vim_ww,
    &chord_vim_G,
    &chord_test,
    NULL
};

#endif /* CHORDS_CHORD_H */
