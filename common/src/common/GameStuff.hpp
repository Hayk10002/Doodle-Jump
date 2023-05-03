#pragma once
#include <string>

enum class UserActions
{
	None,
	Close,
	Resize,
	Left,
	Right,
	Shoot,
	Die,
	Ressurect,
	BreakPoint
};

inline std::string doodle_jump_clipboard{};