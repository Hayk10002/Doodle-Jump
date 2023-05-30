#include <iostream>
#include <deque>

#include <SFML/Graphics.hpp>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <SFML/imgui-SFML.h>

#include <Thor/Input.hpp>
#include <Thor/Resources.hpp>
#include <Thor/Math.hpp>
#include <Thor/Vectors.hpp>
#include <SelbaWard.hpp>
#include <nlohmann/json.hpp>

#include <DoodleJumpConfig.hpp>
#include <common/Resources.hpp>
#include <common/GameStuff.hpp>
#include <common/Previews.hpp>
#include <level/Level.hpp>



int main()
{
	//setup the window
	constexpr size_t WINDOW_WIDTH = 500;
	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, 800), "Doodle Jump");
	sf::Vector2u window_prev_size(window.getSize());
	window.setKeyRepeatEnabled(false);

	//setup Dear ImGui
	ImGui::SFML::Init(window);
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;

	//setup thor resource manager
	init_resources();

	//create the level
	Level level(window);
	size_t current_level = 0;
	
	//setup the user actions
	thor::ActionMap<std::string> action_map;
	action_map["close"] = thor::Action(sf::Event::Closed);
	action_map["resize"] = thor::Action(sf::Event::Resized);
	//frame clock
	size_t frame_count = 0;
	sf::Clock deltaClock;
	sf::Time full_time, dt;
	
	Previews::window = &window;

	while (window.isOpen())
	{
		//event handling
		action_map.clearEvents();
		sf::Event event;
		while (window.pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(window, event);
			switch (event.type)
			{
			case sf::Event::KeyPressed:
			case sf::Event::KeyReleased:
			case sf::Event::TextEntered:
				if (!ImGui::GetIO().WantCaptureKeyboard) action_map.pushEvent(event);
				break;
			case sf::Event::MouseButtonPressed:
			case sf::Event::MouseButtonReleased:
			case sf::Event::MouseMoved:
			case sf::Event::MouseWheelScrolled:
				if (!ImGui::GetIO().WantCaptureMouse) action_map.pushEvent(event);
				break;
			default:
				action_map.pushEvent(event);
				break;
			}

			if (event.type == sf::Event::MouseWheelScrolled)
			{
				level.scene.scrollUp(100.f * event.mouseWheelScroll.delta);
			}
		}

		//updating

		frame_count++;
		full_time += (dt = deltaClock.restart());

		ImGui::SFML::Update(window, dt);
		ImGui::Begin("Info");
		ImGui::BeginChild("Info_level", ImVec2(ImGui::GetContentRegionAvail().x * 0.5, 0), true);
		ImGui::Text("Frame count: %d", frame_count);
		ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);
		level.level_generator.toImGui();

		ImGui::Text("Level no.");
		ImGui::SameLine();
		ImGui::InputScalar("##Level no", ImGuiDataType_U64, &current_level);
		ImGui::SameLine();
		if (ImGui::SmallButton("Load")) level.loadFromFile(std::format(RESOURCES_PATH"Levels/level{}.json", current_level));
		ImGui::SameLine();
		if (ImGui::SmallButton("Save")) level.saveToFile(std::format(RESOURCES_PATH"Levels/level{}.json", current_level));

		ImGui::EndChild();
		ImGui::SameLine();

		ImGui::BeginChild("Info_clipboard", ImVec2(ImGui::GetContentRegionAvail().x * 0.5, 0));
		doodle_jump_clipboard.toImGui();
		ImGui::EndChild();

		ImGui::SameLine();
		ImGui::BeginChild("Info_save_files", ImVec2{}, true);
		saveFilesInfoToImGui();
		ImGui::EndChild();
		ImGui::End();

		if (action_map.isActive("close"))
			window.close();
		if (action_map.isActive("resize"))
		{
			window.setSize({ WINDOW_WIDTH, window.getSize().y });
			sf::Vector2f scale = { (float)window_prev_size.x / window.getSize().x, (float)window_prev_size.y / window.getSize().y };
			window_prev_size = { window.getSize().x, window.getSize().y };
			level.scene.zoom(scale, true);
		}

		action_map.update(window);
		level.ib.update();
		level.scene.updateScrolling();

		//drawing
		window.clear();
		window.draw(level.ib);
		if (level.level_generator.getGeneration()) level.level_generator.getGeneration()->drawPreview({0.f, (float)window.getSize().y});
		ImGui::SFML::Render(window);
		window.display();
	}



	ImGui::SFML::Shutdown();
	release_resources();

	return 0;
}

