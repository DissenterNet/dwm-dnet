/* See LICENSE file for copyright and license details.
 *
 * dynamic window manager is designed like any other X client as well. It is
 * driven through handling X events. In contrast to other X clients, a window
 * manager selects for SubstructureRedirectMask on the root window, to receive
 * events about window (dis-)appearance. Only one X connection at a time is
 * allowed to select for this event mask.
 *
 * The event handlers of dwm are organized in an array which is accessed
 * whenever a new event has been fetched. This allows event dispatching
 * in O(1) time.
 *
 * Each child of the root window is called a client, except windows which have
 * set the override_redirect flag. Clients are organized in a linked client
 * list on each monitor, the focus history is remembered through a stack list
 * on each monitor. Each client contains a bit array to indicate the tags of a
 * client.
 *
 * Keys and tagging rules are organized as arrays and defined in config.h.
 *
 * To understand everything else, start reading main().
 */
#include <X11/XF86keysym.h>
#include <errno.h>
#include <locale.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif /* XINERAMA - multihead */
#include <X11/Xft/Xft.h>
#include <X11/Xlib-xcb.h>
#include <xcb/res.h>
#ifdef __OpenBSD__
#include <sys/sysctl.h>
#include <kvm.h>
#endif /* __OpenBSD */

#include "drw.h"
#include "util.h"

/* macros */
#define BUTTONMASK              (ButtonPressMask|ButtonReleaseMask)
#define CLEANMASK(mask)         (mask & ~(numlockmask|LockMask) & (ShiftMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask))
#define GETINC(X)               ((X) - 2000)
#define INC(X)                  ((X) + 2000)
#define INTERSECT(x,y,w,h,m)    (MAX(0, MIN((x)+(w),(m)->wx+(m)->ww) - MAX((x),(m)->wx)) \
                               * MAX(0, MIN((y)+(h),(m)->wy+(m)->wh) - MAX((y),(m)->wy)))
#define ISINC(X)                ((X) > 1000 && (X) < 3000)
#define ISVISIBLE(C)            ((C->tags & C->mon->tagset[C->mon->seltags]) || C->issticky)
#define PREVSEL                 3000
#define MOD(N,M)                ((N)%(M) < 0 ? (N)%(M) + (M) : (N)%(M))
#define MOUSEMASK               (BUTTONMASK|PointerMotionMask)
#define WIDTH(X)                ((X)->w + 2 * (X)->bw)
#define HEIGHT(X)               ((X)->h + 2 * (X)->bw)
#define TAGMASK                 ((1 << LENGTH(tags)) - 1)
#define TEXTW(X)                (drw_fontset_getwidth(drw, (X)) + lrpad)
#define XRDB_LOAD_COLOR(R,V)    if (XrmGetResource(xrdb, R, NULL, &type, &value) == True) { \
                                  if (value.addr != NULL && strnlen(value.addr, 8) == 7 && value.addr[0] == '#') { \
                                    int i = 1; \
                                    for (; i <= 6; i++) { \
                                      if (value.addr[i] < 48) break; \
                                      if (value.addr[i] > 57 && value.addr[i] < 65) break; \
                                      if (value.addr[i] > 70 && value.addr[i] < 97) break; \
                                      if (value.addr[i] > 102) break; \
                                    } \
                                    if (i == 7) { \
                                      strncpy(V, value.addr, 7); \
                                      V[7] = '\0'; \
                                    } \
                                  } \
                                }
#define TRUNC(X,A,B)            (MAX((A), MIN((X), (B))))

/* enums */
enum { CurNormal, CurResize, CurMove, CurLast }; /* cursor */
enum { SchemeNorm, SchemeSel, SchemeStatus, SchemeTagsSel, SchemeTagsNorm, SchemeInfoSel, SchemeInfoNorm }; /* color schemes */
enum { NetSupported, NetWMName, NetWMState, NetWMCheck,
       NetWMFullscreen, NetWMSticky, NetActiveWindow, NetWMWindowType,
       NetWMWindowTypeDialog, NetClientList, NetLast }; /* EWMH atoms (extended window manager hints) */
enum { WMProtocols, WMDelete, WMState, WMTakeFocus, WMLast }; /* default atoms */
enum { ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle,
       ClkClientWin, ClkRootWin, ClkLast }; /* clicks */

typedef union {
	int i; /* integer argument */
	unsigned int ui; /* unsigned integer */
	float f; /* float */
	const void *v; /* generic pointer */
} Arg;

typedef struct { /* mouse button interactions */
	unsigned int click; /* where the click happened */
	unsigned int mask; /* modifier keys */
	unsigned int button; /* the mouse button used */
	void (*func)(const Arg *arg); /* a pointer to the function that should be executed */
	const Arg arg; /* argument to pass into the function */
} Button;

typedef struct Monitor Monitor;
typedef struct Client Client;
struct Client { /* a window that dwm is managing */
	char name[256]; /* window's title as shown in bar */
	float mina, maxa; /* min and max aspect ratios when resizing windows */
	int x, y, w, h; /* current pos of window */
	int sfx, sfy, sfw, sfh; /* stored float geometry, used on mode revert */
	int oldx, oldy, oldw, oldh; /* prev pos of window */
	int basew, baseh, incw, inch, maxw, maxh, minw, minh, hintsvalid; /* size hints */
	int bw, oldbw; /* current and prev border widths */
	unsigned int tags; /* bitmasks for which tags window is visible on */
	int isfixed, isfloating, isurgent, neverfocus, oldstate, isfullscreen, issticky, isterminal, noswallow; /* window states */
	char scratchkey;
	int floatborderpx;
	int hasfloatbw;
	pid_t pid; /* pid of application in window - useful for swallowing */
	Client *next; /* next client, in the linked list of all clients */
	Client *snext; /* next in the STACK */
	Client *swallowing; /* points to the client this one is swallowing (swallow patch) */
	Monitor *mon; /* the monitor this client is on */
	Window win; /* X11 win id */
};

typedef struct { /* key press interactions */
	unsigned int mod;
	KeySym keysym;
	void (*func)(const Arg *);
	const Arg arg;
} Key;

typedef struct { /* layouts */
	const char *symbol;
	void (*arrange)(Monitor *);
} Layout;

struct Monitor {
	char ltsymbol[16];
	float mfact;
	int nmaster;
	int num;
	int by;               /* bar geometry */
	int mx, my, mw, mh;   /* screen size */
	int wx, wy, ww, wh;   /* window area  */
	int gappih;           /* horizontal gap between windows */
	int gappiv;           /* vertical gap between windows */
	int gappoh;           /* horizontal outer gaps */
	int gappov;           /* vertical outer gaps */
	unsigned int seltags;
	unsigned int sellt;
	unsigned int tagset[2];
	int showbar;
	int showtitle;
	int showtags;
	int showlayout;
	int showstatus;
	int showfloating;
	int topbar;
	Client *clients;
	Client *sel;
	Client *stack;
	Client *tagmarked[32];
	Monitor *next;
	Window barwin;
	const Layout *lt[2];
};

typedef struct {
	const char *class;
	const char *instance;
	const char *title;
	unsigned int tags;
	int isfloating;
	int isterminal;
	int noswallow;
	int monitor;
	int floatx, floaty, floatw, floath;
	int floatborderpx;
	const char scratchkey;
} Rule;

typedef struct {
	void (*func1)(const Arg *arg);
	void (*func2)(const Arg *arg);
	const Arg arg1;
	const Arg arg2;
} TwoFuncPtr;

/* function declarations */
static void applyrules(Client *c);
static int applysizehints(Client *c, int *x, int *y, int *w, int *h, int interact);
static void arrange(Monitor *m);
static void arrangemon(Monitor *m);
static void attach(Client *c);
static void attachstack(Client *c);
static void buttonpress(XEvent *e);
static void checkotherwm(void);
static void cleanup(void);
static void cleanupmon(Monitor *mon);
static void clientmessage(XEvent *e);
static void configure(Client *c);
static void configurenotify(XEvent *e);
static void configurerequest(XEvent *e);
static Monitor *createmon(void);
static void destroynotify(XEvent *e);
static void detach(Client *c);
static void detachstack(Client *c);
static Monitor *dirtomon(int dir);
static void drawbar(Monitor *m);
static void drawbars(void);
static void enternotify(XEvent *e);
static void expose(XEvent *e);
static void focus(Client *c);
static void focusin(XEvent *e);
static void focusmaster(const Arg *arg);
static void focusmon(const Arg *arg);
static void focusstack(const Arg *arg);
static Atom getatomprop(Client *c, Atom prop);
static int getrootptr(int *x, int *y);
static long getstate(Window w);
static pid_t getstatusbarpid(void);
static void sigstatusbar(const Arg *arg);
static int gettextprop(Window w, Atom atom, char *text, unsigned int size);
static void grabbuttons(Client *c, int focused);
static void grabkeys(void);
static void ifroot(const Arg *arg);
static void incnmaster(const Arg *arg);
static void keypress(XEvent *e);
static void killthis(Client *c);
static void killclient(const Arg *arg);
static void loadxrdb(void);
static void manage(Window w, XWindowAttributes *wa);
static void mappingnotify(XEvent *e);
static void maprequest(XEvent *e);
static void monocle(Monitor *m);
static void motionnotify(XEvent *e);
static void movemouse(const Arg *arg);
static Client *nexttiled(Client *c);
static void pop(Client *c);
static void propertynotify(XEvent *e);
static void pushstack(const Arg *arg);
static void quit(const Arg *arg);
static void quitprompt(const Arg *arg);
static Monitor *recttomon(int x, int y, int w, int h);
static void resize(Client *c, int x, int y, int w, int h, int interact);
static void resizeclient(Client *c, int x, int y, int w, int h);
static void resizemouse(const Arg *arg);
static void restack(Monitor *m);
static void run(void);
static void scan(void);
static int sendevent(Client *c, Atom proto);
static void sendmon(Client *c, Monitor *m);
static void setclientstate(Client *c, long state);
static void setfocus(Client *c);
static void setfullscreen(Client *c, int fullscreen);
static void setsticky(Client *c, int sticky);
static void setlayout(const Arg *arg);
static void setmfact(const Arg *arg);
static void setup(void);
static void seturgent(Client *c, int urg);
static void showhide(Client *c);
static void spawn(const Arg *arg);
static void spawnscratch(const Arg *arg);
static void sighup(int unused);
static void sigterm(int unused);
static int stackpos(const Arg *arg);
static void tag(const Arg *arg);
static void tagmon(const Arg *arg);
static void togglebar(const Arg *arg);
static void togglebarcolor(const Arg *arg);
static void togglebartags(const Arg *arg);
static void togglebartitle(const Arg *arg);
static void togglebarlt(const Arg *arg);
static void togglebarstatus(const Arg *arg);
static void togglebarfloat(const Arg *arg);
static void togglefloating(const Arg *arg);
static void togglescratch(const Arg *arg);
static void togglefullscreen(const Arg *arg);
static void togglesticky(const Arg *arg);
static void toggletag(const Arg *arg);
static void toggleview(const Arg *arg);
static void unfocus(Client *c, int setfocus);
static void unmanage(Client *c, int destroyed);
static void unmapnotify(XEvent *e);
static void updatebarpos(Monitor *m);
static void updatebars(void);
static void updateclientlist(void);
static int updategeom(void);
static void updatenumlockmask(void);
static void updatesizehints(Client *c);
static void updatestatus(void);
static void updatetitle(Client *c);
static void updatewindowtype(Client *c);
static void updatewmhints(Client *c);
static void view(const Arg *arg);
static Client *wintoclient(Window w);
static Monitor *wintomon(Window w);
static int xerror(Display *dpy, XErrorEvent *ee);
static int xerrordummy(Display *dpy, XErrorEvent *ee);
static int xerrorstart(Display *dpy, XErrorEvent *ee);
static void xrdb(const Arg *arg);
static void zoom(const Arg *arg);

static pid_t getparentprocess(pid_t p);
static int isdescprocess(pid_t p, pid_t c);
static Client *swallowingclient(Window w);
static Client *termforwin(const Client *c);
static pid_t winpid(Window w);

/* variables */
static const char broken[] = "broken";
static char stext[256];
static int statusw;
static int statussig;
static pid_t statuspid = -1;
static int screen;
static int sw, sh;           /* X display screen geometry width, height */
static int bh;               /* bar height */
static int lrpad;            /* sum of left and right padding for text */
static int (*xerrorxlib)(Display *, XErrorEvent *);
static unsigned int numlockmask = 0;
static void (*handler[LASTEvent]) (XEvent *) = { /* maps X event type to matching function */
	[ButtonPress] = buttonpress, /* mouse button */
	[ClientMessage] = clientmessage, /* messages sent from windows, like fullscreen toggle */
	[ConfigureRequest] = configurerequest, /* window (client) asks to change geometry */
	[ConfigureNotify] = configurenotify, /* root window geom changes */
	[DestroyNotify] = destroynotify, /* window closed - removes client from dwm */
	[EnterNotify] = enternotify, /* mouse enters a window */
	[Expose] = expose, /* a part of the window needs to be redrawn */
	[FocusIn] = focusin,
	[KeyPress] = keypress, /* keyboard */
	[MappingNotify] = mappingnotify, /* keyboard mapping change */
	[MapRequest] = maprequest, /* a new window needs to be mapped */
	[MotionNotify] = motionnotify, /* mouse movement */
	[PropertyNotify] = propertynotify, /* window property changes */
	[UnmapNotify] = unmapnotify /* window needs to be unmapped */
};
static Atom wmatom[WMLast], netatom[NetLast];
static int restart = 1;
static int running = 1;
static Cur *cursor[CurLast];
static Clr **scheme;
static Display *dpy;
static Drw *drw;
static Monitor *mons, *selmon;
static Window root, wmcheckwin;

