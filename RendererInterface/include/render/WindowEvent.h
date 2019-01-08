#pragma once

enum class KeyCode : s32 {
	eUnknown = -1,
	eMouseLeft = 0, eMouseRight, eMouseMiddle, eMouse4, eMouse5, eMouse6, eMouse7, eMouse8,
	eSpace, eApostrophe, eComma, eSemicolon, eGraveAccent,
	eMinus, ePlus,
	eEqual, ePeriod, eSlash, eBackslash,
	eLBracket, eRBracket,
	
	e0, e1, e2, e3, e4, e5, e6, e7, e8, e9,
	eA, eB, eC, eD, eE, eF, eG, eH, eI, eJ, 
	eK, eL, eM, eN, eO, eP, eQ, eR, eS, eT, 
	eU, eV, eW, eX, eY, eZ, 
	
	eF1, eF2, eF3, eF4, eF5, eF6, eF7, eF8, eF9, eF10,
	eF11, eF12, eF13, eF14, eF15, eF16, eF17, eF18, eF19, eF20,
	eF21, eF22, eF23, eF24, eF25,
	
	eKP0, eKP1, eKP2, eKP3, eKP4, eKP5, eKP6, eKP7, eKP8, eKP9,
	eKPDecimal, eKPDivide, eKPMultiply, eKPSubtract, eKPAdd, eKPEnter, eKPEqual, eKP,
	
	eLShift, eRShift, eLCntrl, eRCntrl, eLAlt, eRAlt, eLSuper, eRSuper,
	eMenu, eWorld1, eWorld2,
	eEscape, eEnter, eTab, eBackspace, 
	eInsert, eDelete, ePageUp, ePageDown,
	eHome, eEnd, ePrintScreen, ePause,
	eCapsLock, eScrollLock,  eNumLock, 
	eRight, eLeft, eDown, eUp,
	
	eMax
};
enum class PressAction : u32 {
	ePress,
	eRelease,
	eRepeat
};
enum class MouseMoveAction : u32 {
	eMoved,
	eEntered,
	eLeft
};
enum class WindowAction : u32 {
	eMoved,
	eResized,
	eIconify,
	eMaximize,
	eFocused,
	eClosed,
};
struct ButtonEvent {//20 bytes
	PressAction action;
	KeyCode keycode;
	u32 scancode;
	char utf8[4];
	bool shift, cntrl, alt, super;
};

struct MouseEvent {//20 bytes
	MouseMoveAction action;
	float posx, posy;
	float deltax, deltay;
};
struct ScrollEvent {//4 bytes
	float deltax;
	float deltay;
};
struct CharEvent {//4 bytes
	char utf8[4];
	bool shift, cntrl, alt, super;
};
struct WindowEvent {//4 bytes
	WindowAction action;
	u32 x, y;
	bool value;
};
enum class OSEventType : u32 {
	eButton, eMouse, eScroll, eChar, eWindow
};
struct OSEvent {
	OSEventType type;
	union{
		WindowEvent window;
		CharEvent charinput;
		ScrollEvent scroll;
		MouseEvent mouse;
		ButtonEvent button;
	};
};
struct KeyState {
	bool pressed = false;
	u64 time_pressed = 0;
};
struct InputState {
	KeyState basic_keystates[(u32)KeyCode::eMax];
	Map<u32, KeyState> utf8_keystates;
	Map<u32, KeyState> scancode_keystates;
};