#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <SFML/imgui-SFML.h>
#include <PlinthSfml.hpp>
#include <DoodleJumpConfig.hpp>
#include <drawables/ImageBackground.hpp>

int main()
{
	sf::RenderWindow window(sf::VideoMode(600, 800), "Doodle Jump");
	window.setFramerateLimit(60);
	ImGui::SFML::Init(window);
	sf::Texture background_texture;
	background_texture.loadFromFile(RESOURCES_PATH"background.png");
	ImageBackground ib(background_texture, &window);
	sf::Clock deltaClock;
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(window, event);
			if (event.type == sf::Event::Closed)
				window.close();
			if (event.type == sf::Event::Resized)
			{
				sf::View v(sf::FloatRect(0, 0, event.size.width, event.size.height));
				window.setView(v);
			}
			if (!ImGui::GetIO().WantCaptureMouse)
			{
				if (event.type == sf::Event::MouseWheelScrolled)
				{
					sf::View v = window.getView();
					v.zoom(std::pow(1.3f, event.mouseWheelScroll.delta));
					window.setView(v);
				}
			}
			if (!ImGui::GetIO().WantCaptureKeyboard)
			{
				if (event.type == sf::Event::KeyPressed)
				{
					switch (event.key.code)
					{
					case sf::Keyboard::W:
						ib.move(0, -1050);
						break;
					case sf::Keyboard::A:
						ib.move(-1050, 0);
						break;
					case sf::Keyboard::S:
						ib.move(0, 1050);
						break;
					case sf::Keyboard::D:
						ib.move(1050, 0);
						break;

					}
				}
			}
		}

		ib.update();

		ImGui::SFML::Update(window, deltaClock.restart());

		ImGui::Begin("Info");
		ImGui::Text("Background Position %f, %f", ib.getPosition().x, ib.getPosition().y);
		ImGui::End();

		window.clear();
		window.draw(ib);
		ImGui::SFML::Render(window);
		window.display();
	}

	ImGui::SFML::Shutdown();
	return 0;
}