static xcb_connection_t *xcon;

/* configuration, allows nested code to access above variables */
#include "config.h"

/* compile-time check if all tags fit into an unsigned int bit array. */
struct NumTags { char limitexceeded[LENGTH(tags) > 31 ? -1 : 1]; };

/* function implementations */
/**
 * @brief Apply matching rules to a new client window
 * @param c Pointer to the client structure to apply rules to
 * 
 * This function applies configuration rules to newly created windows based on their
 * class, instance, and title properties. Rules are defined in config.h and control
 * initial window properties like tags, floating state, monitor assignment, and
 * terminal/swallowing behavior.
 * 
 * The function iterates through all rules and applies the first matching rule or
 * multiple matching rules (rules can accumulate effects). Window properties are
 * extracted using XGetClassHint() and matched against rule criteria using strstr().
 * 
 * @note Rules can set: terminal status, floating state, tags, scratchpad key,
 *       floating border width, geometry, and target monitor
 * @note If no tags are set after rule application, defaults to current monitor tagset
 * 
 * @warning SECURITY ISSUE: No input sanitization on class/instance strings before
 *          strstr() matching could lead to unexpected matches
 * @warning BOUNDS CHECKING: No validation that r->floatx/y/w/h values are within
 *          screen bounds, could place windows outside visible area
 * @warning MEMORY LEAK: If XGetClassHint fails, ch.res_class/res_name may not be
 *          properly freed in all error paths
 * @warning DUPLICATE RULES: No protection against multiple rules applying conflicting
 *          settings to the same client
 * @warning INVALID VALUES: No validation of scratchkey character range or validity
 * 
 * @bug COMPLEX LOGIC: Nested condition with multiple && operators makes debugging
 *      and maintenance difficult
 * @bug MONITOR SEARCH: Linear search through monitor list could be inefficient with
 *      many monitors
 * 
 * @return void
 */
void
applyrules(Client *c)
{
	const char *class, *instance;
	unsigned int i;
	const Rule *r;
	Monitor *m;
	XClassHint ch = { NULL, NULL };

	/* rule matching */
	c->isfloating = 0;
	c->tags = 0;

	XGetClassHint(dpy, c->win, &ch);
	class    = ch.res_class ? ch.res_class : broken;
	instance = ch.res_name  ? ch.res_name  : broken;
	c->scratchkey = 0;
	for (i = 0; i < LENGTH(rules); i++) {
		r = &rules[i];
		if ((!r->title || strstr(c->name, r->title))
		&& (!r->class || strstr(class, r->class))
		&& (!r->instance || strstr(instance, r->instance)))
		{
			c->isterminal = r->isterminal;
			c->noswallow  = r->noswallow;
			c->isfloating = r->isfloating;
			c->tags |= r->tags;
			c->scratchkey = r->scratchkey;
			if (r->floatborderpx >= 0) {
				c->floatborderpx = r->floatborderpx;
				c->hasfloatbw = 1;
			}
			if (r->isfloating) {
				if (r->floatx >= 0) c->x = c->mon->mx + r->floatx;
				if (r->floaty >= 0) c->y = c->mon->my + r->floaty;
				if (r->floatw >= 0) c->w = r->floatw;
				if (r->floath >= 0) c->h = r->floath;
			}
			for (m = mons; m && m->num != r->monitor; m = m->next);
			if (m)
				c->mon = m;
		}
	}
	if (ch.res_class)
		XFree(ch.res_class);
	if (ch.res_name)
		XFree(ch.res_name);

	c->tags = c->tags & TAGMASK ? c->tags & TAGMASK : c->mon->tagset[c->mon->seltags];
}

/**
 * @brief Apply size hints to client window geometry
 * @param c Pointer to client structure to apply hints to
 * @param x Pointer to x coordinate to be modified
 * @param y Pointer to y coordinate to be modified  
 * @param w Pointer to width to be modified
 * @param h Pointer to height to be modified
 * @param interact Boolean flag indicating if this is an interactive resize
 * @return int Returns 1 if geometry was changed, 0 otherwise
 * 
 * This function enforces ICCCM size hints including minimum/maximum sizes, aspect ratios,
 * and increment values. It modifies the provided geometry parameters to comply with
 * the client's size hints while keeping the window as large as possible within
 * the constraints.
 * 
 * The function handles both interactive (user-driven) and programmatic resizes
 * differently. Interactive resizes allow the window to be partially outside screen
 * bounds, while programmatic resizes are constrained to the monitor area.
 * 
 * @note Only applies size hints if resizehints is enabled, client is floating,
 *       or current layout has no arrange function
 * @note Follows ICCCM 4.1.2.3 specification for base size handling
 * 
 * @warning COMPLEX LOGIC: Nested condition structure is difficult to maintain
 *          and debug
 * @warning PERFORMANCE: Multiple calls to updatesizehints() may be redundant
 * @warning BOUNDS CHECKING: Interactive mode allows negative coordinates which
 *          may cause issues with some applications
 * 
 * @return 1 if geometry parameters were modified, 0 otherwise
 */
int
applysizehints(Client *c, int *x, int *y, int *w, int *h, int interact)
{
	int baseismin;
	Monitor *m = c->mon;

	/* set minimum possible */
	*w = MAX(1, *w);
	*h = MAX(1, *h);
	if (interact) {
		if (*x > sw)
			*x = sw - WIDTH(c);
		if (*y > sh)
			*y = sh - HEIGHT(c);
		if (*x + *w + 2 * c->bw < 0)
			*x = 0;
		if (*y + *h + 2 * c->bw < 0)
			*y = 0;
	} else {
		if (*x >= m->wx + m->ww)
			*x = m->wx + m->ww - WIDTH(c);
		if (*y >= m->wy + m->wh)
			*y = m->wy + m->wh - HEIGHT(c);
		if (*x + *w + 2 * c->bw <= m->wx)
			*x = m->wx;
		if (*y + *h + 2 * c->bw <= m->wy)
			*y = m->wy;
	}
	if (*h < bh)
		*h = bh;
	if (*w < bh)
		*w = bh;
	if (resizehints || c->isfloating || !c->mon->lt[c->mon->sellt]->arrange) {
		if (!c->hintsvalid)
			updatesizehints(c);
		/* see last two sentences in ICCCM 4.1.2.3 */
		baseismin = c->basew == c->minw && c->baseh == c->minh;
		if (!baseismin) { /* temporarily remove base dimensions */
			*w -= c->basew;
			*h -= c->baseh;
		}
		/* adjust for aspect limits */
		if (c->mina > 0 && c->maxa > 0) {
			if (c->maxa < (float)*w / *h)
				*w = *h * c->maxa + 0.5;
			else if (c->mina < (float)*h / *w)
				*h = *w * c->mina + 0.5;
		}
		if (baseismin) { /* increment calculation requires this */
			*w -= c->basew;
			*h -= c->baseh;
		}
		/* adjust for increment value */
		if (c->incw)
			*w -= *w % c->incw;
		if (c->inch)
			*h -= *h % c->inch;
		/* restore base dimensions */
		*w = MAX(*w + c->basew, c->minw);
		*h = MAX(*h + c->baseh, c->minh);
		if (c->maxw)
			*w = MIN(*w, c->maxw);
		if (c->maxh)
			*h = MIN(*h, c->maxh);
	}
	return *x != c->x || *y != c->y || *w != c->w || *h != c->h;
}

/**
 * @brief Arrange windows on a specific monitor or all monitors
 * @param m Pointer to monitor to arrange (NULL arranges all monitors)
 * @note This function handles window visibility and calls the appropriate layout function
 * @note It also ensures proper stacking order with focused window on top
 * @warning The caller must ensure m is either NULL or a valid Monitor pointer
 * @return void
 */
void
arrange(Monitor *m)
{
	XEvent ev; /* declares a generic X event */
	if (m)
		showhide(m->stack); /* show visible windows and hide others */
	else for (m = mons; m; m = m->next)
		showhide(m->stack);
	if (m) { /* if *m is a single monitor */
		arrangemon(m); /* applies to current layout to windows (tile, spiral, etc)*/
		restack(m); /* proper stacking order --> focused window on top */
	} else { /* else if *m was NULL */
		for (m = mons; m; m = m->next) /* loop through all monitors */
			arrangemon(m); /* apply layout to each monitor */
		XSync(dpy, False); /* ensure X server processed all commands (cleaning up) */
		while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));
	}
}

/**
 * @brief Apply the current layout to all windows on a specific monitor
 * @param m Pointer to the Monitor structure to arrange windows on
 * @note This function updates the monitor's layout symbol and calls the layout's arrange function
 * @note If no arrange function is defined for the current layout, no window repositioning occurs
 * @warning The caller must ensure m is a valid Monitor pointer
 */
void
arrangemon(Monitor *m)
{ /* applies current layout to window on monitor *m */
	strncpy(m->ltsymbol, m->lt[m->sellt]->symbol, sizeof m->ltsymbol); /* copies layout symbol string to show in bar */
	if (m->lt[m->sellt]->arrange) /* if the layout has an associated arrange func, call it (ex. tile() )*/
		m->lt[m->sellt]->arrange(m);
}

/**
 * @brief Attach a client to the beginning of the monitor's client list
 * @param c Pointer to Client structure to attach
 * @note This function adds the client to the front of the linked list
 * @note The client becomes the first in the monitor's client list
 * @warning The caller must ensure c is a valid Client pointer and c->mon is set
 * @return void
 */
void
attach(Client *c)
{
	c->next = c->mon->clients;
	c->mon->clients = c;
}

/**
 * @brief Attach a client to the top of the monitor's stack
 * @param c Pointer to Client structure to attach to stack
 * @note This function adds the client to the front of the stacking order
 * @note The stack determines which window appears on top (focused)
 * @warning The caller must ensure c is a valid Client pointer and c->mon is set
 * @return void
 */
void
attachstack(Client *c)
{
	c->snext = c->mon->stack;
	c->mon->stack = c;
}

/**
 * @brief Make a terminal window swallow its child window
 * @param p Pointer to parent client (terminal) that will swallow the child
 * @param c Pointer to child client that will be swallowed
 * 
 * This function implements window swallowing where a terminal window "swallows" its child
 * process window. The parent window becomes hidden and the child takes its place in the
 * window management system. This is commonly used for terminal-based applications.
 * 
 * The function performs several operations:
 * - Checks if swallowing is allowed (not noswallow, not terminal itself)
 * - Detaches child from current management
 * - Sets child state to WithdrawnState
 * - Swaps window IDs between parent and child
 * - Updates title and geometry
 * - Rearranges the monitor
 * 
 * @note Swallowing is only done if the child is not marked as noswallow and not a terminal
 * @note The child window inherits the parent's monitor and geometry
 * 
 * @warning This function modifies client structures extensively and can cause unexpected behavior
 * @warning POTENTIAL ISSUE: Complex state management could lead to race conditions
 * 
 * @return void
 */
void
swallow(Client *p, Client *c)
{

	if (c->noswallow || c->isterminal)
		return;
	if (c->noswallow && !swallowfloating && c->isfloating)
		return;

	detach(c);
	detachstack(c);

	setclientstate(c, WithdrawnState);
	XUnmapWindow(dpy, p->win);

	p->swallowing = c;
	c->mon = p->mon;

	Window w = p->win;
	p->win = c->win;
	c->win = w;
	updatetitle(p);
	XMoveResizeWindow(dpy, p->win, p->x, p->y, p->w, p->h);
	arrange(p->mon);
	configure(p);
	updateclientlist();
}

/**
 * @brief Reverse the swallowing process and restore original window
 * @param c Pointer to client that is currently swallowing another window
 * 
 * This function reverses the swallowing process by restoring the original window
 * and cleaning up the swallowing relationship. It un-fullscreens the client,
 * restores the original window ID, maps the window, and updates the client state.
 * 
 * The function performs these operations:
 * - Restores original window ID from swallowed client
 * - Frees the swallowing client structure
 * - Disables fullscreen mode
 * - Updates window title
 * - Maps and positions the window
 * - Sets client state to NormalState
 * - Rearranges the monitor
 * 
 * @note This function assumes c->swallowing is not NULL
 * @note The client will be unfullscreened regardless of previous state
 * 
 * @warning MEMORY: Frees c->swallowing memory, caller must ensure it's not used afterward
 * @warning STATE: Modifies client state significantly
 * 
 * @return void
 */
void
unswallow(Client *c)
{
	c->win = c->swallowing->win;

	free(c->swallowing);
	c->swallowing = NULL;

	/* unfullscreen the client */
	setfullscreen(c, 0);
	updatetitle(c);
	arrange(c->mon);
	XMapWindow(dpy, c->win);
	XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
	setclientstate(c, NormalState);
	focus(NULL);
	arrange(c->mon);
}

/**
 * @brief Handle mouse button press events on the bar and windows
 * @param e Pointer to XEvent containing button press data
 * @note This function determines which area was clicked and calls appropriate handlers
 * @note Handles clickable areas: tags, layout symbol, status text, window title, client windows, root window
 * @note Processes status bar click signals for dwmblocks integration
 * @warning Complex nested loops could be hard to maintain
 * @warning Status text parsing assumes ASCII encoding
 * @return void
 */
