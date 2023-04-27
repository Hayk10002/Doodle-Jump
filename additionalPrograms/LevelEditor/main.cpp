#include <iostream>

#include <SFML/Graphics.hpp>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <SFML/imgui-SFML.h>

#include <Thor/Input.hpp>
#include <Thor/Resources.hpp>
#include <Thor/Math.hpp>
#include <SelbaWard.hpp>
#include <nlohmann/json.hpp>

#include <DoodleJumpConfig.hpp>
#include <common/Resources.hpp>
#include <common/DebugImGui.hpp>
#include <common/Utils.hpp>
#include <common/GameStuff.hpp>
#include <drawables/ImageBackground.hpp>
#include <drawables/Scene.hpp>
#include <level/Level.hpp>
#include <level/LevelGenerator.hpp>
#include <gameObjects/Doodle.hpp>
#include <gameObjects/Tiles.hpp>
#include <gameObjects/Items.hpp>
#include <gameObjects/Monsters.hpp>





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
	std::string save_file_name{ "level.json" };
	
	//setup the user actions
	thor::ActionMap<std::string> action_map;
	action_map["close"] = thor::Action(sf::Event::Closed);
	action_map["resize"] = thor::Action(sf::Event::Resized);
	//frame clock
	size_t frame_count = 0;
	sf::Clock deltaClock;
	sf::Time full_time, dt;

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
		ImGui::ShowDemoWindow();
		ImGui::Begin("Info");
		ImGui::Text("Frame count: %d", frame_count);
		ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);
		level.level_generator.toImGui();
		ImGui::InputText("Save file", &save_file_name);
		if (ImGui::Button("Load from the save file")) level.loadFromFile(RESOURCES_PATH + save_file_name);
		if (ImGui::Button("Save to the save file")) level.saveToFile(RESOURCES_PATH + save_file_name);
		if (ImGui::TreeNode("Hello"))
		{
			ImGui::Text("World?");
			ImGui::TreePop();
		}
		
		
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
		ImGui::SFML::Render(window);
		window.display();
	}



	ImGui::SFML::Shutdown();
	release_resources();

	return 0;
}
