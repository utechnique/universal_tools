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

enum Button
{
	button_escape,        button_1,            button_2,         button_3,
	button_4,             button_5,            button_6,         button_7,
	button_8,             button_9,            button_0,         button_minus,
	button_equals,        button_back,         button_tab,       button_q,
	button_w,             button_e,            button_r,         button_t,
	button_y,             button_u,            button_i,         button_o,
	button_p,             button_lbracket,     button_rbracket,  button_return,
	button_lcontrol,      button_a,            button_s,         button_d,
	button_f,             button_g,            button_h,         button_j,
	button_k,             button_l,            button_semicolon, button_apostrophe,
	button_grave,         button_lshift,       button_backslash, button_z,
	button_x,             button_c,            button_v,         button_b,
	button_n,             button_m,            button_comma,     button_period,
	button_slash,         button_rshift,       button_multiply,  button_lmenu,
	button_space,         button_capital,      button_f1,        button_f2,
	button_f3,            button_f4,           button_f5,        button_f6,
	button_f7,            button_f8,           button_f9,        button_f10,
	button_numlock,       button_scroll,       button_numpad7,   button_numpad8,
	button_numpad9,       button_subtract,     button_numpad4,   button_numpad5,
	button_numpad6,       button_add,          button_numpad1,   button_numpad2,
	button_numpad3,       button_numpad0,      button_decimal,   button_F11,
	button_F12,           button_F13,          button_F14,       button_F15,
	button_kana,          button_convert,      button_noconvert, button_yen,
	button_numpad_equals, button_circumflex,   button_at,        button_colon,
	button_underline,     button_kanji,        button_stop,      button_ax,
	button_unlabeled,     button_numpad_enter, button_rcontrol,  button_numpad_comma,
	button_divide,        button_sysrq,        button_rmenu,     button_home,
	button_up,            button_prior,        button_left,      button_right,
	button_end,           button_down,         button_next,      button_insert,
	button_delete,        button_lwin,         button_rwin,      button_apps,
	button_pause,
	button_count
};

static const char* skKeyNames[button_count] =
{
	"Esc",      "1",          "2",         "3",
	"4",        "5",          "6",         "7",
	"8",        "9",          "0",         "-",
	"=",        "Back",       "Tab",       "Q",
	"W",        "E",          "R",         "T",
	"Y",        "U",          "I",         "O",
	"P",        "[",          "]",         "Enter",
	"LCtrl",    "A",          "S",         "D",
	"F",        "G",          "H",         "J",
	"K",        "L",          ";",         "\'",
	"`",        "LShift",     "\\",        "Z",
	"X",        "C",          "V",         "B",
	"N",        "M",          ",",         ".",
	"/",        "RShift",     "*",         "LMenu",
	"Space",    "CapsLock",   "F1",        "F2",
	"F3",       "F4",         "F5",        "F6",
	"F7",       "F8",         "F9",        "F10",
	"NumLock",  "ScrollLock", "Num7",      "Num8",
	"Num9",     "Num-",       "Num4",      "Num5",
	"Num6",     "Num+",       "Num1",      "Num2",
	"Num3",     "Num0",       "Num.",      "F11",
	"F12",      "F13",        "F14",       "F15",
	"Kana",     "Convert",    "NoConvert", "Yen",
	"Num=",     "Circumflex", "AT",        ":",
	"_",        "Kanji",      "Stop",      "AX",
	"Unlabeled", "NumEnter",  "RCtrl",     "Num,",
	"Num/",      "SysRq",     "RMenu",     "Home",
	"Up",        "Prior",     "Left",      "Right",
	"End",       "Down",      "Next",      "Insert",
	"Delete",    "LWin",      "RWin",      "Apps",
	"Pause"
};

//----------------------------------------------------------------------------//
END_NAMESPACE(keyboard)
END_NAMESPACE(input)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//