void
buttonpress(XEvent *e)
{ /* handles clickable areas on the bar and windows */
	unsigned int i, x, click;
	Arg arg = {0};
	Client *c;
	Monitor *m;
	XButtonPressedEvent *ev = &e->xbutton;
	char *text, *s, ch;

	click = ClkRootWin;
	/* focus monitor if necessary */
	if ((m = wintomon(ev->window)) && m != selmon) {
		unfocus(selmon->sel, 1);
		selmon = m;
		focus(NULL);
	}
	if (ev->window == selmon->barwin) {
		i = x = 0;
		unsigned int occ = 0;
		for(c = m->clients; c; c=c->next)
			occ |= c->tags == TAGMASK ? 0 : c->tags;
		do {
			/* do not reserve space for vacant tags */
			if (!(occ & 1 << i || m->tagset[m->seltags] & 1 << i))
				continue;
		    if (selmon->showtags)
				x += TEXTW(tags[i]);
		} while (ev->x >= x && ++i < LENGTH(tags));
		if (i < LENGTH(tags) && selmon->showtags) {
			click = ClkTagBar;
			arg.ui = 1 << i;
		} else if (ev->x < x + TEXTW(selmon->ltsymbol) && selmon->showlayout)
			click = ClkLtSymbol;
		else if (ev->x > selmon->ww - statusw && selmon->showstatus) {
			x = selmon->ww - statusw;
			click = ClkStatusText;
			statussig = 0; /* statuscmd stuff */
			for (text = s = stext; *s && x <= ev->x; s++) { /* loop through to determine which block was clicked */
				if ((unsigned char)(*s) < ' ') { /* sig delim, if block boundaries are off check this */
					ch = *s; /* measure width */
					*s = '\0';
					x += TEXTW(text) - lrpad; /* debug test try (lrpad / 2) */
					*s = ch;
					text = s + 1; /* move text to next char after delim */
					if (x >= ev->x) /* check click pos */
						break;
					statussig = ch; /* save control char as sig # */
				}
			}
		} else if (selmon->showtitle) {
			statussig = 0;
			for (text = s = stext; *s && x <= ev->x; s++) {
				if ((unsigned char)(*s) < ' ') {
					ch = *s;
					*s = '\0';
					x += TEXTW(text) - lrpad;
					*s = ch;
					text = s + 1;
						break;
					statussig = ch;
								}
						}
			}
		} else
			click = ClkWinTitle; 
		if ((c = wintoclient(ev->window))) {
		focus(c);
		restack(selmon);
		XAllowEvents(dpy, ReplayPointer, CurrentTime);
		click = ClkClientWin;
	}
	for (i = 0; i < LENGTH(buttons); i++)
		if (click == buttons[i].click && buttons[i].func && buttons[i].button == ev->button
		&& CLEANMASK(buttons[i].mask) == CLEANMASK(ev->state))
			buttons[i].func(click == ClkTagBar && buttons[i].arg.i == 0 ? &arg : &buttons[i].arg);
}

/**
 * @brief Check if another window manager is already running
 * @note This function attempts to select SubstructureRedirectMask on the root window
 * @note If another WM is running, this will cause an error and trigger xerrorstart
 * @note Uses temporary error handler to detect existing WM
 * @warning This must be called before any other X operations
 * @return void
 */
void
checkotherwm(void)
{
	xerrorxlib = XSetErrorHandler(xerrorstart);
	/* this causes an error if some other window manager is running */
	XSelectInput(dpy, DefaultRootWindow(dpy), SubstructureRedirectMask);
	XSync(dpy, False);
	XSetErrorHandler(xerror);
	XSync(dpy, False);
}

/**
 * @brief Clean up and exit dwm normally or through restart
 * @note This function is called when dwm exits normally or through restart
 * @note Switches to viewing all windows before shutdown and sets layout to NULL
 * @note Releases all X keybindings and cleans up memory allocations
 * @note Restores keyboard focus to root window and removes EWMH properties
 * @return void
 */
void
cleanup(void)
{ /* called when dwm exits normally or thru restart */
	Arg a = {.ui = ~0}; /* switches to viewing all windows before shutdown */
	Layout foo = { "", NULL }; /* switches layout to NULL so no more layout logic is ran */
	Monitor *m;
	size_t i;

	view(&a);
	selmon->lt[selmon->sellt] = &foo;
	for (m = mons; m; m = m->next) /* loop through every monitor */
		while (m->stack) /* call unmanage on every client window in the stack */
			unmanage(m->stack, 0);
	XUngrabKey(dpy, AnyKey, AnyModifier, root); /* releases all X keybindings */
	while (mons)
		cleanupmon(mons);
	for (i = 0; i < CurLast; i++)
		drw_cur_free(drw, cursor[i]);
	for (i = 0; i < LENGTH(colors); i++)
		free(scheme[i]);
	free(scheme);
	XDestroyWindow(dpy, wmcheckwin);
	drw_free(drw);
	XSync(dpy, False); /* flushes all X requests */
	XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime); /* reverts keyboard focus to root win */
	XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
}

/**
 * @brief Clean up and free a monitor structure
 * @param mon Pointer to the Monitor structure to clean up
 * @note This function removes the monitor from the global monitor list, destroys its bar window, and frees memory
 * @warning The caller must ensure mon is a valid Monitor pointer
 * @bug Potential memory leak if barwin destruction fails
 * @return void
 */
void
cleanupmon(Monitor *mon)
{
	Monitor *m;

	if (mon == mons)
		mons = mons->next;
	else {
		for (m = mons; m && m->next != mon; m = m->next);
		m->next = mon->next;
	}
	XUnmapWindow(dpy, mon->barwin); /* hides statusbar associated w/ monitor */
	XDestroyWindow(dpy, mon->barwin); /* deletes bar window from X server entirely */
	free(mon); /* free memory */
}

/**
 * @brief Handle client message events from applications
 * @param e Pointer to XEvent containing client message data
 * @note This function handles EWMH client messages like fullscreen and sticky state changes
 * @note Processes messages for window state changes and active window notifications
 * @warning The caller must ensure e is a valid ClientMessage XEvent
 * @return void
 */
void
clientmessage(XEvent *e)
{
	XClientMessageEvent *cme = &e->xclient;
	Client *c = wintoclient(cme->window);

	if (!c)
		return;
	if (cme->message_type == netatom[NetWMState]) {
		if (cme->data.l[1] == netatom[NetWMFullscreen]
		|| cme->data.l[2] == netatom[NetWMFullscreen])
			setfullscreen(c, (cme->data.l[0] == 1 /* _NET_WM_STATE_ADD    */
				|| (cme->data.l[0] == 2 /* _NET_WM_STATE_TOGGLE */ && !c->isfullscreen)));

        if (cme->data.l[1] == netatom[NetWMSticky]
                || cme->data.l[2] == netatom[NetWMSticky])
            setsticky(c, (cme->data.l[0] == 1 || (cme->data.l[0] == 2 && !c->issticky)));
	} else if (cme->message_type == netatom[NetActiveWindow]) {
		if (c != selmon->sel && !c->isurgent)
			seturgent(c, 1);
	}
}

/**
 * @brief Send a ConfigureNotify event to a client window
 * @param c Pointer to client structure to send event to
 * @note This function sends a synthetic ConfigureNotify event to inform the client of its current geometry
 * @note Used to synchronize client's view of its window properties
 * @note The event includes current position, size, border width, and stacking information
 * @warning The caller must ensure c is a valid Client pointer
 * @return void
 */
void
configure(Client *c)
{
	XConfigureEvent ce;

	ce.type = ConfigureNotify;
	ce.display = dpy;
	ce.event = c->win;
	ce.window = c->win;
	ce.x = c->x;
	ce.y = c->y;
	ce.width = c->w;
	ce.height = c->h;
	ce.border_width = c->bw;
	ce.above = None;
	ce.override_redirect = False;
	XSendEvent(dpy, c->win, False, StructureNotifyMask, (XEvent *)&ce);
}

/**
 * @brief Handle configure notify events from X server
 * @param e Pointer to XEvent containing configure notify data
 * @note This function handles root window geometry changes and updates monitor layout accordingly
 * @note Triggers geometry updates when screen resolution changes
 * @warning The TODO comment indicates this function needs refactoring for better maintainability
 * @warning Complex nested logic makes debugging difficult
 * @return void
 */
void
configurenotify(XEvent *e)
{
	Monitor *m;
	Client *c;
	XConfigureEvent *ev = &e->xconfigure;
	int dirty;

	/* TODO: updategeom handling sucks, needs to be simplified */
	if (ev->window == root) {
		dirty = (sw != ev->width || sh != ev->height);
		sw = ev->width;
		sh = ev->height;
		if (updategeom() || dirty) {
			drw_resize(drw, sw, bh);
			updatebars();
			for (m = mons; m; m = m->next) {
				for (c = m->clients; c; c = c->next)
					if (c->isfullscreen)
						resizeclient(c, m->mx, m->my, m->mw, m->mh);
				XMoveResizeWindow(dpy, m->barwin, m->wx, m->by, m->ww, bh);
			}
			focus(NULL);
			arrange(NULL);
		}
	}
}

/**
 * @brief Handle configure request events from client windows
 * @param e Pointer to XEvent containing configure request data
 * 
 * This function handles ConfigureRequest events sent by clients that want to change
 * their window geometry or stacking order. For managed windows, it applies the
 * requested changes if the window is floating or if no layout function is active.
 * For unmanaged windows, it forwards the request directly to X server.
 * 
 * The function ensures floating windows stay within monitor bounds by centering
 * them if they would extend beyond the monitor area.
 * 
 * @note Only applies geometry changes for floating windows or when no layout is active
 * @note Centers floating windows that would extend beyond monitor boundaries
 * 
 * @warning BOUNDS CHECKING: May place windows at negative coordinates if monitor
 *          geometry is invalid
 * @warning PERFORMANCE: Multiple XConfigureWindow calls may impact performance
 * 
 * @return void
 */
void
configurerequest(XEvent *e)
{
	Client *c;
	Monitor *m;
	XConfigureRequestEvent *ev = &e->xconfigurerequest;
	XWindowChanges wc;

	if ((c = wintoclient(ev->window))) {
		if (ev->value_mask & CWBorderWidth)
			c->bw = ev->border_width;
		else if (c->isfloating || !selmon->lt[selmon->sellt]->arrange) {
			m = c->mon;
			if (ev->value_mask & CWX) {
				c->oldx = c->x;
				c->x = m->mx + ev->x;
			}
			if (ev->value_mask & CWY) {
				c->oldy = c->y;
				c->y = m->my + ev->y;
			}
			if (ev->value_mask & CWWidth) {
				c->oldw = c->w;
				c->w = ev->width;
			}
			if (ev->value_mask & CWHeight) {
				c->oldh = c->h;
				c->h = ev->height;
			}
			if ((c->x + c->w) > m->mx + m->mw && c->isfloating)
				c->x = m->mx + (m->mw / 2 - WIDTH(c) / 2); /* center in x direction */
			if ((c->y + c->h) > m->my + m->mh && c->isfloating)
				c->y = m->my + (m->mh / 2 - HEIGHT(c) / 2); /* center in y direction */
			if ((ev->value_mask & (CWX|CWY)) && !(ev->value_mask & (CWWidth|CWHeight)))
				configure(c);
			if (ISVISIBLE(c))
				XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
		} else
			configure(c);
	} else {
		wc.x = ev->x;
		wc.y = ev->y;
		wc.width = ev->width;
		wc.height = ev->height;
		wc.border_width = ev->border_width;
		wc.sibling = ev->above;
		wc.stack_mode = ev->detail;
		XConfigureWindow(dpy, ev->window, ev->value_mask, &wc);
	}
	XSync(dpy, False);
}

/**
 * @brief Create and initialize a new monitor structure
 * @return Pointer to newly allocated Monitor structure, or NULL on failure
 * 
 * This function allocates and initializes a Monitor structure with default
 * values from the global configuration variables. It sets up the initial
 * tagset, layout, gap settings, and bar visibility options.
 * 
 * The monitor is configured with:
 * - Both tagsets initialized to tag 1 (first tag)
 * - Layout pointers set to first and second layout from global layouts array
 * - All visibility flags (bar, title, tags, layout, status, floating) set from config
 * - Gap settings initialized from global variables
 * - Layout symbol copied from first layout
 * 
 * @note The caller is responsible for adding this monitor to the global monitor list
 * @note Uses ecalloc() which exits on allocation failure
 * 
 * @warning MEMORY: Returns NULL if ecalloc() fails (though ecalloc() typically exits)
 * @warning CONFIGURATION: Depends on global variables being properly initialized
 * 
 * @return Pointer to initialized Monitor structure
 */
Monitor *
createmon(void)
{
	Monitor *m;

	m = ecalloc(1, sizeof(Monitor));
	m->tagset[0] = m->tagset[1] = 1;
	m->mfact = mfact;
	m->nmaster = nmaster;
	m->showbar = showbar;
	m->showtitle = showtitle;
	m->showtags = showtags;
	m->showlayout = showlayout;
	m->showstatus = showstatus;
	m->showfloating = showfloating;
	m->topbar = topbar;
	m->gappih = gappih;
	m->gappiv = gappiv;
	m->gappoh = gappoh;
	m->gappov = gappov;
	m->lt[0] = &layouts[0];
	m->lt[1] = &layouts[1 % LENGTH(layouts)];
	strncpy(m->ltsymbol, layouts[0].symbol, sizeof m->ltsymbol);
	return m;
}

/**
 * @brief Handle destroy window notify events from X server
 * @param e Pointer to XEvent containing destroy window data
 * 
 * This function handles DestroyNotify events which are sent when a window
 * is destroyed. It checks if the destroyed window is a managed client
 * or a client that is currently swallowing another window, and calls unmanage() accordingly.
 * 
 * The function handles two cases:
 * 1. Direct client destruction - removes the client from DWM management
 * 2. Swallowing client destruction - removes the parent client that was swallowing
 * 
 * @note This is the primary way windows are removed from DWM's control
 * @note Swallowing relationships are properly cleaned up during destruction
 * 
 * @warning The caller must ensure e is a valid DestroyNotify XEvent
 * @warning MEMORY: No explicit NULL check for wintoclient() result, but unmanage() handles NULL
 * 
 * @return void
 */
