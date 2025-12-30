/*
patches applied (incl. some slight changes):

dwm-bartoggle-keybinds-6.4.diff                 ----- toggle pieces of the bar.
dwm-bulkill-20231029-9f88553.diff               ----- kill all except current; kill all in tag.
dwm-colorbar-6.3.diff                           ----- fine control over statusbar colors.
dwm-fixmultimon-6.4.diff                        ----- pretty much a bug fix.
dwm-focusfullscreen-20211121-95e7342.diff       ----- proper fullscreen.
dwm-focusmonmouse-6.2.diff                      ----- move mouse when you switch monitors via keybind.
dwm-hide_vacant_tags-6.4.diff                   ----- hide tags with no windows.
dwm-preventfocusshift-20240831-6.5.diff         ----- automatically exit fullscreen when a window is spawned.
dwm-restartsig-20180523-6.2.diff                ----- refresh dwm after recompile, without quitting.
dwm-spawntag-6.2.diff                           ----- spawn an application when tag is middle-clicked.
dwm-stacker-6.2.diff                            ----- more utilities to manage the stack.
dwm-statuscmd-20210405-67d76bd.diff             ----- execute cmds when statusbar is clicked (used for dwmblocks).
dwm-sticky-6.5.diff                             ----- make a window follow from tag to tag.
dwm-swallow-6.3.diff                            ----- terminal swallows launched programs.
dwm-vanitygaps-6.2.diff                         ----- gaps are functional: your eye is less inclined to drift while working.
dwm-xrdb-6.4.diff                               ----- xresource database colors.
dwm-quitprompt-20220718-6613d9f.diff			----- prompts for confirmation on quit.
dwm-ifroot-6.6.diff								----- map keys to have one behavior when a window is in focus and another on bg.
dwm-alwayscenter-20200625-f04cac6.diff     		----- center floating windows.
dwm-centeredwindowname-20200723-f035e1e.diff	----- center window name.
dwm-savefloats-20181212-b69c870.diff			----- saves and restores from saved floating windows possitions.
*/

/* appearance */
#include <X11/X.h>
static unsigned int borderpx        = 1;        /* border pixel of windows */
static unsigned int snap            = 32;       /* snap pixel */
static const unsigned int gappih    = 10;       /* horiz inner gap between windows */
static const unsigned int gappiv    = 10;       /* vert inner gap between windows */
static const unsigned int gappoh    = 10;       /* horiz outer gap between windows and screen edge */
static const unsigned int gappov    = 15;       /* vert outer gap between windows and screen edge */
static int smartgaps                = 1;        /* 1 means no outer gap when there is only one window */
static const int swallowfloating    = 1;        /* 1 means swallow floating windows by default */
static int showbar                  = 1;        /* 0 means no bar */
static const int showtitle          = 1;        /* 0 means no title */
static const int showtags           = 1;        /* 0 means no tags */
static const int showlayout         = 1;        /* 0 means no layout indicator */
static const int showstatus         = 1;        /* 0 means no status bar */
static const int showfloating       = 0;        /* 0 means no floating indicator */
static int topbar                   = 1;        /* 0 means bottom bar */

static char dmenufont[]             = "JetBrainsMono Nerd Font:bold:size=14";
static const char *fonts[]          = { "JetBrainsMono Nerd Font:bold:size=14:antialias=true:autohint=true" };

/* default colors used if xrdb is not loaded */
static char normbgcolor[]           = "#2e3440";
static char normbordercolor[]       = "#4c566a";
static char normfgcolor[]           = "#d8dee9";
static char selfgcolor[]            = "#eceff4";
static char selbordercolor[]        = "#a3be8c";
static char selbgcolor[]            = "#b48ead";

static char *colors[][3] = {
       /*               fg           bg           border   */
		[SchemeNorm]      = { normfgcolor, normbgcolor, normbordercolor },
		[SchemeSel]       = { selbgcolor, selfgcolor,  selbordercolor  },
		/* for bar --> {text, background, null} */
		[SchemeStatus]    = { normfgcolor, normbgcolor,  normbgcolor  }, /* status R */
		[SchemeTagsSel]   = { normfgcolor, normbgcolor,  normbgcolor  }, /* tag L selected */
		[SchemeTagsNorm]  = { selbordercolor, normbgcolor,  normbgcolor  }, /* tag L unselected */
		[SchemeInfoSel]   = { normfgcolor, normbgcolor,  normbgcolor  }, /* info M selected */
		[SchemeInfoNorm]  = { normfgcolor, normbgcolor,  normbgcolor  }, /* info M unselected */
};

