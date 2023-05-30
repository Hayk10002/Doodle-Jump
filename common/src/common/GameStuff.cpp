#include "GameStuff.hpp"
#include <filesystem>
#include <fstream>
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <level/LevelGenerator.hpp>
#include <common/Returners.hpp>
#include <DoodleJumpConfig.hpp>

template<ReturnType RT>
void returnerToImGui(const std::type_info& ti, const nl::json& j)
{
	if (ti == typeid(Returner<RT>))
	{
		Returner<RT>* ret = getReturnerPointerFromJson<Returner<RT>>(j);
		ret->from_json(j);
		ret->toImGui();
	}
}

template<int... types>
void returnersToImGui(const std::type_info& ti, const nl::json& j, std::integer_sequence<int, types...>)
{
	(returnerToImGui<ReturnType(types)>(ti, j), ...);
}

void DoodleJumpClipboard::toImGui()
{
	ImGui::BeginChild("Clipboard (Recent copies)", ImVec2(0, ImGui::GetContentRegionAvail().y / 2.f), true);
	ImGui::Text("Recently copied");
	ImGui::Separator();
	for (size_t i = 0; i < recent_copies.size(); i++)
	{
		ImGui::Text(std::format("[{}]:", i + 1).c_str()); ImGui::SameLine(); 
		nl::json j = nl::json::parse(recent_copies[i].second);
		if (recent_copies[i].first == typeid(Generation))
		{
			Generation* gen = getGenerationPointerFromJson<Generation>(j);
			gen->from_json(j);
			gen->toImGui();
		}
		returnersToImGui(recent_copies[i].first, j, std::make_integer_sequence<int, ReturnTypesCount>());

	}
	ImGui::EndChild();
	
	ImGui::BeginChild("Clipboard (Recent deletions)", ImVec2(0, ImGui::GetContentRegionAvail().y), true);
	ImGui::Text("Recently deleted");
	ImGui::Separator();
	for (size_t i = 0; i < recent_deletions.size(); i++)
	{
		ImGui::Text(std::format("[{}]:", i + 1).c_str()); ImGui::SameLine(); 
		nl::json j = nl::json::parse(recent_deletions[i].second);
		if (recent_deletions[i].first == typeid(Generation))
		{
			Generation* gen = getGenerationPointerFromJson<Generation>(j);
			gen->from_json(j);
			gen->toImGui();
		}
		returnersToImGui(recent_deletions[i].first, j, std::make_integer_sequence<int, ReturnTypesCount>());

	}
	ImGui::EndChild();
}

bool DoodleJumpClipboard::empty() const
{
	return recent_copies.empty() && recent_deletions.empty();
}

void saveFilesInfoToImGui()
{
	namespace fs = std::filesystem;
	ImGui::Text("Saved generations and returners");
	ImGui::Separator();
	fs::path file_to_delete;
	for (const auto& file : fs::directory_iterator(RESOURCES_PATH "Saved generations and returners"))
	{
		bool tree_node_open = ImGui::TreeNodeEx(file.path().filename().string().c_str(), ImGuiTreeNodeFlags_DefaultOpen);
		if (ImGui::BeginPopupContextItem(("Deleting file" + file.path().string()).c_str()))
		{
			if (ImGui::SmallButton("Delete file")) file_to_delete = file.path();
			ImGui::EndPopup();
		}
		if(tree_node_open)
		{
			nl::json j;
			std::ifstream fin(file.path());
			fin >> j;
			fin.close();
			std::string to_erase;
			for (const auto& [name, obj] : j.items())
			{
				ImGui::Text(name.c_str());
				if (ImGui::BeginPopupContextItem(("Deleting object" + file.path().string() + name).c_str()))
				{
					if (ImGui::SmallButton("Delete")) to_erase = name;
					ImGui::EndPopup();
				}
			}
			if (to_erase != "")
			{
				j.erase(to_erase);
				std::ofstream fout(file.path());
				fout << j;
			}
			ImGui::TreePop();
		}
	}
	if (file_to_delete != fs::path{}) fs::remove(file_to_delete);
}