void
destroynotify(XEvent *e)
{
	Client *c;
	XDestroyWindowEvent *ev = &e->xdestroywindow;

	if ((c = wintoclient(ev->window)))
		unmanage(c, 1);

	else if ((c = swallowingclient(ev->window)))
		unmanage(c->swallowing, 1);
}

/**
 * @brief Remove a client from the monitor's client list
 * @param c Pointer to Client structure to detach
 * 
 * This function removes a client from the beginning of the monitor's client list
 * by updating the next pointer of the previous client to point to the
 * client after the one being detached. It also clears any tagmarked
 * references that point to this client.
 * 
 * @note This function only removes from the client list, not from the stack
 * @note The client remains in the stack and may still be focused
 * 
 * @warning The caller must ensure c is a valid Client pointer and c->mon is set
 * @warning This function does not update focus or arrange windows
 * 
 * @return void
 */
void
detach(Client *c)
{
	Client **tc;

	for (int i = 1; i < LENGTH(tags); i++)
		if (c == c->mon->tagmarked[i])
			c->mon->tagmarked[i] = NULL;

	for (tc = &c->mon->clients; *tc && *tc != c; tc = &(*tc)->next);
	*tc = c->next;
}

void
detachstack(Client *c)
{
	Client **tc, *t;

	for (tc = &c->mon->stack; *tc && *tc != c; tc = &(*tc)->snext);
	*tc = c->snext;

	if (c == c->mon->sel) {
		for (t = c->mon->stack; t && !ISVISIBLE(t); t = t->snext);
		c->mon->sel = t;
	}
}

Monitor *
dirtomon(int dir)
{
	Monitor *m = NULL;

	if (dir > 0) {
		if (!(m = selmon->next))
			m = mons;
	} else if (selmon == mons)
		for (m = mons; m->next; m = m->next);
	else
		for (m = mons; m->next != selmon; m = m->next);
	return m;
}

void
drawbar(Monitor *m) /* take a pointer to the monitor we want to draw the bar on */
{
	int x, w, tw = 0; /* x pos, width, and text width */
	int boxs = drw->fonts->h / 9; /* the little square box for indicators */
	int boxw = drw->fonts->h / 6 + 2;
	unsigned int i, occ = 0, urg = 0; /* track which tags are in use and which are urgent */
	Client *c; /* pointer to iterate over list of windows on this monitor */

	if (!m->showbar) /* if the bar is hidden, do not draw it */
		return;

	/* draw status first so it can be overdrawn by tags later */
	if (m == selmon && selmon->showstatus) { /* status is only drawn on selected monitor */
		char *text, *s, ch;
		drw_setscheme(drw, scheme[SchemeStatus]); /* set the colorscheme used by the drawing context */
		x = 0; /* keep track of horiz pos whil drawing */
		for (text = s = stext; *s; s++) { /* this loop handles the dwmblocks clickable blocks */
			if ((unsigned char)(*s) < ' ') { /* ctrl chars (ASCII < 32) are used to separate click events */
				ch = *s;
				*s = '\0';
				tw = TEXTW(text) - lrpad;
				drw_text(drw, m->ww - statusw + x, 0, tw, bh, 0, text, 0);
				x += tw;
				*s = ch;
				text = s + 1;
			}
		}
		tw = TEXTW(text) - lrpad + 2; /* draw the last remaining segment after the last ctrl char */
		drw_text(drw, m->ww - statusw + x, 0, tw, bh, 0, text, 0);
		tw = statusw; 
	}

	for (c = m->clients; c; c = c->next) { /* drawing tag indicators */
		occ |= c->tags == TAGMASK ? 0 : c->tags; /* if a client has all tags, skip it */
		if (c->isurgent && selmon->showtags)
			urg |= c->tags;
	}
	x = 0;
	for (i = 0; i < LENGTH(tags); i++) { /* loop over each tag */
		if(!(occ & 1 << i || m->tagset[m->seltags] & 1 << i)) /* do not draw vacant tags */
			continue;
		if (selmon->showtags) {
				w = TEXTW(tags[i]);
				drw_setscheme(drw, scheme[m->tagset[m->seltags] & 1 << i ? SchemeTagsSel : SchemeTagsNorm]);
				drw_text(drw, x, 0, w, bh, lrpad / 2, tags[i], urg & 1 << i);
				if (occ & 1 << i && selmon->showfloating) /* if tag has a win and showfloating is enabled, draw the floating indicator */
				drw_rect(drw, x + boxs, boxs, boxw, boxw,
						m == selmon && selmon->sel && selmon->sel->tags & 1 << i,
						urg & 1 << i);
				x += w; /* move cursor to the right to prepare for drawing next tag */
		}
    }
	
	/* draw layout indicator if selmon->showlayout */
	if (selmon->showlayout) {
		w = TEXTW(m->ltsymbol);
		drw_setscheme(drw, scheme[SchemeTagsNorm]);
		x = drw_text(drw, x, 0, w, bh, lrpad / 2, m->ltsymbol, 0);
	}


	if ((w = m->ww - tw - x) > bh) {
		if (m->sel && selmon->showtitle) {
/* fix overflow when window name is bigger than window width */
			int mid = (m->ww - (int)TEXTW(m->sel->name)) / 2 - x;
/* make sure name will not overlap on tags even when it is very long */
			mid = mid >= lrpad / 2 ? mid : lrpad / 2;
			drw_setscheme(drw, scheme[m == selmon ? SchemeInfoSel : SchemeInfoNorm]);
			drw_text(drw, x, 0, w, bh, mid, m->sel->name, 0);
			if (m->sel->isfloating && selmon->showfloating)
				drw_rect(drw, x + boxs, boxs, boxw, boxw, m->sel->isfixed, 0);
		} else {
			drw_setscheme(drw, scheme[SchemeInfoNorm]);
			drw_rect(drw, x, 0, w, bh, 1, 1);
		}
	}
	drw_map(drw, m->barwin, 0, 0, m->ww, bh);
}

void
drawbars(void)
{ /* loop through every monitor to redraw statusbar */
	Monitor *m;

	for (m = mons; m; m = m->next)
		drawbar(m);
}

void
enternotify(XEvent *e)
{
	Client *c;
	Monitor *m;
	XCrossingEvent *ev = &e->xcrossing;

	if ((ev->mode != NotifyNormal || ev->detail == NotifyInferior) && ev->window != root)
		return;
	c = wintoclient(ev->window);
	m = c ? c->mon : wintomon(ev->window);
	if (m != selmon) {
		unfocus(selmon->sel, 1);
		selmon = m;
	} else if (!c || c == selmon->sel)
		return;
	focus(c);
}

void
expose(XEvent *e)
{
	Monitor *m;
	XExposeEvent *ev = &e->xexpose;

	if (ev->count == 0 && (m = wintomon(ev->window)))
		drawbar(m);
}

void
focus(Client *c)
{ /* handling when focus shifts to a different client */
	if (!c || !ISVISIBLE(c)) /* if the target client c isn't visible, */
		for (c = selmon->stack; c && !ISVISIBLE(c); c = c->snext); /* search the stack for the next visible one to focus */
	if (selmon->sel && selmon->sel != c) /* if there's a selected window that is not c, */
		unfocus(selmon->sel, 0); /* unfocus it */
	if (c) {
		if (c->mon != selmon) /* if the new client c is on a different monitor, */
			selmon = c->mon; /* switch selmon (selected monitor) to that monitor */
		if (c->isurgent) /* if urgent state was marked, */
			seturgent(c, 0); /* clear urgent state */
		detachstack(c);
		attachstack(c); /* move c to top of the stack */
		grabbuttons(c, 1);
		XSetWindowBorder(dpy, c->win, scheme[SchemeSel][ColBorder].pixel);
		setfocus(c); /* inform X11 that c now has input focus */
	} else {
		XSetInputFocus(dpy, selmon->barwin, RevertToPointerRoot, CurrentTime);
		XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
	}
	if(selmon->sel && selmon->sel->isfullscreen){ /* if previous client was fullscreen, toggle off and then back for the new */
		togglefullscreen(NULL);
		selmon->sel = c;
		togglefullscreen(NULL);
	}else{
		selmon->sel = c;
	}
	drawbars(); /* redraw statusbar */
}

/* there are some broken focus acquiring clients needing extra handling */
void
focusin(XEvent *e)
{ /* when a window receives focus */
	XFocusChangeEvent *ev = &e->xfocus;

	if (selmon->sel && ev->window != selmon->sel->win)
		setfocus(selmon->sel);
}

void
focusmaster(const Arg *arg)
{
	Client *master;

	if (selmon->nmaster > 1)
		return;
	if (!selmon->sel || (selmon->sel->isfullscreen && lockfullscreen))
		return;

	master = nexttiled(selmon->clients);

	if (!master)
		return;

	int i;
	for (i = 0; !(selmon->tagset[selmon->seltags] & 1 << i); i++);
	i++;

	if (selmon->sel == master) {
		if (selmon->tagmarked[i] && ISVISIBLE(selmon->tagmarked[i]))
			focus(selmon->tagmarked[i]);
	} else {
		selmon->tagmarked[i] = selmon->sel;
		focus(master);
	}
}

void
focusmon(const Arg *arg)
{
	Monitor *m;

	if (!mons->next)
		return;
	if ((m = dirtomon(arg->i)) == selmon)
		return;
	unfocus(selmon->sel, 0);
	XWarpPointer(dpy, None, m->barwin, 0, 0, 0, 0, m->mw / 2, m->mh / 2);
	selmon = m;
	focus(NULL);
}

void
focusstack(const Arg *arg)
{
	int i = stackpos(arg);
	Client *c, *p;

	if(i < 0)
		return;
	for(p = NULL, c = selmon->clients; c && (i || !ISVISIBLE(c));
	    i -= ISVISIBLE(c) ? 1 : 0, p = c, c = c->next);
	focus(c ? c : p);
	restack(selmon);
}

Atom
getatomprop(Client *c, Atom prop)
{
	int di;
	unsigned long dl;
	unsigned char *p = NULL;
	Atom da, atom = None;

	if (XGetWindowProperty(dpy, c->win, prop, 0L, sizeof atom, False, XA_ATOM,
		&da, &di, &dl, &dl, &p) == Success && p) {
		atom = *(Atom *)p;
		XFree(p);
	}
	return atom;
}


pid_t
getstatusbarpid(void)
{
	char buf[32], *str = buf, *c;
	FILE *fp;

	if (statuspid > 0) {
		snprintf(buf, sizeof(buf), "/proc/%u/cmdline", statuspid);
		if ((fp = fopen(buf, "r"))) {
			fgets(buf, sizeof(buf), fp);
			while ((c = strchr(str, '/')))
				str = c + 1;
			fclose(fp);
			if (!strcmp(str, STATUSBAR))
				return statuspid;
		}
	}

#ifdef __FreeBSD__
	if (!(fp = popen("pgrep "STATUSBAR, "r")))
#else
	if (!(fp = popen("pidof -s "STATUSBAR, "r")))
#endif
		return -1;


	fgets(buf, sizeof(buf), fp);
	pclose(fp);
	return strtol(buf, NULL, 10);
}


int
getrootptr(int *x, int *y)
{
	int di;
	unsigned int dui;
	Window dummy;

	return XQueryPointer(dpy, root, &dummy, &dummy, x, y, &di, &di, &dui);
}

long
getstate(Window w)
{
	int format;
	long result = -1;
	unsigned char *p = NULL;
	unsigned long n, extra;
	Atom real;

	if (XGetWindowProperty(dpy, w, wmatom[WMState], 0L, 2L, False, wmatom[WMState],
		&real, &format, &n, &extra, (unsigned char **)&p) != Success)
		return -1;
	if (n != 0)
		result = *p;
	XFree(p);
	return result;
}

int
gettextprop(Window w, Atom atom, char *text, unsigned int size)
{
	char **list = NULL;
	int n;
	XTextProperty name;

	if (!text || size == 0)
		return 0;
	text[0] = '\0';
	if (!XGetTextProperty(dpy, w, &name, atom) || !name.nitems)
		return 0;
	if (name.encoding == XA_STRING) {
		strncpy(text, (char *)name.value, size - 1);
	} else if (XmbTextPropertyToTextList(dpy, &name, &list, &n) >= Success && n > 0 && *list) {
		strncpy(text, *list, size - 1);
		XFreeStringList(list);
	}
	text[size - 1] = '\0';
	XFree(name.value);
	return 1;
}

void
grabbuttons(Client *c, int focused)
{
	updatenumlockmask();
	{
		unsigned int i, j;
		unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };
		XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
		if (!focused)
			XGrabButton(dpy, AnyButton, AnyModifier, c->win, False,
				BUTTONMASK, GrabModeSync, GrabModeSync, None, None);
		for (i = 0; i < LENGTH(buttons); i++)
			if (buttons[i].click == ClkClientWin)
				for (j = 0; j < LENGTH(modifiers); j++)
					XGrabButton(dpy, buttons[i].button,
						buttons[i].mask | modifiers[j],
						c->win, False, BUTTONMASK,
						GrabModeAsync, GrabModeSync, None, None);
	}
}

void
grabkeys(void)
{
	updatenumlockmask();
	{
		unsigned int i, j, k;
		unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };
		int start, end, skip;
		KeySym *syms;

		XUngrabKey(dpy, AnyKey, AnyModifier, root);
		XDisplayKeycodes(dpy, &start, &end);
		syms = XGetKeyboardMapping(dpy, start, end - start + 1, &skip);
		if (!syms)
			return;
		for (k = start; k <= end; k++)
			for (i = 0; i < LENGTH(keys); i++)
				/* skip modifier codes, we do that ourselves */
				if (keys[i].keysym == syms[(k - start) * skip])
					for (j = 0; j < LENGTH(modifiers); j++)
						XGrabKey(dpy, k,
							 keys[i].mod | modifiers[j],
							 root, True,
							 GrabModeAsync, GrabModeAsync);
		XFree(syms);
	}
}

