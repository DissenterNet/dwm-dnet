#ifndef DWM_API_H
#define DWM_API_H

/* dwm_api.h - Comprehensive API for helper functions
 *
 * This header provides a clean interface for helper functions to access
 * dwm core functionality without including the entire dwm.c
 */

#ifdef DWM_CORE
/* We're building the main dwm.c - these are already defined */
#else
/* Helper modules building separately - provide forward declarations */

#include <X11/Xlib.h>

/* Core types */
typedef union {
    int i;
    unsigned int ui;
    float f;
    const void *v;
} Arg;

typedef struct Client Client;
typedef struct Monitor Monitor;

/* Key and button modes */
#define KEYMODE_ANY          -1
#define KEYMODE_NORMAL        0
#define KEYMODE_ALT           1
#define KEYMODE_VIM           2
#define KEYMODE_TESTING       3
#define KEYMODE_ADMIN         4
#define KEYMODE_VIM_INSERT    5
#define KEYMODE_VIM_VISUAL    6

#define BMODE_ANY           -1
#define BMODE_NORMAL         0
#define BMODE_ALT           1
#define BMODE_ALT_TWO       2
#define BMODE_ALT_THREE     3

/* Key trigger types */
typedef enum {
    KeyPressOnly = 0,
    KeyReleaseOnly,
    KeyHold
} KeyTrigger;

#endif /* DWM_CORE */

/* Core function prototypes that helpers may need */
void setkeymode(const Arg *arg);
void setbuttonmode(const Arg *arg);
void view(const Arg *arg);
void focus(const Arg *arg);
void focusstack(const Arg *arg);
void zoom(const Arg *arg);
void killclient(const Arg *arg);
void togglebar(const Arg *arg);
void togglefloating(const Arg *arg);
void togglefullscreen(const Arg *arg);
void togglesticky(const Arg *arg);
void setlayout(const Arg *arg);
void setmfact(const Arg *arg);
void incnmaster(const Arg *arg);
void focusmaster(const Arg *arg);
void focusmon(const Arg *arg);
void tag(const Arg *arg);
void toggletag(const Arg *arg);
void toggleview(const Arg *arg);
void spawn(const Arg *arg);

/* Helper-specific function prototypes */
void view_next_tag(const Arg *arg);
void view_prev_tag(const Arg *arg);
void toggle_to_vim(const Arg *arg);

/* Vim mode specific functions */
void vim_enter_normal_mode(const Arg *arg);
void vim_enter_insert_mode(const Arg *arg);
void vim_enter_visual_mode(const Arg *arg);
void vim_toggle_mode(const Arg *arg);

/* Global variables (extern declarations for helpers) */
extern int keymode;
extern int buttonmode;
extern Monitor *selmon;
extern Client *selmon->sel;

#endif /* DWM_API_H */
