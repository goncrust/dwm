/* See LICENSE file for copyright and license details. */
#include <X11/XF86keysym.h>

/* appearance */
static const unsigned int borderpx  = 2;        /* border pixel of windows */
static const unsigned int gappx     = 8;        /* gaps between windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const unsigned int systraypinning = 0;   /* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static const unsigned int systrayonleft  = 0;   /* 0: systray in the right corner, >0: systray on left of status text */
static const unsigned int systrayspacing = 4;   /* systray spacing */
static const int systraypinningfailfirst = 1;   /* 1: if pinning fails, display systray on the first monitor, False: display systray on the last monitor*/
static const int showsystray        = 1;        /* 0 means no systray */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
#define ICONSIZE 19                             /* icon size */
#define ICONSPACING 8                           /* space between icon and title */
static const int horizpadbar        = 0;        /* horizontal padding for statusbar */
static const int vertpadbar         = 2;        /* vertical padding for statusbar */
static const int vertpad            = 0;        /* vertical padding of bar */
static const int sidepad            = 0;        /* horizontal padding of bar */

/* fonts */
static const char *fonts[]               = {
    "Iosevka:style:medium:size=11",
    "Hack Nerd Font Mono:style:medium:size=19",
};
static const char dmenufont[]       = "Iosevka:style:medium:size=11";

#include "themes/onedark.h"