/* Audio */
static const char *mute_volume[] = { "amixer", "-D pulse", "set", "Master", "toggle", NULL };
static const char *lower_volume[] = { "amixer", "-D pulse", "set", "Master", "5%-", NULL };
static const char *raise_volume[] = { "amixer", "-D pulse", "set", "Master", "5%+", NULL };

/* Display brightness */
static const char *lower_brightness[] = { "brightnessctl", "set", "5%-", NULL };
static const char *raise_brightness[] = { "brightnessctl", "set", "5%+", NULL };

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };
// static const char *tags[] = { "󰎤", "󰎧", "󰎪", "󰎭", "󰎱", "󰎳", "󰎶", "󰎹", "󰎼" };


static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class     instance  title           tags mask  isfloating  isterminal  noswallow  monitor */
	{ "St",      NULL,     NULL,           0,         0,          1,           0,        -1 },
	{ "fzfmenu", NULL,     "fzf", 	       0,         1,          1,           1,        -1 }, /* xev */
	{ NULL,      NULL,     "Event Tester", 0,         0,          0,           1,        -1 }, /* xev */
};

#include "vanitygaps.c"


/* layout(s) */
static const float mfact        = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster        = 1;    /* number of clients in master area */
static const int resizehints    = 1;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 0;    /* 1 will force focus on the fullscreen window */


static const Layout layouts[] = { /* alt glyphs: 󱡗 󱏋 */
	/* symbol     arrange function */
	{ "󰓒",      	tile },    /* first entry is default */
	{ "󰇥",			NULL },    /* no layout function means floating behavior */
	{ "",		   monocle },
	{ "󰫣",      	spiral },
	{ "󰫥",      	dwindle },
};


/* key definitions */
#define MODKEY Mod4Mask // windows key
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