void
ifroot(const Arg *arg)
{
	TwoFuncPtr *funcs = (TwoFuncPtr*)arg->v;
	if (!selmon->sel) { /*no client -> root window*/
		funcs->func1(&(funcs->arg1));
		return;
	} /*client window*/
	funcs->func2(&(funcs->arg2));
}


void
incnmaster(const Arg *arg)
{
	selmon->nmaster = MAX(selmon->nmaster + arg->i, 0);
	arrange(selmon);
}

#ifdef XINERAMA
static int
isuniquegeom(XineramaScreenInfo *unique, size_t n, XineramaScreenInfo *info)
{
	while (n--)
		if (unique[n].x_org == info->x_org && unique[n].y_org == info->y_org
		&& unique[n].width == info->width && unique[n].height == info->height)
			return 0;
	return 1;
}
#endif /* XINERAMA */

void
keypress(XEvent *e) /* receives a keyboard press XEvent from X11 */
{
	unsigned int i; /* will loop over keybinds */
	KeySym keysym; /* symbolic name, like XK_b or XK_space */
	XKeyEvent *ev; /* pointer to the key event data */

	ev = &e->xkey; /* grabs the XKeyEvent from XEvent */
	keysym = XKeycodeToKeysym(dpy, (KeyCode)ev->keycode, 0); /* translates raw keycode into symbolic keysym */
	for (i = 0; i < LENGTH(keys); i++) /* loop thru all binds in keys[] (config.h) */
		if (keysym == keys[i].keysym
		&& CLEANMASK(keys[i].mod) == CLEANMASK(ev->state)
		&& keys[i].func) /* check that a pointer function exists */
			keys[i].func(&(keys[i].arg)); /* if the pressed key matches a bind, call the function associated w/ that bind */
}

void
killthis(Client *c) {
	if (!sendevent(c, wmatom[WMDelete])) {
		XGrabServer(dpy);
		XSetErrorHandler(xerrordummy);
		XSetCloseDownMode(dpy, DestroyAll);
		XKillClient(dpy, c->win);
		XSync(dpy, False);
		XSetErrorHandler(xerror);
		XUngrabServer(dpy);
	}
}

void
killclient(const Arg *arg)
{
    Client *c;

	if (!selmon->sel)
		return;

    if (!arg->ui || arg->ui == 0) {
        killthis(selmon->sel);
        return;
    }

    for (c = selmon->clients; c; c = c->next) {
        if (!ISVISIBLE(c) || (arg->ui == 1 && c == selmon->sel))
            continue;
        killthis(c);
    }
}

void
quitprompt(const Arg *arg)
{
	FILE *pp = popen("echo -e \"no\nrestart\nyes\" | dmenu -i -sb red -p \"Quit DWM?\"", "r");
	if(pp != NULL) {
		char buf[1024];
		if (fgets(buf, sizeof(buf), pp) == NULL) {
			fprintf(stderr, "Quitprompt: Error reading pipe!\n");
			return;
		}
		if (strcmp(buf, "yes\n") == 0) {
			pclose(pp);
			restart = 0;
			quit(NULL);
		} else if (strcmp(buf, "restart\n") == 0) {
			pclose(pp);
			restart = 1;
			quit(NULL);
		} else if (strcmp(buf, "no\n") == 0) {
			pclose(pp);
			return;
		}
	}
}

void
loadxrdb(void)
{
  Display *display;
  char * resm;
  XrmDatabase xrdb;
  char *type;
  XrmValue value;

  display = XOpenDisplay(NULL);

  if (display != NULL) {
    resm = XResourceManagerString(display);

    if (resm != NULL) {
      xrdb = XrmGetStringDatabase(resm);

      if (xrdb != NULL) {
        XRDB_LOAD_COLOR("dwm.normbordercolor", normbordercolor);
        XRDB_LOAD_COLOR("dwm.normbgcolor", normbgcolor);
        XRDB_LOAD_COLOR("dwm.normfgcolor", normfgcolor);
        XRDB_LOAD_COLOR("dwm.selbordercolor", selbordercolor);
        XRDB_LOAD_COLOR("dwm.selbgcolor", selbgcolor);
        XRDB_LOAD_COLOR("dwm.selfgcolor", selfgcolor);
      }
    }
  }

  XCloseDisplay(display);
}

/**
 * @brief Manage a new window and integrate it into the window management system
 * @param w Window ID of the new window to manage
 * @param wa Pointer to XWindowAttributes containing initial window properties
 * 
 * This function is called when a new window requests to be mapped. It creates a Client
 * structure to represent the window, applies configuration rules, handles window swallowing,
 * and integrates the window into the appropriate monitor's client list and stack.
 * 
 * The function performs several critical operations:
 * 1. Creates and initializes a Client structure
 * 2. Extracts window properties (class, title, PID)
 * 3. Applies matching rules from config.h for tags, floating state, etc.
 * 4. Handles transient windows (inherit parent's properties)
 * 5. Implements window swallowing for terminal-child relationships
 * 6. Ensures window fits within monitor bounds
 * 7. Sets up event masks and window properties
 * 8. Maps the window and potentially performs swallowing
 * 
 * @note This is the primary entry point for all new windows in DWM
 * @note Window swallowing allows terminals to "swallow" their child processes
 * @note Transient windows inherit properties from their parent window
 * 
 * @warning COMPLEXITY: This function performs many operations and has complex
 *          control flow that can be difficult to debug
 * @warning PERFORMANCE: Multiple function calls in sequence may impact startup time
 * @warning MEMORY: Client structure allocation could fail if system is out of memory
 * @warning RACE CONDITIONS: Window positioning calculations assume certain monitor
 *          properties are valid
 * @warning SIDE EFFECTS: Window swallowing can cause unexpected behavior with
 *          some applications
 * 
 * @return void
 */
void
manage(Window w, XWindowAttributes *wa)
{ /* after a window is passed from maprequest, manage determines how to treat it and add it to the stack */
	Client *c, *t = NULL, *term = NULL;
	Window trans = None;
	XWindowChanges wc;

	c = ecalloc(1, sizeof(Client)); /* allocate and initialize a new Client struct to represent the window */
	c->win = w;
	c->pid = winpid(w); /* pid used for things like swallowing */
	/* geometry using XWindowAttributes */
	c->x = c->oldx = wa->x;
	c->y = c->oldy = wa->y;
	c->w = c->oldw = wa->width;
	c->h = c->oldh = wa->height;
	c->oldbw = wa->border_width;

	updatetitle(c); /* grabs title */
	if (XGetTransientForHint(dpy, w, &trans) && (t = wintoclient(trans))) { /* if transient, it inherits tags+mon of parent */
		c->mon = t->mon;
		c->tags = t->tags;
	} else {
		c->mon = selmon; /* assigns to selected monitor */
		term = termforwin(c); /* tries to find a terminal for swallowing */
		applyrules(c); /* apply any matching tag rules from config.h */
	}
		/* ensure window fits to visible bounds of monitor */
	if (c->x + WIDTH(c) > c->mon->wx + c->mon->ww)
		c->x = c->mon->wx + c->mon->ww - WIDTH(c);
	if (c->y + HEIGHT(c) > c->mon->wy + c->mon->wh)
		c->y = c->mon->wy + c->mon->wh - HEIGHT(c);
	c->x = MAX(c->x, c->mon->wx);
	c->y = MAX(c->y, c->mon->wy);
	c->bw = borderpx;

	wc.border_width = c->bw;
	XConfigureWindow(dpy, w, CWBorderWidth, &wc);
	XSetWindowBorder(dpy, w, scheme[SchemeNorm][ColBorder].pixel);
	configure(c); /* propagates border_width, if size doesn't change */
	updatewindowtype(c);
	updatesizehints(c);
	updatewmhints(c);
	c->sfx = c->x;
	c->sfy = c->y;
	c->sfw = c->w;
	c->sfh = c->h;
	c->x = c->mon->mx + (c->mon->mw - WIDTH(c)) / 2;
	c->y = c->mon->my + (c->mon->mh - HEIGHT(c)) / 2;
	XSelectInput(dpy, w, EnterWindowMask|FocusChangeMask|PropertyChangeMask|StructureNotifyMask);
	grabbuttons(c, 0);
	if (!c->isfloating)
		c->isfloating = c->oldstate = trans != None || c->isfixed;
	if (c->isfloating)
		XRaiseWindow(dpy, c->win);
	attach(c); /* add to the monitor's client list */
	attachstack(c); /* add it to the stack */
	XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32, PropModeAppend, /* add to X11 client list */
		(unsigned char *) &(c->win), 1);
	XMoveResizeWindow(dpy, c->win, c->x + 2 * sw, c->y, c->w, c->h); /* some windows require this */
	setclientstate(c, NormalState);
	if(selmon->sel && selmon->sel->isfullscreen && !c->isfloating) /* if a fullscreen window was focused, toggle fullscreen */
		setfullscreen(selmon->sel, 0);
	if (c->mon == selmon)
		unfocus(selmon->sel, 0); /* unfocus other monitor if new window is on current monitor */
	c->mon->sel = c;
	arrange(c->mon); /* recalc based on layout */
	XMapWindow(dpy, c->win);
	if (term)
		swallow(term, c); /* if new window is child of a terminal, replace terminal (swallow) */
	focus(NULL); /* focus the client */
}

void
mappingnotify(XEvent *e)
{ /* keyboard mapping change */
	XMappingEvent *ev = &e->xmapping;

	XRefreshKeyboardMapping(ev);
	if (ev->request == MappingKeyboard)
		grabkeys();
}

void
maprequest(XEvent *e)
{ /* window requests map */
	static XWindowAttributes wa; /* wa will now hold window attributes */
	XMapRequestEvent *ev = &e->xmaprequest;

	if (!XGetWindowAttributes(dpy, ev->window, &wa) || wa.override_redirect)
		return; /* gets window geom and data */
	if (!wintoclient(ev->window)) /* if window is not already managed by dwm */
		manage(ev->window, &wa); /* call manage to start managing it */
}

void
monocle(Monitor *m)
{
	unsigned int n = 0;
	Client *c;

	for (c = m->clients; c; c = c->next)
		if (ISVISIBLE(c))
			n++;
	if (n > 0) /* override layout symbol */
		snprintf(m->ltsymbol, sizeof m->ltsymbol, "[%d]", n);
	for (c = nexttiled(m->clients); c; c = nexttiled(c->next))
		resize(c, m->wx, m->wy, m->ww - 2 * c->bw, m->wh - 2 * c->bw, 0);
}

void
motionnotify(XEvent *e)
{
	static Monitor *mon = NULL;
	Monitor *m;
	XMotionEvent *ev = &e->xmotion;

	if (ev->window != root)
		return;
	if ((m = recttomon(ev->x_root, ev->y_root, 1, 1)) != mon && mon) {
		unfocus(selmon->sel, 1);
		selmon = m;
		focus(NULL);
	}
	mon = m;
}

void
movemouse(const Arg *arg)
{
	int x, y, ocx, ocy, nx, ny;
	Client *c;
	Monitor *m;
	XEvent ev;
	Time lasttime = 0;

	if (!(c = selmon->sel))
		return;
	if (c->isfullscreen) /* no support moving fullscreen windows by mouse */
		return;
	restack(selmon);
	ocx = c->x;
	ocy = c->y;
	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
		None, cursor[CurMove]->cursor, CurrentTime) != GrabSuccess)
		return;
	if (!getrootptr(&x, &y))
		return;
	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch(ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (1000 / 60))
				continue;
			lasttime = ev.xmotion.time;

			nx = ocx + (ev.xmotion.x - x);
			ny = ocy + (ev.xmotion.y - y);
			if (abs(selmon->wx - nx) < snap)
				nx = selmon->wx;
			else if (abs((selmon->wx + selmon->ww) - (nx + WIDTH(c))) < snap)
				nx = selmon->wx + selmon->ww - WIDTH(c);
			if (abs(selmon->wy - ny) < snap)
				ny = selmon->wy;
			else if (abs((selmon->wy + selmon->wh) - (ny + HEIGHT(c))) < snap)
				ny = selmon->wy + selmon->wh - HEIGHT(c);
			if (!c->isfloating && selmon->lt[selmon->sellt]->arrange
			&& (abs(nx - c->x) > snap || abs(ny - c->y) > snap))
				togglefloating(NULL);
			if (!selmon->lt[selmon->sellt]->arrange || c->isfloating)
				resize(c, nx, ny, c->w, c->h, 1);
			break;
		}
	} while (ev.type != ButtonRelease);
	XUngrabPointer(dpy, CurrentTime);
	if ((m = recttomon(c->x, c->y, c->w, c->h)) != selmon) {
		sendmon(c, m);
		selmon = m;
		focus(NULL);
	}
}

Client *
nexttiled(Client *c)
{
	for (; c && (c->isfloating || !ISVISIBLE(c)); c = c->next);
	return c;
}

void
pop(Client *c)
{
	int i;
	for (i = 0; !(selmon->tagset[selmon->seltags] & 1 << i); i++);
	i++;

	c->mon->tagmarked[i] = nexttiled(c->mon->clients);
	detach(c);
	attach(c);
	focus(c);
	arrange(c->mon);
}

