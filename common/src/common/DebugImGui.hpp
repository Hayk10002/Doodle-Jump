#pragma once
#include <functional>
#include <map>
#include <string>

class DebugImGui
{
	static std::map<std::string, std::function<void()>> callables;

public:
	template<class F, class ... T>
	static void add_imgui_call(std::string name, F callable, T ... params)
	{
		callables[name] = [=]()
		{
			callable(params...); 
		};
	}

	static void call(std::string name);
};

