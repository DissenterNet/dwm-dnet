#ifndef HELPERS_API_H
#define HELPERS_API_H

/* helpers_api.h — light, non-conflicting prototypes for helper headers.
 *
 * If DWM_CORE is defined (we're building the core dwm.c which already
 * defines Arg, Monitor, function prototypes, etc.), do NOT redefine types.
 * If DWM_CORE is NOT defined (building helpers standalone), provide
 * minimal forward declarations.
 */

#ifndef DWM_CORE
/* forward-declare the union/typedef names so helpers can compile in isolation */
typedef union Arg Arg;
typedef struct Monitor Monitor;
#endif /* DWM_CORE */

/* Prototypes helpers may call in the core.
 * Keep this list minimal and add only functions helpers actually call.
 * Match the signatures in dwm.c (void func(const Arg *arg)).
 */
void setkeymode(const Arg *arg);
void setbuttonmode(const Arg *arg);
void view(const Arg *arg);

// Add other core helper-callable prototypes here if your helper headers call them:
// e.g. void somefunc(const Arg *arg);


#endif /* HELPERS_API_H */