void
propertynotify(XEvent *e)
{
	Client *c;
	Window trans;
	XPropertyEvent *ev = &e->xproperty;

	if ((ev->window == root) && (ev->atom == XA_WM_NAME))
		updatestatus();
	else if (ev->state == PropertyDelete)
		return; /* ignore */
	else if ((c = wintoclient(ev->window))) {
		switch(ev->atom) {
		default: break;
		case XA_WM_TRANSIENT_FOR:
			if (!c->isfloating && (XGetTransientForHint(dpy, c->win, &trans)) &&
				(c->isfloating = (wintoclient(trans)) != NULL))
				arrange(c->mon);
			break;
		case XA_WM_NORMAL_HINTS:
			c->hintsvalid = 0;
			break;
		case XA_WM_HINTS:
			updatewmhints(c);
			drawbars();
			break;
		}
		if (ev->atom == XA_WM_NAME || ev->atom == netatom[NetWMName]) {
			updatetitle(c);
			if (c == c->mon->sel && selmon->showtitle)
				drawbar(c->mon);
		}
		if (ev->atom == netatom[NetWMWindowType])
			updatewindowtype(c);
	}
}

void
pushstack(const Arg *arg) {
	int i = stackpos(arg);
	Client *sel = selmon->sel, *c, *p;

	if(i < 0)
		return;
	else if(i == 0) {
		detach(sel);
		attach(sel);
	}
	else {
		for(p = NULL, c = selmon->clients; c; p = c, c = c->next)
			if(!(i -= (ISVISIBLE(c) && c != sel)))
				break;
		c = c ? c : p;
		detach(sel);
		sel->next = c->next;
		c->next = sel;
	}
	arrange(selmon);
}

void
quit(const Arg *arg)
{
	if(arg->i) restart = 1;
	running = 0;
}

Monitor *
recttomon(int x, int y, int w, int h)
{
	Monitor *m, *r = selmon;
	int a, area = 0;

	for (m = mons; m; m = m->next)
		if ((a = INTERSECT(x, y, w, h, m)) > area) {
			area = a;
			r = m;
		}
	return r;
}

void
resize(Client *c, int x, int y, int w, int h, int interact)
{
	if (applysizehints(c, &x, &y, &w, &h, interact))
		resizeclient(c, x, y, w, h);
}

void
resizeclient(Client *c, int x, int y, int w, int h)
{
	XWindowChanges wc;

	c->oldx = c->x; c->x = wc.x = x;
	c->oldy = c->y; c->y = wc.y = y;
	c->oldw = c->w; c->w = wc.width = w;
	c->oldh = c->h; c->h = wc.height = h;
	if (c->isfloating && c->hasfloatbw && !c->isfullscreen)
		wc.border_width = c->floatborderpx;
	else
		wc.border_width = c->bw;
	XConfigureWindow(dpy, c->win, CWX|CWY|CWWidth|CWHeight|CWBorderWidth, &wc);
	configure(c);
	XSync(dpy, False);
}

void
resizemouse(const Arg *arg)
{
	int ocx, ocy, nw, nh;
	Client *c;
	Monitor *m;
	XEvent ev;
	Time lasttime = 0;

	if (!(c = selmon->sel))
		return;
	if (c->isfullscreen) /* no support resizing fullscreen windows by mouse */
		return;
	restack(selmon);
	ocx = c->x;
	ocy = c->y;
	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
		None, cursor[CurResize]->cursor, CurrentTime) != GrabSuccess)
		return;
	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);
	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch(ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (1000 / 60))
				continue;
			lasttime = ev.xmotion.time;

			nw = MAX(ev.xmotion.x - ocx - 2 * c->bw + 1, 1);
			nh = MAX(ev.xmotion.y - ocy - 2 * c->bw + 1, 1);
			if (c->mon->wx + nw >= selmon->wx && c->mon->wx + nw <= selmon->wx + selmon->ww
			&& c->mon->wy + nh >= selmon->wy && c->mon->wy + nh <= selmon->wy + selmon->wh)
			{
				if (!c->isfloating && selmon->lt[selmon->sellt]->arrange
				&& (abs(nw - c->w) > snap || abs(nh - c->h) > snap))
					togglefloating(NULL);
			}
			if (!selmon->lt[selmon->sellt]->arrange || c->isfloating)
				resize(c, c->x, c->y, nw, nh, 1);
			break;
		}
	} while (ev.type != ButtonRelease);
	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);
	XUngrabPointer(dpy, CurrentTime);
	while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));
	if ((m = recttomon(c->x, c->y, c->w, c->h)) != selmon) {
		sendmon(c, m);
		selmon = m;
		focus(NULL);
	}
}

void
restack(Monitor *m)
{
	Client *c;
	XEvent ev;
	XWindowChanges wc;

	drawbar(m);
	if (!m->sel)
		return;
	if (m->sel->isfloating || !m->lt[m->sellt]->arrange)
		XRaiseWindow(dpy, m->sel->win);
	if (m->lt[m->sellt]->arrange) {
		wc.stack_mode = Below;
		wc.sibling = m->barwin;
		for (c = m->stack; c; c = c->snext)
			if (!c->isfloating && ISVISIBLE(c)) {
				XConfigureWindow(dpy, c->win, CWSibling|CWStackMode, &wc);
				wc.sibling = c->win;
			}
	}
	XSync(dpy, False);
	while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

void
run(void)
{
	XEvent ev; /* store any type of X11 event received - keypress, mouse, etc. */
	/* main event loop */
	XSync(dpy, False); /* all pending X11 requests sent to X server */
	while (running && !XNextEvent(dpy, &ev)) /* keep running while dwm is supposed to be running, and keep handling events from X */
		if (handler[ev.type]) /* check if there's a handler for the event type */
			handler[ev.type](&ev); /* call that handler and pass it the event data */
}

void
scan(void)
{
	unsigned int i, num;
	Window d1, d2, *wins = NULL;
	XWindowAttributes wa;

	if (XQueryTree(dpy, root, &d1, &d2, &wins, &num)) {
		for (i = 0; i < num; i++) {
			if (!XGetWindowAttributes(dpy, wins[i], &wa)
			|| wa.override_redirect || XGetTransientForHint(dpy, wins[i], &d1))
				continue;
			if (wa.map_state == IsViewable || getstate(wins[i]) == IconicState)
				manage(wins[i], &wa);
		}
		for (i = 0; i < num; i++) { /* now the transients */
			if (!XGetWindowAttributes(dpy, wins[i], &wa))
				continue;
			if (XGetTransientForHint(dpy, wins[i], &d1)
			&& (wa.map_state == IsViewable || getstate(wins[i]) == IconicState))
				manage(wins[i], &wa);
		}
		if (wins)
			XFree(wins);
	}
}

void
sendmon(Client *c, Monitor *m)
{
	if (c->mon == m)
		return;
	unfocus(c, 1);
	detach(c);
	detachstack(c);
	c->mon = m;
	c->tags = m->tagset[m->seltags]; /* assign tags of target monitor */
	attach(c);
	attachstack(c);
	focus(NULL);
	arrange(NULL);
}

void
setclientstate(Client *c, long state)
{
	long data[] = { state, None };

	XChangeProperty(dpy, c->win, wmatom[WMState], wmatom[WMState], 32,
		PropModeReplace, (unsigned char *)data, 2);
}

int
sendevent(Client *c, Atom proto)
{
	int n;
	Atom *protocols;
	int exists = 0;
	XEvent ev;

	if (XGetWMProtocols(dpy, c->win, &protocols, &n)) {
		while (!exists && n--)
			exists = protocols[n] == proto;
		XFree(protocols);
	}
	if (exists) {
		ev.type = ClientMessage;
		ev.xclient.window = c->win;
		ev.xclient.message_type = wmatom[WMProtocols];
		ev.xclient.format = 32;
		ev.xclient.data.l[0] = proto;
		ev.xclient.data.l[1] = CurrentTime;
		XSendEvent(dpy, c->win, False, NoEventMask, &ev);
	}
	return exists;
}

void
setfocus(Client *c)
{
	if (!c->neverfocus) {
		XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
		XChangeProperty(dpy, root, netatom[NetActiveWindow],
			XA_WINDOW, 32, PropModeReplace,
			(unsigned char *) &(c->win), 1);
	}
	sendevent(c, wmatom[WMTakeFocus]);
}

void
setfullscreen(Client *c, int fullscreen)
{
	if (fullscreen && !c->isfullscreen) {
		XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
			PropModeReplace, (unsigned char*)&netatom[NetWMFullscreen], 1);
		c->isfullscreen = 1;
		c->oldstate = c->isfloating;
		c->oldbw = c->bw;
		c->bw = 0;
		c->isfloating = 1;
		resizeclient(c, c->mon->mx, c->mon->my, c->mon->mw, c->mon->mh);
		XRaiseWindow(dpy, c->win);
	} else if (!fullscreen && c->isfullscreen){
		XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
			PropModeReplace, (unsigned char*)0, 0);
		c->isfullscreen = 0;
		c->isfloating = c->oldstate;
		c->bw = c->oldbw;
		c->x = c->oldx;
		c->y = c->oldy;
		c->w = c->oldw;
		c->h = c->oldh;
		resizeclient(c, c->x, c->y, c->w, c->h);
		arrange(c->mon);
	}
}



void
	 setsticky(Client *c, int sticky)
	 {

		 if(sticky && !c->issticky) {
			 XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
					 PropModeReplace, (unsigned char *) &netatom[NetWMSticky], 1);
			 c->issticky = 1;
		 } else if(!sticky && c->issticky){
			 XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
					 PropModeReplace, (unsigned char *)0, 0);
			 c->issticky = 0;
			 arrange(c->mon);
		 }
	 }


void
setlayout(const Arg *arg)
{
	if (!arg || !arg->v || arg->v != selmon->lt[selmon->sellt])
		selmon->sellt ^= 1;
	if (arg && arg->v)
		selmon->lt[selmon->sellt] = (Layout *)arg->v;
	strncpy(selmon->ltsymbol, selmon->lt[selmon->sellt]->symbol, sizeof selmon->ltsymbol);
	if (selmon->sel)
		arrange(selmon);
	else
		drawbar(selmon);
}

/* arg > 1.0 will set mfact absolutely */
void
setmfact(const Arg *arg)
{
	float f;

	if (!arg || !selmon->lt[selmon->sellt]->arrange)
		return;
	f = arg->f < 1.0 ? arg->f + selmon->mfact : arg->f - 1.0;
	if (f < 0.05 || f > 0.95)
		return;
	selmon->mfact = f;
	arrange(selmon);
}

void
setup(void)
{
	int i;
	XSetWindowAttributes wa;
	Atom utf8string;
	struct sigaction sa;

	/* do not transform children into zombies when they terminate */
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_NOCLDSTOP | SA_NOCLDWAIT | SA_RESTART;
	sa.sa_handler = SIG_IGN;
	sigaction(SIGCHLD, &sa, NULL);

	/* clean up any zombies (inherited from .xinitrc etc) immediately */
	while (waitpid(-1, NULL, WNOHANG) > 0);

	signal(SIGHUP, sighup);
	signal(SIGTERM, sigterm);

	/* init screen */
	screen = DefaultScreen(dpy); /* gets the default screen number from the display (dpy) */
	sw = DisplayWidth(dpy, screen);
	sh = DisplayHeight(dpy, screen);
	root = RootWindow(dpy, screen); /* dwm attaches to the root window so it can recieve global events */
	drw = drw_create(dpy, screen, root, sw, sh); /* a drawing context for rendering the UI (like the bar) */
	if (!drw_fontset_create(drw, fonts, LENGTH(fonts)))
		die("no fonts could be loaded.");
	lrpad = drw->fonts->h; /* sets lrpad to the height of the font, for correct spacing in bar */
	bh = drw->fonts->h + 2;
	updategeom(); /* setting up monitor geometry, if using multiple monitors, creates a linked list of monitor structs */
	/* init X11 atoms */
	/* this is asking the X server for an atom (a unique integer) corresponding to each string */
	/* if it exists already, the atom ID is returned */
	utf8string = XInternAtom(dpy, "UTF8_STRING", False);
	wmatom[WMProtocols] = XInternAtom(dpy, "WM_PROTOCOLS", False);
	wmatom[WMDelete] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	wmatom[WMState] = XInternAtom(dpy, "WM_STATE", False);
	wmatom[WMTakeFocus] = XInternAtom(dpy, "WM_TAKE_FOCUS", False);
	netatom[NetActiveWindow] = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
	netatom[NetSupported] = XInternAtom(dpy, "_NET_SUPPORTED", False);
	netatom[NetWMName] = XInternAtom(dpy, "_NET_WM_NAME", False);
	netatom[NetWMState] = XInternAtom(dpy, "_NET_WM_STATE", False);
	netatom[NetWMCheck] = XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", False);
	netatom[NetWMFullscreen] = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);
	netatom[NetWMSticky] = XInternAtom(dpy, "_NET_WM_STATE_STICKY", False);
	netatom[NetWMWindowType] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
	netatom[NetWMWindowTypeDialog] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", False);
	netatom[NetClientList] = XInternAtom(dpy, "_NET_CLIENT_LIST", False);
	/* init cursors */
	cursor[CurNormal] = drw_cur_create(drw, XC_left_ptr);
	cursor[CurResize] = drw_cur_create(drw, XC_sizing);
	cursor[CurMove] = drw_cur_create(drw, XC_fleur);
	/* init appearance */
	scheme = ecalloc(LENGTH(colors), sizeof(Clr *)); /* allocates memory to set up colorschemes */
	for (i = 0; i < LENGTH(colors); i++)
		scheme[i] = drw_scm_create(drw, colors[i], 3); /* each scheme[i] corresponds to a SchemeNorm, SchemeSel, etc */
	/* init bars */
	updatebars(); /* creates a bar window for each monitor */
	updatestatus(); /* renders the status text */
	/* supporting window for NetWMCheck */
	wmcheckwin = XCreateSimpleWindow(dpy, root, 0, 0, 1, 1, 0, 0, 0); /* a dummy window created so applications can detect the WM */
	/* setting X11 properties so applications know that dwm is the window manager, and supports EWMH hints */
	XChangeProperty(dpy, wmcheckwin, netatom[NetWMCheck], XA_WINDOW, 32,
		PropModeReplace, (unsigned char *) &wmcheckwin, 1);
	XChangeProperty(dpy, wmcheckwin, netatom[NetWMName], utf8string, 8,
		PropModeReplace, (unsigned char *) "dwm", 3);
	XChangeProperty(dpy, root, netatom[NetWMCheck], XA_WINDOW, 32,
		PropModeReplace, (unsigned char *) &wmcheckwin, 1);
	/* EWMH support per view */
	XChangeProperty(dpy, root, netatom[NetSupported], XA_ATOM, 32,
		PropModeReplace, (unsigned char *) netatom, NetLast);
	XDeleteProperty(dpy, root, netatom[NetClientList]);
	/* select events */
	wa.cursor = cursor[CurNormal]->cursor;
	wa.event_mask = SubstructureRedirectMask|SubstructureNotifyMask
		|ButtonPressMask|PointerMotionMask|EnterWindowMask
		|LeaveWindowMask|StructureNotifyMask|PropertyChangeMask; /* X11 events that dwm will listen for */
	XChangeWindowAttributes(dpy, root, CWEventMask|CWCursor, &wa); /* applies the event mask and cursor to the root window */
	XSelectInput(dpy, root, wa.event_mask);
	grabkeys(); /* register the keybinds in config.h */
	focus(NULL); /* focus needs to start cleanly */
}

