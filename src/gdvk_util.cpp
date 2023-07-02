#include "gdvk_util.h"
#include "xcompgrab.h" // gDisplay
#include <unordered_map>
#include <string>
#include <unistd.h>
//#include <X11/Xlib.h>
//#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/XF86keysym.h>
#include <X11/extensions/XTest.h>

static std::unordered_map<std::string, unsigned int> keymap;

KEYCODE stringToKeycode(const char* key_name) {

	return XKeysymToKeycode(gDisplay, XStringToKeysym(key_name));
}

KEYCODE keysymToKeycode(unsigned long keysym) {
	return XKeysymToKeycode(gDisplay, keysym);
}

void strprint(const std::string &str)
{
	return gprint(str.c_str());
}

void setKeyState(KEYCODE keyCode, bool pressState) {

#ifdef _DEBUG
	if (pressState)
		strprint("Sending keyboard DOWN event for key '" + std::to_string(keyCode) + "'");
	else
		strprint("Sending keyboard UP event for key '" + std::to_string(keyCode) + "'");
#endif

	if (!XTestFakeKeyEvent(gDisplay, keyCode, pressState, 0)) {
		strprint("Error sending keyboard event for key '" + std::to_string((int)keyCode) + "'");
	}
	XFlush(gDisplay);
}

KEYCODE lookupKeycode(const char *key_name) {

	if (keymap.find(key_name) != keymap.end()) {
		return keymap[key_name];
	}
#ifdef _DEBUG
	print("Warning: keyName: '" + keyName + "' not in keymap, guessing (platform dependent)");
#endif
	return stringToKeycode(key_name);
}





