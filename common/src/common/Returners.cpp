#include "Returners.hpp"
#include <level/Level.hpp>
#include <level/LevelGenerator.hpp>

void toImGui(float& val, const float* min, const float* max)
{
	ImGui::DragScalar(std::format("##{}", (uintptr_t)&val).c_str(), ImGuiDataType_Float, &val, 1.f, min, max);
}

void toImGui(int& val, const int* min, const int* max)
{
	ImGui::DragScalar(std::format("##{}", (uintptr_t)&val).c_str(), ImGuiDataType_S32, &val, 1.f, min, max);
}

void toImGui(size_t& val, const size_t* min, const size_t* max)
{
	ImGui::DragScalar(std::format("##{}", (uintptr_t)&val).c_str(), ImGuiDataType_U64, &val, 1.f, min, max);
}

void toImGui(sf::Vector2f& val)
{
	ImGui::DragScalarN(std::format("##{}", (uintptr_t)&val).c_str(), ImGuiDataType_Float, &val, 2);
}

template<ReturnType RT>
	requires (RT == Height)
GeneratedHeightReturner<RT>::ValT GeneratedHeightReturner<RT>::get() const
{
	return -Generation::getCurrentLevelForGenerating()->level_generator.getGeneratedHeight();
}

void dummy()
{
	GeneratedHeightReturner<Height> ret;
}