void
seturgent(Client *c, int urg)
{
	XWMHints *wmh;

	c->isurgent = urg;
	if (!(wmh = XGetWMHints(dpy, c->win)))
		return;
	wmh->flags = urg ? (wmh->flags | XUrgencyHint) : (wmh->flags & ~XUrgencyHint);
	XSetWMHints(dpy, c->win, wmh);
	XFree(wmh);
}

void
showhide(Client *c)
{
	if (!c)
		return;
	if (ISVISIBLE(c)) {
		/* show clients top down */
		XMoveWindow(dpy, c->win, c->x, c->y);
		if ((!c->mon->lt[c->mon->sellt]->arrange || c->isfloating) && !c->isfullscreen)
			resize(c, c->x, c->y, c->w, c->h, 0);
		showhide(c->snext);
	} else {
		/* hide clients bottom up */
		showhide(c->snext);
		XMoveWindow(dpy, c->win, WIDTH(c) * -2, c->y);
	}
}


void
sigstatusbar(const Arg *arg)
{
	union sigval sv;

	if (!statussig)
		return;
	sv.sival_int = arg->i;
	if ((statuspid = getstatusbarpid()) <= 0)
		return;

	sigqueue(statuspid, SIGRTMIN+statussig, sv);
}


void
sighup(int unused)
{
	Arg a = {.i = 1};
	quit(&a);
}

void
sigterm(int unused)
{
	Arg a = {.i = 0};
	quit(&a);
}

void
spawn(const Arg *arg)
{ /* runs shell commands, ie. launching programs from keybinds */
	struct sigaction sa;

	if (arg->v == dmenucmd)
		dmenumon[0] = '0' + selmon->num;
	if (fork() == 0) {
		if (dpy)
			close(ConnectionNumber(dpy));
		setsid();

		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;
		sa.sa_handler = SIG_DFL;
		sigaction(SIGCHLD, &sa, NULL);

		execvp(((char **)arg->v)[0], (char **)arg->v);
		die("dwm: execvp '%s' failed:", ((char **)arg->v)[0]);
	}
}

void spawnscratch(const Arg *arg)
{
	struct sigaction sa;

	if (fork() == 0) {
		if (dpy)
			close(ConnectionNumber(dpy));
		setsid();

		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;
		sa.sa_handler = SIG_DFL;
		sigaction(SIGCHLD, &sa, NULL);

		execvp(((char **)arg->v)[1], ((char **)arg->v)+1);
		fprintf(stderr, "dwm: execvp %s", ((char **)arg->v)[1]);
		perror(" failed");
		exit(EXIT_SUCCESS);
	}
}

int
stackpos(const Arg *arg) {
	int n, i;
	Client *c, *l;

	if(!selmon->clients)
		return -1;

	if(arg->i == PREVSEL) {
		for(l = selmon->stack; l && (!ISVISIBLE(l) || l == selmon->sel); l = l->snext);
		if(!l)
			return -1;
		for(i = 0, c = selmon->clients; c != l; i += ISVISIBLE(c) ? 1 : 0, c = c->next);
		return i;
	}
	else if(ISINC(arg->i)) {
		if(!selmon->sel)
			return -1;
		for(i = 0, c = selmon->clients; c != selmon->sel; i += ISVISIBLE(c) ? 1 : 0, c = c->next);
		for(n = i; c; n += ISVISIBLE(c) ? 1 : 0, c = c->next);
		return MOD(i + GETINC(arg->i), n);
	}
	else if(arg->i < 0) {
		for(i = 0, c = selmon->clients; c; i += ISVISIBLE(c) ? 1 : 0, c = c->next);
		return MAX(i + arg->i, 0);
	}
	else
		return arg->i;
}

void
tag(const Arg *arg)
{
	if (selmon->sel && arg->ui & TAGMASK) {
		selmon->sel->tags = arg->ui & TAGMASK;
		focus(NULL);
		arrange(selmon);
	}
}

void
spawntag(const Arg *arg)
{
	if (arg->ui & TAGMASK) {
		for (int i = LENGTH(tags); i >= 0; i--) {
			if (arg->ui & 1<<i) {
				spawn(&tagexec[i]);
				return;
			}
		}
	}
}

void
tagmon(const Arg *arg)
{
	if (!selmon->sel || !mons->next)
		return;
	sendmon(selmon->sel, dirtomon(arg->i));
}

/* layouts no longer needed here because of vanitygaps.c */

void
togglebar(const Arg *arg)
{
	selmon->showbar = !selmon->showbar;
	updatebarpos(selmon);
	XMoveResizeWindow(dpy, selmon->barwin, selmon->wx, selmon->by, selmon->ww, bh);
	arrange(selmon);
}

void
togglebarcolor(const Arg *arg)
{ /* this really needs to be integrated with the other toggle bar stuff */
	Clr tmp;
	tmp = scheme[SchemeTagsNorm][ColBorder];
	scheme[SchemeTagsNorm][ColBorder] = scheme[SchemeTagsNorm][ColBg];
	scheme[SchemeTagsNorm][ColBg] = tmp;
	tmp = scheme[SchemeTagsSel][ColFg];
	scheme[SchemeTagsSel][ColFg] = scheme[SchemeTagsSel][ColBg];
	scheme[SchemeTagsSel][ColBg] = tmp;
	tmp = scheme[SchemeSel][ColFg];
	scheme[SchemeSel][ColFg] = scheme[SchemeSel][ColBg];
	scheme[SchemeSel][ColBg] = tmp;
	arrange(selmon);
}

void
togglebartags(const Arg *arg)
{
    selmon->showtags = !selmon->showtags;
	arrange(selmon);
}

void
togglebartitle(const Arg *arg)
{
    selmon->showtitle = !selmon->showtitle;
	arrange(selmon);
}

void
togglebarlt(const Arg *arg)
{
    selmon->showlayout = !selmon->showlayout;
	arrange(selmon);
}

void
togglebarstatus(const Arg *arg)
{
    selmon->showstatus = !selmon->showstatus;
	arrange(selmon);
}

void
togglebarfloat(const Arg *arg)
{
    selmon->showfloating = !selmon->showfloating;
	arrange(selmon);
}

void
togglefloating(const Arg *arg)
{
	if (!selmon->sel)
		return;
	if (selmon->sel->isfullscreen) /* no support for fullscreen windows */
		return;
	selmon->sel->isfloating = !selmon->sel->isfloating || selmon->sel->isfixed;
	if (selmon->sel->isfloating)
		/* restore last known float dimensions */
		resize(selmon->sel, selmon->sel->sfx, selmon->sel->sfy,
		       selmon->sel->sfw, selmon->sel->sfh, False);
	else {
		/* save last known float dimensions */
		selmon->sel->sfx = selmon->sel->x;
		selmon->sel->sfy = selmon->sel->y;
		selmon->sel->sfw = selmon->sel->w;
		selmon->sel->sfh = selmon->sel->h;
	}
	arrange(selmon);
}

void
togglescratch(const Arg *arg)
{
	Client *c;
	unsigned int found = 0;

	for (c = selmon->clients; c && !(found = c->scratchkey == ((char**)arg->v)[0][0]); c = c->next);
	if (found) {
		c->tags = ISVISIBLE(c) ? 0 : selmon->tagset[selmon->seltags];
		focus(NULL);
		arrange(selmon);

		if (ISVISIBLE(c)) {
			focus(c);
			restack(selmon);
		}

	} else{
		spawnscratch(arg);
	}
}

void
togglesticky(const Arg *arg)
{
	if (!selmon->sel)
		return;
	setsticky(selmon->sel, !selmon->sel->issticky);
	arrange(selmon);
}

void
togglefullscreen(const Arg *arg)
{
	if (selmon->sel){
		setfullscreen(selmon->sel, !selmon->sel->isfullscreen);
	}
}

void
toggletag(const Arg *arg)
{
	unsigned int newtags;

	if (!selmon->sel)
		return;
	newtags = selmon->sel->tags ^ (arg->ui & TAGMASK);
	if (newtags) {
		selmon->sel->tags = newtags;
		focus(NULL);
		arrange(selmon);
	}
}

void
toggleview(const Arg *arg)
{
	unsigned int newtagset = selmon->tagset[selmon->seltags] ^ (arg->ui & TAGMASK);

	if (newtagset) {
		selmon->tagset[selmon->seltags] = newtagset;
		focus(NULL);
		arrange(selmon);
	}
}

void
unfocus(Client *c, int setfocus)
{
	if (!c)
		return;
	grabbuttons(c, 0);
	XSetWindowBorder(dpy, c->win, scheme[SchemeNorm][ColBorder].pixel);
	if (setfocus) {
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
		XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
	}
}

void
unmanage(Client *c, int destroyed)
{ /* removing a window from dwm's control */
	Monitor *m = c->mon;
	XWindowChanges wc;
	int fullscreen = (selmon->sel == c && selmon->sel->isfullscreen)?1:0;

	if (c->swallowing) { /* handle swallowing first */
		unswallow(c);
		return;
	}

	Client *s = swallowingclient(c->win);
	if (s) { /* if a window c is being swallowed, it's reversed */
		free(s->swallowing);
		s->swallowing = NULL;
		arrange(m);
		focus(NULL);
		return;
	}

	detach(c); /* remove c from the monitor's client list */
	detachstack(c); /* remove c from the stacking order */
	if (!destroyed) { /* restore X11 properties if not destroyed */
		wc.border_width = c->oldbw;
		XGrabServer(dpy); /* avoid race conditions */
		XSetErrorHandler(xerrordummy);
		XSelectInput(dpy, c->win, NoEventMask);
		XConfigureWindow(dpy, c->win, CWBorderWidth, &wc); /* restore border */
		XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
		setclientstate(c, WithdrawnState);
		XSync(dpy, False);
		XSetErrorHandler(xerror);
		XUngrabServer(dpy);
	}
	free(c); /* free memory */

	if (!s) { /* recalcs layout now that c is gone */
		arrange(m);
		focus(NULL);
	if(fullscreen){ /* if fullscreen, toggle it off */
		togglefullscreen(NULL);
	}
		updateclientlist(); /* update EWMH property */
	}
}

void
unmapnotify(XEvent *e)
{
	Client *c;
	XUnmapEvent *ev = &e->xunmap;

	if ((c = wintoclient(ev->window))) {
		if (ev->send_event)
			setclientstate(c, WithdrawnState);
		else
			unmanage(c, 0);
	}
}

void
updatebars(void)
{
	Monitor *m;
	XSetWindowAttributes wa = {
		.override_redirect = True,
		.background_pixmap = ParentRelative,
		.event_mask = ButtonPressMask|ExposureMask
	};
	XClassHint ch = {"dwm", "dwm"};
	for (m = mons; m; m = m->next) {
		if (m->barwin)
			continue;
		m->barwin = XCreateWindow(dpy, root, m->wx, m->by, m->ww, bh, 0, DefaultDepth(dpy, screen),
				CopyFromParent, DefaultVisual(dpy, screen),
				CWOverrideRedirect|CWBackPixmap|CWEventMask, &wa);
		XDefineCursor(dpy, m->barwin, cursor[CurNormal]->cursor);
		XMapRaised(dpy, m->barwin);
		XSetClassHint(dpy, m->barwin, &ch);
	}
}

void
updatebarpos(Monitor *m)
{
	m->wy = m->my;
	m->wh = m->mh;
	if (m->showbar) {
		m->wh -= bh;
		m->by = m->topbar ? m->wy : m->wy + m->wh;
		m->wy = m->topbar ? m->wy + bh : m->wy;
	} else
		m->by = -bh;
}

void
updateclientlist(void)
{
	Client *c;
	Monitor *m;

	XDeleteProperty(dpy, root, netatom[NetClientList]);
	for (m = mons; m; m = m->next)
		for (c = m->clients; c; c = c->next)
			XChangeProperty(dpy, root, netatom[NetClientList],
				XA_WINDOW, 32, PropModeAppend,
				(unsigned char *) &(c->win), 1);
}

