#pragma once

enum class KeyCode : s32 {
	Unknown = -1,
	MouseLeft = 0, MouseRight, MouseMiddle, Mouse4, Mouse5, Mouse6, Mouse7, Mouse8,
	Space, Apostrophe, Comma, Semicolon, GraveAccent,
	Minus, Plus,
	Equal, Period, Slash, Backslash,
	LBracket, RBracket,
	
	Digit0, Digit1, Digit2, Digit3, Digit4, Digit5, Digit6, Digit7, Digit8, Digit9,
	A, B, C, D, E, F, G, H, I, J, 
	K, L, M, N, O, P, Q, R, S, T, 
	U, V, W, X, Y, Z, 
	
	F1, F2, F3, F4, F5, F6, F7, F8, F9, F10,
	F11, F12, F13, F14, F15, F16, F17, F18, F19, F20,
	F21, F22, F23, F24, F25,
	
	KP0, KP1, KP2, KP3, KP4, KP5, KP6, KP7, KP8, KP9,
	KPDecimal, KPDivide, KPMultiply, KPSubtract, KPAdd, KPEnter, KPEqual, KP,
	
	LShift, RShift, LCntrl, RCntrl, LAlt, RAlt, LSuper, RSuper,
	Menu, World1, World2,
	Escape, Enter, Tab, Backspace, 
	Insert, Delete, PageUp, PageDown,
	Home, End, PrintScreen, Pause,
	CapsLock, ScrollLock,  NumLock, 
	Right, Left, Down, Up,
	
	Max
};
enum class PressAction : u32 {
	Press,
	Release,
	Repeat
};
enum class MouseMoveAction : u32 {
	Moved,
	Entered,
	Left
};
enum class WindowAction : u32 {
	Moved,
	Resized,
	FrameResized,
	Iconify,
	Maximize,
	Focused,
	Closed,
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
	Button, Mouse, Scroll, Char, Window
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
	double time_pressed = 0.0;
};
struct InputState {
	KeyState basic_keystates[(u32) KeyCode::Max];
	Map<u32, KeyState> utf8_keystates;
	Map<u32, KeyState> scancode_keystates;
};