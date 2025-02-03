//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_input_device.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(input)
START_NAMESPACE(keyboard)
//----------------------------------------------------------------------------//

enum class Button
{
	escape,       one,          two,           three,
	four,         five,         six,           seven,
	eight,        nine,         zero,          minus,
	equals,       back,         tab,           q,
	w,            e,            r,             t,
	y,            u,            i,             o,
	p,            lbracket,     rbracket,      enter,
	lcontrol,     a,            s,             d,
	f,            g,            h,             j,
	k,            l,            semicolon,     apostrophe,
	grave,        lshift,       backslash,     z,
	x,            c,            v,             b,
	n,            m,            comma,         dot,
	slash,        rshift,       multiply,      lalt,
	space,        capslock,     f1,            f2,
	f3,           f4,           f5,            f6,
	f7,           f8,           f9,            f10,
	numlock,      scroll,       numpad7,       numpad8,
	numpad9,      numpad_minus, numpad4,       numpad5,
	numpad6,      numpad_plus,  numpad1,       numpad2,
	numpad3,      numpad0,      numpad_dot,    F11,
	F12,          F13,          F14,           F15,
	kana,         yen,          numpad_equals, stop,
	numpad_enter, rcontrol,     numpad_comma,  numpad_slash,
	sysrq,        ralt,         home,          up,
	previous,     left,         right,         end,
	down,         next,         insert,        del,
	lwin,         rwin,         pause,
	count
};

static const char* skKeyNames[static_cast<size_t>(Button::count)] =
{
	"Esc",      "1",          "2",      "3",
	"4",        "5",          "6",      "7",
	"8",        "9",          "0",      "-",
	"=",        "Back",       "Tab",    "Q",
	"W",        "E",          "R",      "T",
	"Y",        "U",          "I",      "O",
	"P",        "[",          "]",      "Enter",
	"LCtrl",    "A",          "S",      "D",
	"F",        "G",          "H",      "J",
	"K",        "L",          ";",      "\'",
	"`",        "LShift",     "\\",     "Z",
	"X",        "C",          "V",      "B",
	"N",        "M",          ",",      ".",
	"/",        "RShift",     "*",      "LAlt",
	"Space",    "CapsLock",   "F1",     "F2",
	"F3",       "F4",         "F5",     "F6",
	"F7",       "F8",         "F9",     "F10",
	"NumLock",  "ScrollLock", "Num7",   "Num8",
	"Num9",     "Num-",       "Num4",   "Num5",
	"Num6",     "Num+",       "Num1",   "Num2",
	"Num3",     "Num0",       "Num.",   "F11",
	"F12",      "F13",        "F14",    "F15",
	"Kana",     "Yen",        "Num=",   "Stop",
	"NumEnter", "RCtrl",      "Num,",   "Num/",
	"SysRq",    "RAlt",       "Home",   "Up",
	"Previous", "Left",       "Right",  "End",
	"Down",     "Next",       "Insert", "Delete",
	"LWin",     "RWin",       "Pause"
};

//----------------------------------------------------------------------------//
END_NAMESPACE(keyboard)
END_NAMESPACE(input)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//