int
updategeom(void)
{
	int dirty = 0;

#ifdef XINERAMA
	if (XineramaIsActive(dpy)) {
		int i, j, n, nn;
		Client *c;
		Monitor *m;
		XineramaScreenInfo *info = XineramaQueryScreens(dpy, &nn);
		XineramaScreenInfo *unique = NULL;

		for (n = 0, m = mons; m; m = m->next, n++);
		/* only consider unique geometries as separate screens */
		unique = ecalloc(nn, sizeof(XineramaScreenInfo));
		for (i = 0, j = 0; i < nn; i++)
			if (isuniquegeom(unique, j, &info[i]))
				memcpy(&unique[j++], &info[i], sizeof(XineramaScreenInfo));
		XFree(info);
		nn = j;

		/* new monitors if nn > n */
		for (i = n; i < nn; i++) {
			for (m = mons; m && m->next; m = m->next);
			if (m)
				m->next = createmon();
			else
				mons = createmon();
		}
		for (i = 0, m = mons; i < nn && m; m = m->next, i++)
			if (i >= n
			|| unique[i].x_org != m->mx || unique[i].y_org != m->my
			|| unique[i].width != m->mw || unique[i].height != m->mh)
			{
				dirty = 1;
				m->num = i;
				m->mx = m->wx = unique[i].x_org;
				m->my = m->wy = unique[i].y_org;
				m->mw = m->ww = unique[i].width;
				m->mh = m->wh = unique[i].height;
				updatebarpos(m);
			}
		/* removed monitors if n > nn */
		for (i = nn; i < n; i++) {
			for (m = mons; m && m->next; m = m->next);
			while ((c = m->clients)) {
				dirty = 1;
				m->clients = c->next;
				detachstack(c);
				c->mon = mons;
				attach(c);
				attachstack(c);
			}
			if (m == selmon)
				selmon = mons;
			cleanupmon(m);
		}
		free(unique);
	} else
#endif /* XINERAMA */
	{ /* default monitor setup */
		if (!mons)
			mons = createmon();
		if (mons->mw != sw || mons->mh != sh) {
			dirty = 1;
			mons->mw = mons->ww = sw;
			mons->mh = mons->wh = sh;
			updatebarpos(mons);
		}
	}
	if (dirty) {
		selmon = mons;
		selmon = wintomon(root);
	}
	return dirty;
}

void
updatenumlockmask(void)
{
	unsigned int i, j;
	XModifierKeymap *modmap;

	numlockmask = 0;
	modmap = XGetModifierMapping(dpy);
	for (i = 0; i < 8; i++)
		for (j = 0; j < modmap->max_keypermod; j++)
			if (modmap->modifiermap[i * modmap->max_keypermod + j]
				== XKeysymToKeycode(dpy, XK_Num_Lock))
				numlockmask = (1 << i);
	XFreeModifiermap(modmap);
}

void
updatesizehints(Client *c)
{
	long msize;
	XSizeHints size;

	if (!XGetWMNormalHints(dpy, c->win, &size, &msize))
		/* size is uninitialized, ensure that size.flags aren't used */
		size.flags = PSize;
	if (size.flags & PBaseSize) {
		c->basew = size.base_width;
		c->baseh = size.base_height;
	} else if (size.flags & PMinSize) {
		c->basew = size.min_width;
		c->baseh = size.min_height;
	} else
		c->basew = c->baseh = 0;
	if (size.flags & PResizeInc) {
		c->incw = size.width_inc;
		c->inch = size.height_inc;
	} else
		c->incw = c->inch = 0;
	if (size.flags & PMaxSize) {
		c->maxw = size.max_width;
		c->maxh = size.max_height;
	} else
		c->maxw = c->maxh = 0;
	if (size.flags & PMinSize) {
		c->minw = size.min_width;
		c->minh = size.min_height;
	} else if (size.flags & PBaseSize) {
		c->minw = size.base_width;
		c->minh = size.base_height;
	} else
		c->minw = c->minh = 0;
	if (size.flags & PAspect) {
		c->mina = (float)size.min_aspect.y / size.min_aspect.x;
		c->maxa = (float)size.max_aspect.x / size.max_aspect.y;
	} else
		c->maxa = c->mina = 0.0;
	c->isfixed = (c->maxw && c->maxh && c->maxw == c->minw && c->maxh == c->minh);
	c->hintsvalid = 1;
}

void
updatestatus(void)
{
	if (!gettextprop(root, XA_WM_NAME, stext, sizeof(stext)) && selmon->showstatus) {
		strcpy(stext, "dwm-"VERSION);
		statusw = TEXTW(stext) - lrpad + 2;
	} else {
		char *text, *s, ch;

		statusw  = 0;
		for (text = s = stext; *s; s++) {
			if ((unsigned char)(*s) < ' ') {
				ch = *s;
				*s = '\0';
				statusw += TEXTW(text) - lrpad;
				*s = ch;
				text = s + 1;
			}
		}
		statusw += TEXTW(text) - lrpad + 2;

	}
	drawbar(selmon);
}

void
updatetitle(Client *c)
{
	if (!gettextprop(c->win, netatom[NetWMName], c->name, sizeof c->name))
		gettextprop(c->win, XA_WM_NAME, c->name, sizeof c->name);
	if (c->name[0] == '\0') /* hack to mark broken clients */
		strcpy(c->name, broken);
}

void
updatewindowtype(Client *c)
{
	Atom state = getatomprop(c, netatom[NetWMState]);
	Atom wtype = getatomprop(c, netatom[NetWMWindowType]);

	if (state == netatom[NetWMFullscreen])
		setfullscreen(c, 1);
	if (state == netatom[NetWMSticky]) {
		setsticky(c, 1);
	}
	if (wtype == netatom[NetWMWindowTypeDialog])
		c->isfloating = 1;
}

void
updatewmhints(Client *c)
{
	XWMHints *wmh;

	if ((wmh = XGetWMHints(dpy, c->win))) {
		if (c == selmon->sel && wmh->flags & XUrgencyHint) {
			wmh->flags &= ~XUrgencyHint;
			XSetWMHints(dpy, c->win, wmh);
		} else
			c->isurgent = (wmh->flags & XUrgencyHint) ? 1 : 0;
		if (wmh->flags & InputHint)
			c->neverfocus = !wmh->input;
		else
			c->neverfocus = 0;
		XFree(wmh);
	}
}

void
view(const Arg *arg)
{
	if ((arg->ui & TAGMASK) == selmon->tagset[selmon->seltags])
		return;
	selmon->seltags ^= 1; /* toggle sel tagset */
	if (arg->ui & TAGMASK)
		selmon->tagset[selmon->seltags] = arg->ui & TAGMASK;
	focus(NULL);
	arrange(selmon);
}

pid_t
winpid(Window w)
{

	pid_t result = 0;

#ifdef __linux__
	xcb_res_client_id_spec_t spec = {0};
	spec.client = w;
	spec.mask = XCB_RES_CLIENT_ID_MASK_LOCAL_CLIENT_PID;

	xcb_generic_error_t *e = NULL;
	xcb_res_query_client_ids_cookie_t c = xcb_res_query_client_ids(xcon, 1, &spec);
	xcb_res_query_client_ids_reply_t *r = xcb_res_query_client_ids_reply(xcon, c, &e);

	if (!r)
		return (pid_t)0;

	xcb_res_client_id_value_iterator_t i = xcb_res_query_client_ids_ids_iterator(r);
	for (; i.rem; xcb_res_client_id_value_next(&i)) {
		spec = i.data->spec;
		if (spec.mask & XCB_RES_CLIENT_ID_MASK_LOCAL_CLIENT_PID) {
			uint32_t *t = xcb_res_client_id_value_value(i.data);
			result = *t;
			break;
		}
	}

	free(r);

	if (result == (pid_t)-1)
		result = 0;

#endif /* __linux__ */

#if defined(__OpenBSD__) || defined(__FreeBSD__)
        Atom type;
        int format;
        unsigned long len, bytes;
        unsigned char *prop;
        pid_t ret;

        if (XGetWindowProperty(dpy, w, XInternAtom(dpy, "_NET_WM_PID", 0), 0, 1, False, AnyPropertyType, &type, &format, &len, &bytes, &prop) != Success || !prop)
               return 0;

        ret = *(pid_t*)prop;
        XFree(prop);
        result = ret;

#endif /* __OpenBSD__ */
	return result;
}

pid_t
getparentprocess(pid_t p)
{
	unsigned int v = 0;

#ifdef __linux__
	FILE *f;
	char buf[256];
	snprintf(buf, sizeof(buf) - 1, "/proc/%u/stat", (unsigned)p);

	if (!(f = fopen(buf, "r")))
		return 0;

	fscanf(f, "%*u %*s %*c %u", &v);
	fclose(f);
#endif /* __linux__*/

#ifdef __FreeBSD__
    FILE *f;
	char buf[256];
	snprintf(buf, sizeof(buf) - 1, "/proc/%u/status", (unsigned)p);

    if (!(f = fopen(buf, "r")))
		return 0;

	fscanf(f, "%*s %*u %u", &v);
	fclose(f);
#endif

#ifdef __OpenBSD__
	int n;
	kvm_t *kd;
	struct kinfo_proc *kp;

	kd = kvm_openfiles(NULL, NULL, NULL, KVM_NO_FILES, NULL);
	if (!kd)
		return 0;

	kp = kvm_getprocs(kd, KERN_PROC_PID, p, sizeof(*kp), &n);
	v = kp->p_ppid;
#endif /* __OpenBSD__ */

	return (pid_t)v;
}

int
isdescprocess(pid_t p, pid_t c)
{
	while (p != c && c != 0)
		c = getparentprocess(c);

	return (int)c;
}

Client *
termforwin(const Client *w)
{
	Client *c;
	Monitor *m;

	if (!w->pid || w->isterminal)
		return NULL;

	for (m = mons; m; m = m->next) {
		for (c = m->clients; c; c = c->next) {
			if (c->isterminal && !c->swallowing && c->pid && isdescprocess(c->pid, w->pid))
				return c;
		}
	}

	return NULL;
}

Client *
swallowingclient(Window w)
{
	Client *c;
	Monitor *m;

	for (m = mons; m; m = m->next) {
		for (c = m->clients; c; c = c->next) {
			if (c->swallowing && c->swallowing->win == w)
				return c;
		}
	}

	return NULL;
}

Client *
wintoclient(Window w)
{
	Client *c;
	Monitor *m;

	for (m = mons; m; m = m->next)
		for (c = m->clients; c; c = c->next)
			if (c->win == w)
				return c;
	return NULL;
}

Monitor *
wintomon(Window w)
{ /* move window w to monitor m */
	int x, y;
	Client *c;
	Monitor *m;

	if (w == root && getrootptr(&x, &y))
		return recttomon(x, y, 1, 1);
	for (m = mons; m; m = m->next)
		if (w == m->barwin)
			return m;
	if ((c = wintoclient(w)))
		return c->mon;
	return selmon;
}

/* There's no way to check accesses to destroyed windows, thus those cases are
 * ignored (especially on UnmapNotify's). Other types of errors call Xlibs
 * default error handler, which may call exit. */
int
xerror(Display *dpy, XErrorEvent *ee)
{
	if (ee->error_code == BadWindow
	|| (ee->request_code == X_SetInputFocus && ee->error_code == BadMatch)
	|| (ee->request_code == X_PolyText8 && ee->error_code == BadDrawable)
	|| (ee->request_code == X_PolyFillRectangle && ee->error_code == BadDrawable)
	|| (ee->request_code == X_PolySegment && ee->error_code == BadDrawable)
	|| (ee->request_code == X_ConfigureWindow && ee->error_code == BadMatch)
	|| (ee->request_code == X_GrabButton && ee->error_code == BadAccess)
	|| (ee->request_code == X_GrabKey && ee->error_code == BadAccess)
	|| (ee->request_code == X_CopyArea && ee->error_code == BadDrawable))
		return 0;
	fprintf(stderr, "dwm: fatal error: request code=%d, error code=%d\n",
		ee->request_code, ee->error_code);
	return xerrorxlib(dpy, ee); /* may call exit */
}

int
xerrordummy(Display *dpy, XErrorEvent *ee)
{
	return 0;
}

/* Startup Error handler to check if another window manager
 * is already running. */
int
xerrorstart(Display *dpy, XErrorEvent *ee)
{
	die("dwm: another window manager is already running");
	return -1;
}

void
xrdb(const Arg *arg)
{ /* load xresource database colors */
  loadxrdb();
  int i;
  for (i = 0; i < LENGTH(colors); i++)
                scheme[i] = drw_scm_create(drw, colors[i], 3);
  focus(NULL);
  arrange(NULL);
}

void
zoom(const Arg *arg)
{
	Client *c = selmon->sel;

	if (!selmon->lt[selmon->sellt]->arrange || !c || c->isfloating)
		return;
	if (c == nexttiled(selmon->clients) && !(c = nexttiled(c->next)))
		return;
	pop(c);
}



int
main(int argc, char *argv[])
{
	if (argc == 2 && !strcmp("-v", argv[1]))
		die("dwm-"VERSION);
	else if (argc != 1)
		die("usage: dwm [-v]");
	if (!setlocale(LC_CTYPE, "") || !XSupportsLocale())
		fputs("warning: no locale support\n", stderr);
	if (!(dpy = XOpenDisplay(NULL)))
		die("dwm: cannot open display");
	if (!(xcon = XGetXCBConnection(dpy)))
		die("dwm: cannot get xcb connection\n");
	checkotherwm(); /* see if any other WMs are running */
        XrmInitialize();
        loadxrdb(); /* added by xrdb patch */
	setup(); /* initialize everything needed to start dwm */
#ifdef __OpenBSD__
	if (pledge("stdio rpath proc exec ps", NULL) == -1)
		die("pledge");
#endif /* __OpenBSD__ */
	scan(); /* see if other applications are already running */
	run(); /* main event loop of dwm -->
* continuously listens to events from the X server (window changes, key presses, mouse) 
* and sends them to the correct event handler */
	if(restart) execvp(argv[0], argv);
	cleanup();
	XCloseDisplay(dpy);
	if (restart == 1) {
		execlp("dwm", "dwm", NULL);
	}
	return EXIT_SUCCESS;
}
