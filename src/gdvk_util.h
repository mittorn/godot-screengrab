typedef unsigned long KEYCODE;

KEYCODE lookupKeycode(const char *keycode);
void generateKeymap();
void setKeyState(KEYCODE, bool pressed);