#define STACKKEYS(MOD,ACTION) \
	{ MOD, XK_j,     ACTION##stack, {.i = INC(+1) } }, \
	{ MOD, XK_k,     ACTION##stack, {.i = INC(-1) } }, \
	{ MOD, XK_grave, ACTION##stack, {.i = PREVSEL } },\
	{ MOD, XK_w,     ACTION##stack, {.i = 0 } }, \
	{ MOD, XK_a,     ACTION##stack, {.i = 1 } }, \
	{ MOD, XK_z,     ACTION##stack, {.i = 2 } }, \
	{ MOD, XK_x,     ACTION##stack, {.i = -1 } },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* helper for launching gtk application */
#define GTKCMD(cmd) { .v = (const char*[]){ "/usr/bin/gtk-launch", cmd, NULL } }

#define STATUSBAR "dwmblocks"
#define BROWSER "firefox-developer-edition"

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", normbgcolor, "-nf", normfgcolor, "-sb", selbordercolor, "-sf", selfgcolor, NULL };
static const char *termcmd[]  = { "st", NULL };


static const Arg tagexec[] = { /* spawn application when tag is middle-clicked */
	{ .v = termcmd }, /* 1 */
	{ .v = termcmd }, /* 2 */
	{ .v = termcmd }, /* 3 */
	{ .v = termcmd }, /* 4 */
	{ .v = termcmd }, /* 5 */
	{ .v = termcmd }, /* 6 */
	{ .v = termcmd }, /* 7 */
	{ .v = termcmd }, /* 8 */
	{ .v = termcmd }, /* 9 */
    /* GTKCMD("gtkapplication") */
};

static const Key keys[] = {
	/* modifier                     	key        				function        			argument */
	{ MODKEY,                       	XK_d,      				spawn,          			{.v = dmenucmd } },
	{ MODKEY,                       	XK_Return, 				spawn,          			{.v = termcmd } },
	{ MODKEY|ShiftMask,             	XK_b,      				togglebar,      			{0} },
	STACKKEYS(MODKEY,                          					focus)
	STACKKEYS(MODKEY|ShiftMask,                					push)
	{ MODKEY|ShiftMask,             	XK_i,      				incnmaster,     			{.i = +1 } }, /* increase # of master windows */
	{ MODKEY|ControlMask,           	XK_i,      				incnmaster,     			{.i = -1 } }, /* decrease # of master windows */
	{ MODKEY,                       	XK_h,      				setmfact,       			{.f = -0.05} },
	{ MODKEY,                       	XK_l,      				setmfact,       			{.f = +0.05} },


	{ MODKEY,                       	XK_Tab,    				view,           			{0} },
	{ MODKEY,                       	XK_0,      				view,           			{.ui = ~0 } },
	{ MODKEY|ShiftMask,             	XK_0,      				tag,            			{.ui = ~0 } },

/* AUDIO */
	{ 0,								XF86XK_AudioMute,			spawn,						{.v = mute_volume} },
	{ 0,								XF86XK_AudioLowerVolume,	spawn,						{.v = lower_volume} },
	{ 0,								XF86XK_AudioRaiseVolume,	spawn,						{.v = raise_volume} },

/* DISPLAY */
	{ 0,								XF86XK_MonBrightnessDown,	spawn,						{.v = lower_brightness} },
	{ 0,								XF86XK_MonBrightnessUp,		spawn,						{.v = raise_brightness} },


/* refresh dwm (restartsig) */
	{ MODKEY|ShiftMask, 				XK_r,					quit,						{1} },
/* refresh xrdb; run this when setting new colors */
	{ MODKEY|ControlMask,				XK_r,   				xrdb,						{.v = NULL } },
/* quit dwm */
	{ MODKEY|ShiftMask,					XK_BackSpace,   		quitprompt,					{0} },
/* kill all windows besides current */
	{ MODKEY|ShiftMask,					XK_q,					killclient,             	{.ui = 1} },

/* WINDOW MANAGEMENT */

	/* focus fullscreen patch */
	{ MODKEY,					XK_f,	   				togglefullscreen,            {0} },
	/* toggle sticky window */
	{ MODKEY|ShiftMask,             	XK_s,      				togglesticky,                {0} },

/* LAYOUTS */

	/* tile */	
	{ MODKEY|ControlMask,               XK_t,      				setlayout,                   {.v = &layouts[0]} },
	/* floating */	
	{ MODKEY|ControlMask,				XK_f,	   				setlayout,            		 {.v = &layouts[1]} },
	{ MODKEY|ShiftMask,					XK_space,  				togglefloating,              {0} },
	/* monacle */
	{ MODKEY|ControlMask,				XK_m,      				setlayout,                   {.v = &layouts[2]} },
	/* spiral */
	{ MODKEY|ControlMask,				XK_s,      				setlayout,                   {.v = &layouts[3]} },
	/* dwindle */
	{ MODKEY|ControlMask,				XK_d,      				setlayout,                   {.v = &layouts[4]} },

	{ MODKEY|ControlMask,				XK_space,  				setlayout,                   {0} },
	{ MODKEY,							XK_space,  				zoom,                        {0} },
	{ MODKEY|ControlMask,           	XK_space,  				focusmaster,                 {0} },


/* ifroot mappings			first action is on bg second on a window */
	{ MODKEY,         					XK_q,      				ifroot,         			{.v = &(TwoFuncPtr){quitprompt, killclient, {0}, {0} } } },


/* multi-monitor control */
	{ MODKEY,							XK_bracketright,        focusmon,       			{.i = -1 } },
	{ MODKEY|ShiftMask,					XK_bracketright,        tagmon,         			{.i = -1 } },
	{ MODKEY,							XK_bracketleft,         focusmon,       			{.i = +1 } },
	{ MODKEY|ShiftMask,					XK_bracketleft,         tagmon,         			{.i = +1 } },

/* gaps control */
	{ MODKEY,	               			XK_minus,  				incrgaps,       			{.i = -1 } }, /* all */
	{ MODKEY,		      				XK_equal,  				incrgaps,       			{.i = +1 } },
	{ MODKEY|Mod1Mask,					XK_i,      				incrigaps,      			{.i = +1 } }, /* inner */
	{ MODKEY|Mod1Mask|ShiftMask,    	XK_i,       			incrigaps,      			{.i = -1 } },
	{ MODKEY|Mod1Mask,              	XK_o,      				incrogaps,      			{.i = +1 } }, /* outer */
	{ MODKEY|Mod1Mask|ShiftMask,    	XK_o,      				incrogaps,      			{.i = -1 } },
	{ MODKEY|Mod1Mask,              	XK_6,      				incrihgaps,     			{.i = +1 } }, /* inner horiz */
	{ MODKEY|Mod1Mask|ShiftMask,    	XK_6,      				incrihgaps,     			{.i = -1 } },
	{ MODKEY|Mod1Mask,              	XK_7,      				incrivgaps,     			{.i = +1 } }, /* inner vert */
	{ MODKEY|Mod1Mask|ShiftMask,    	XK_7,      				incrivgaps,     			{.i = -1 } },
	{ MODKEY|Mod1Mask,              	XK_8,      				incrohgaps,     			{.i = +1 } }, /* outer horiz */
	{ MODKEY|Mod1Mask|ShiftMask,   		XK_8,      				incrohgaps,     			{.i = -1 } },
	{ MODKEY|Mod1Mask,              	XK_9,      				incrovgaps,     			{.i = +1 } }, /* outer vert */
	{ MODKEY|Mod1Mask|ShiftMask,    	XK_9,      				incrovgaps,     			{.i = -1 } },
	{ MODKEY|ShiftMask,             	XK_equal,  				togglegaps,     			{0} },
	{ MODKEY|ShiftMask,             	XK_minus,  				defaultgaps,    			{0} },


/* tag keys */
	TAGKEYS(                        	XK_1,                      0)
	TAGKEYS(                        	XK_2,                      1)
	TAGKEYS(                        	XK_3,                      2)
	TAGKEYS(                        	XK_4,                      3)
	TAGKEYS(                        	XK_5,                      4)
	TAGKEYS(                        	XK_6,                      5)
	TAGKEYS(                        	XK_7,                      6)
	TAGKEYS(                        	XK_8,                      7)
	TAGKEYS(                        	XK_9,                      8)


/* toggle statusbar pieces individually */
	{ MODKEY|ControlMask,				XK_t,      		togglebartitle, {0} },
	{ MODKEY|ControlMask,				XK_s,      		togglebarstatus,{0} },
	{ MODKEY|ControlMask,				XK_t,      		togglebartags,  {0} },
    { MODKEY|ControlMask,				XK_e,      		togglebarcolor, {0} }, /* swaps fg/bg for tag+win */
	{ MODKEY|ControlMask,				XK_r,      		togglebarlt,    {0} },
	{ MODKEY|ControlMask,				XK_f,      		togglebarfloat, {0} },


/* application bindings */
	{ MODKEY|ControlMask|ShiftMask,					XK_w,          	spawn,      {.v = (const char*[]){ BROWSER, NULL } } },
	{ MODKEY,							XK_n,          	spawn,      {.v = (const char*[]){ "st", "-e", "nvim", NULL } } },
	{ MODKEY|ShiftMask,     			XK_h,          	spawn,      {.v = (const char*[]){ "st", "-e", "htop", NULL } } },

};


/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
	/* click                event mask           button          function        argument */
#ifndef __OpenBSD__
	{ ClkWinTitle,          0,				Button2,	zoom,           {0} },
	{ ClkStatusText,        0,				Button1,	sigstatusbar,   {.i = 1} },
	{ ClkStatusText,        0,				Button2,	sigstatusbar,   {.i = 2} },
	{ ClkStatusText,        0,				Button3,	sigstatusbar,   {.i = 3} },
	{ ClkStatusText,        0,				Button4,	sigstatusbar,   {.i = 4} },
	{ ClkStatusText,        0,				Button5,	sigstatusbar,   {.i = 5} },
	{ ClkStatusText,		ShiftMask,		Button1,	sigstatusbar,	{.i = 6} },
#endif

	{ ClkStatusText,        ShiftMask,		Button3,	spawn,          SHCMD("st -e nvim ~/src/suckless/dwmblocks-dnet/blocks.h") },
	{ ClkClientWin,			MODKEY,			Button1,	movemouse,      {0} }, /* left click */
	{ ClkClientWin,			MODKEY,			Button2,	defaultgaps,    {0} }, /* middle click */
	{ ClkClientWin,			MODKEY,			Button3,	resizemouse,    {0} }, /* right click */
	{ ClkClientWin,			MODKEY,			Button4,	incrgaps,       {.i = +1} }, /* scroll up */
	{ ClkClientWin,			MODKEY,			Button5,	incrgaps,       {.i = -1} }, /* scroll down */
	{ ClkTagBar,			0,				Button1,	view,           {0} },
	{ ClkTagBar,			0,				Button3,	toggleview,     {0} },
	{ ClkTagBar,			MODKEY,			Button1,	tag,            {0} },
	{ ClkTagBar,			MODKEY,			Button3,	toggletag,      {0} },
	{ ClkRootWin,			0,				Button2,	togglebar,      {0} }, /* hide bar */
};
