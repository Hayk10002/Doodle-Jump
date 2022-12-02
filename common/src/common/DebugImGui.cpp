#include "DebugImGui.hpp"

std::map<std::string, std::function<void()>> DebugImGui::callables{};

void DebugImGui::call(std::string name)
{
	try
	{
		callables[name]();
	}
	catch (std::exception) {}
}
