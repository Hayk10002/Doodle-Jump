#include "Returners.hpp"
#include <level/Level.hpp>
#include <level/LevelGenerator.hpp>

void toImGui(float& val)
{
	ImGui::InputScalar(std::format("##{}", (uintptr_t)&val).c_str(), ImGuiDataType_Float, &val);
}

void toImGui(int& val)
{
	ImGui::InputScalar(std::format("##{}", (uintptr_t)&val).c_str(), ImGuiDataType_S32, &val);
}

void toImGui(size_t& val)
{
	ImGui::InputScalar(std::format("##{}", (uintptr_t)&val).c_str(), ImGuiDataType_U64, &val);
}

void toImGui(sf::Vector2f& val)
{
	ImGui::InputScalarN(std::format("##{}", (uintptr_t)&val).c_str(), ImGuiDataType_Float, &val, 2);
}

template<ReturnType RT>
	requires (RT == Height)
GeneratedHeightReturner<RT>::ValT GeneratedHeightReturner<RT>::get() const
{
	return Generation::getCurrentLevelForGenerating()->level_generator.getGeneratedHeight();
}

void dummy()
{
	GeneratedHeightReturner<Height> ret;
}