static const char *colors[][3]      = {
    /*                     fg       bg      border */
    [SchemeNorm]       = { white,   black,  gray2 },
    [SchemeSel]        = { gray4,   blue,   blue  },
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static const unsigned int ulinepad    = 5;    /* horizontal padding between the underline and tag */
static const unsigned int ulinestroke    = 2;    /* thickness / height of the underline */
static const unsigned int ulinevoffset    = 0;    /* how far above the bottom of the bar the line should appear */
static const int ulineall         = 0;    /* 1 to show underline on all tags, 0 for just the active ones */

static const char *tagsel[][2] = {
    { black, blue },
    { black, pink },
    { black, green },
    { black, yellow },
    { black, red },
    { black, orange },
    { black, white },
    { black, white },
    { black, white },
};

static const Rule rules[] = {
    /* xprop(1):
     *    WM_CLASS(STRING) = instance, class
     *    WM_NAME(STRING) = title
     *
     *    to get WM_CLASS: xprop | grep CLASS
     */
    /* class         instance    title       tags mask     isfloating   monitor */
    { "Cider",       NULL,       NULL,       0,            1,           -1 },
    { "SpeedCrunch", NULL,       NULL,       0,            1,           -1 },
    { "Pcmanfm",     NULL,       NULL,       0,            1,           -1 },
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */

static const Layout layouts[] = {
    /* symbol     arrange function */
    { "[]=",      tile },    /* first entry is default */
    { "><>",      NULL },    /* no layout function means floating behavior */
    { "[M]",      monocle },
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
    { MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
    { MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
    { MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
    { MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", black, "-nf", white, "-sb", blue, "-sf", black, NULL };
static const char *termcmd[]  = { "st", "zsh", NULL };
static const char *cidercmd[] = {"cider", NULL};
static const char *speedcrunchcmd[] = {"speedcrunch", NULL};
static const char *prtsccmd[] = {"flameshot", "gui", NULL};
static const char *pcmanfmcmd[] = {"pcmanfm", NULL};

/* volume commands */
static const char mutecmd[] =
    "amixer set Master toggle; if amixer get Master | grep -Fq \"[off]\"; then "
    "volnoti-show -m; else volnoti-show $(amixer get Master | grep -Po "
    "\"[0-9]+(?=%)\" | tail -1); fi";
static const char volupcmd[] =
    "amixer set Master 5%+ && volnoti-show $(amixer get Master | grep -Po "
    "\"[0-9]+(?=%)\" | tail -1)";
static const char voldowncmd[] =
    "amixer set Master 5%- && volnoti-show $(amixer get Master | grep -Po "
    "\"[0-9]+(?=%)\" | tail -1)";
static const char *miccmd[] = {"amixer", "set", "Capture", "toggle", NULL};

/* brightness commands */
static const char brupcmd[] = "light -A 5; volnoti-brightness";
static const char brdowncmd[] = "light -U 5; volnoti-brightness";

static const Key keys[] = {
    /* modifier                     key                       function        argument */
    { MODKEY,                       XK_p,                     spawn,          {.v = dmenucmd } },
    { MODKEY,                       XK_Return,                spawn,          {.v = termcmd } },
    { MODKEY,                       XK_b,                     togglebar,      {0} },
    { MODKEY,                       XK_j,                     focusstack,     {.i = +1 } },
    { MODKEY,                       XK_k,                     focusstack,     {.i = -1 } },
    { MODKEY,                       XK_i,                     incnmaster,     {.i = +1 } },
    { MODKEY,                       XK_d,                     incnmaster,     {.i = -1 } },
    { MODKEY,                       XK_h,                     setmfact,       {.f = -0.05} },
    { MODKEY,                       XK_l,                     setmfact,       {.f = +0.05} },
    { MODKEY|ShiftMask,             XK_Return,                zoom,           {0} },
    { MODKEY,                       XK_Tab,                   view,           {0} },
    { MODKEY|ShiftMask,             XK_c,                     killclient,     {0} },
    { MODKEY,                       XK_t,                     setlayout,      {.v = &layouts[0]} },
    { MODKEY,                       XK_f,                     setlayout,      {.v = &layouts[1]} },
    { MODKEY,                       XK_m,                     setlayout,      {.v = &layouts[2]} },
    { MODKEY,                       XK_space,                 setlayout,      {0} },
    { MODKEY|ShiftMask,             XK_space,                 togglefloating, {0} },
    { MODKEY,                       XK_0,                     view,           {.ui = ~0 } },
    { MODKEY|ShiftMask,             XK_0,                     tag,            {.ui = ~0 } },
    { MODKEY,                       XK_comma,                 focusmon,       {.i = -1 } },
    { MODKEY,                       XK_period,                focusmon,       {.i = +1 } },
    { MODKEY|ShiftMask,             XK_comma,                 tagmon,         {.i = -1 } },
    { MODKEY|ShiftMask,             XK_period,                tagmon,         {.i = +1 } },
    { MODKEY,                       XK_minus,                 setgaps,        {.i = -1 } },
    { MODKEY,                       XK_equal,                 setgaps,        {.i = +1 } },
    { MODKEY|ShiftMask,             XK_equal,                 setgaps,        {.i = 0  } },
    TAGKEYS(                        XK_1,                                     0)
    TAGKEYS(                        XK_2,                                     1)
    TAGKEYS(                        XK_3,                                     2)
    TAGKEYS(                        XK_4,                                     3)
    TAGKEYS(                        XK_5,                                     4)
    TAGKEYS(                        XK_6,                                     5)
    TAGKEYS(                        XK_7,                                     6)
    TAGKEYS(                        XK_8,                                     7)
    TAGKEYS(                        XK_9,                                     8)
    { MODKEY|ShiftMask,             XK_q,                     quit,           {0} },
    { 0,                            XF86XK_AudioMute,         spawn,          SHCMD(mutecmd) },
    { 0,                            XF86XK_AudioMicMute,      spawn,          {.v = miccmd} },
    { 0,                            XF86XK_AudioLowerVolume,  spawn,          SHCMD(voldowncmd) },
    { 0,                            XF86XK_AudioRaiseVolume,  spawn,          SHCMD(volupcmd) },
    { 0,                            XF86XK_MonBrightnessUp,   spawn,          SHCMD(brupcmd) },
    { 0,                            XF86XK_MonBrightnessDown, spawn,          SHCMD(brdowncmd) },
    { 0,                            XK_Print,                 spawn,          {.v = prtsccmd} },
    { MODKEY|ControlMask,           XK_m,                     spawn,          {.v = cidercmd} },
    { MODKEY|ControlMask,           XK_c,                     spawn,          {.v = speedcrunchcmd} },
    { MODKEY|ControlMask,           XK_f,                     spawn,          {.v = pcmanfmcmd} },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
    /* click                event mask      button          function        argument */
    { ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
    { ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
    { ClkWinTitle,          0,              Button2,        zoom,           {0} },
    { ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
    { ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
    { ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
    { ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
    { ClkTagBar,            0,              Button1,        view,           {0} },
    { ClkTagBar,            0,              Button3,        toggleview,     {0} },
    { ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
    { ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};