void generateKeymap() {
#ifdef _DEBUG
	Godot::print("Generating key map");
#endif
	// contains keycodes for special keys that have different names on windows and linux.
	// uses godot naming

/*
	Same name in X11 (omitted here):
	A - Z
	1 - 9
	F1 - F12
	KP_1 - KP_9
*/
	keymap["ESCAPE"]        = keysymToKeycode(XK_Escape);
	keymap["TAB"]           = keysymToKeycode(XK_Tab);
	keymap["BACKSPACE"]     = keysymToKeycode(XK_BackSpace);
	keymap["ENTER"]         = keysymToKeycode(XK_Return);
	keymap["PRINT"]         = keysymToKeycode(XK_Print);

	keymap["INSERT"]        = keysymToKeycode(XK_Insert);
	keymap["DELETE"]        = keysymToKeycode(XK_Delete);
	keymap["HOME"]          = keysymToKeycode(XK_Home);
	keymap["END"]           = keysymToKeycode(XK_End);
	keymap["PAGE_UP"]       = keysymToKeycode(XK_Page_Up);
	keymap["PAGE_DOWN"]     = keysymToKeycode(XK_Page_Down);

	keymap["LEFT"]          = keysymToKeycode(XK_Left);
	keymap["UP"]            = keysymToKeycode(XK_Up);
	keymap["RIGHT"]         = keysymToKeycode(XK_Right);
	keymap["DOWN"]          = keysymToKeycode(XK_Down);
	
	keymap["SHIFT"]         = keysymToKeycode(XK_Shift_L);
	keymap["SHIFT_L"]       = keysymToKeycode(XK_Shift_L);
	keymap["SHIFT_R"]       = keysymToKeycode(XK_Shift_R);
	keymap["CONTROL"]       = keysymToKeycode(XK_Control_L);
	keymap["CONTROL_L"]     = keysymToKeycode(XK_Control_L);
	keymap["CONTROL_R"]     = keysymToKeycode(XK_Control_R);
	keymap["ALT"]           = keysymToKeycode(XK_Alt_L);
	keymap["ALT_L"]         = keysymToKeycode(XK_Alt_L);
	keymap["ALT_R"]         = keysymToKeycode(XK_Alt_R);
	keymap["ALT_GR"]        = keysymToKeycode(XK_ISO_Level3_Shift);// Alt Gr
	keymap["SUPER"]         = keysymToKeycode(XK_Super_L);
	keymap["SUPER_L"]       = keysymToKeycode(XK_Super_L);
	keymap["SUPER_R"]       = keysymToKeycode(XK_Super_R);
	keymap["MENU"]          = keysymToKeycode(XK_Menu);

	keymap["CAPSLOCK"]      = keysymToKeycode(XK_Caps_Lock);
	keymap["NUMLOCK"]       = keysymToKeycode(XK_Num_Lock);
	keymap["SCROLLLOCK"]    = keysymToKeycode(XK_Scroll_Lock);
	
	keymap["KP_MULTIPLY"]   = keysymToKeycode(XK_KP_Multiply);
	keymap["KP_DIVIDE"]     = keysymToKeycode(XK_KP_Divide);
	keymap["KP_SUBTRACT"]   = keysymToKeycode(XK_KP_Subtract);
	keymap["KP_ADD"]        = keysymToKeycode(XK_KP_Add);
	keymap["KP_ENTER"]      = keysymToKeycode(XK_KP_Enter);
	keymap["KP_PERIOD"]     = keysymToKeycode(XK_KP_Separator); // period or comma on keypad

	keymap["BACK"]          = keysymToKeycode(XF86XK_Back);     // browser back
	keymap["FORWARD"]       = keysymToKeycode(XF86XK_Forward);  // browser forward
	keymap["VOLUME_DOWN"]   = keysymToKeycode(XF86XK_AudioLowerVolume);
	keymap["VOLUME_UP"]     = keysymToKeycode(XF86XK_AudioRaiseVolume);
	keymap["MEDIA_PLAY"]    = keysymToKeycode(XF86XK_AudioPlay);
	keymap["MEDIA_STOP"]    = keysymToKeycode(XF86XK_AudioStop);
	keymap["MEDIA_PREVIOUS"]= keysymToKeycode(XF86XK_AudioPrev);
	keymap["MEDIA_NEXT"]    = keysymToKeycode(XF86XK_AudioNext);

	keymap["SPACE"]         = keysymToKeycode(XK_space);
	keymap["EXCLAM"]        = keysymToKeycode(XK_exclam);       // !
	keymap["QUOTEDBL"]      = keysymToKeycode(XK_quotedbl);     // "
	keymap["NUMBERSIGN"]    = keysymToKeycode(XK_numbersign);   // #
	keymap["DOLLAR"]        = keysymToKeycode(XK_dollar);       // $
	keymap["PERCENT"]       = keysymToKeycode(XK_percent);      // %
	keymap["AMPERSAND"]     = keysymToKeycode(XK_ampersand);    // &
	keymap["APOSTROPHE"]    = keysymToKeycode(XK_apostrophe);   // '
	keymap["PARENLEFT"]     = keysymToKeycode(XK_parenleft);    // (
	keymap["PARENRIGHT"]    = keysymToKeycode(XK_parenright);   // )
	keymap["ASTERISK"]      = keysymToKeycode(XK_asterisk);     // *
	keymap["PLUS"]          = keysymToKeycode(XK_plus);         // +
	keymap["COMMA"]         = keysymToKeycode(XK_comma);        // ,
	keymap["MINUS"]         = keysymToKeycode(XK_minus);        // -
	keymap["PERIOD"]        = keysymToKeycode(XK_period);       // .
	keymap["SLASH"]         = keysymToKeycode(XK_slash);        // /
	keymap["BACKSLASH"]     = keysymToKeycode(XK_backslash);    // \    -

	keymap["COLON"]         = keysymToKeycode(XK_colon);        // :
	keymap["SEMICOLON"]     = keysymToKeycode(XK_semicolon);    // ;
	keymap["LESS"]          = keysymToKeycode(XK_less);         // <
	keymap["EQUAL"]         = keysymToKeycode(XK_equal);        // =
	keymap["GREATER"]       = keysymToKeycode(XK_greater);      // >
	keymap["QUESTION"]      = keysymToKeycode(XK_question);     // ?
	keymap["AT"]            = keysymToKeycode(XK_at);           // @
	keymap["SECTION"]       = keysymToKeycode(XK_section);      // § character

	keymap["BRACKET_LEFT"]  = keysymToKeycode(XK_bracketleft);  // [
	keymap["BRACKET_RIGHT"] = keysymToKeycode(XK_bracketright); // ]
	keymap["BRACE_LEFT"]    = keysymToKeycode(XK_braceleft);    // {
	keymap["BRACE_RIGHT"]   = keysymToKeycode(XK_braceright);   // }

	keymap["ASCIICIRCUM"]   = keysymToKeycode(XK_asciicircum);  // ^ character
	keymap["ASCIITILDE"]    = keysymToKeycode(XK_asciitilde);   // ~ character

	// these appear to cause crashes
	//keymap["QUOTELEFT"]     = keysymToKeycode(XK_quoteleft);    // ` character
	//keymap["ASCIIGRAVE"]    = keysymToKeycode(XK_acute);        // ` character

	// dead keys for accents
	keymap["ACUTE"]         = keysymToKeycode(XK_dead_acute);   // ´
	keymap["CEDILLA"]       = keysymToKeycode(XK_dead_cedilla); // ¸
	keymap["CIRCUM"]        = keysymToKeycode(XK_dead_circumflex); // ^
	keymap["DIAERSIS"]      = keysymToKeycode(XK_dead_diaeresis);  // ¨
	keymap["TILDE"]         = keysymToKeycode(XK_dead_tilde);   // ~
	keymap["GRAVE"]         = keysymToKeycode(XK_dead_grave);   // `
	
	// localisation
	keymap["ARING"]         = keysymToKeycode(XK_Aring);        // Å
	keymap["ADIAERSIS"]     = keysymToKeycode(XK_Adiaeresis);   // Ä
	keymap["ODIAERSIS"]     = keysymToKeycode(XK_Odiaeresis);   // Ö

	keymap["AE"]            = keysymToKeycode(XK_AE);           // Æ
	keymap["OOBLIQUE"]      = keysymToKeycode(XK_Ooblique);     // Ø

	keymap["UDIAERSIS"]     = keysymToKeycode(XK_Udiaeresis);   // Ü
	keymap["SSHARP"]        = keysymToKeycode(XK_ssharp);       // ß
}
