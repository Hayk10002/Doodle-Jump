#pragma once
#include <deque>
#include <string>
#include <typeinfo>

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

struct DoodleJumpClipboard
{
	std::deque<std::pair<const std::type_info&, std::string>> recent_copies;
	std::deque<std::pair<const std::type_info&, std::string>> recent_deletions;
	void toImGui();
	bool empty() const;
};

inline DoodleJumpClipboard doodle_jump_clipboard{};

void saveFilesInfoToImGui();