#include <SFML/Graphics.hpp>
#include <PlinthSfml.hpp>
#include <iostream>
#include <DoodleJumpConfig.hpp>
#include <drawables/ImageBackground.hpp>

int main()
{
	sf::RenderWindow window(sf::VideoMode(600, 800), "Doodle Jump");
	sf::Texture background_texture;
	background_texture.loadFromFile(RESOURCE_PATH"background.png");
	ImageBackground ib(background_texture, { window.getView().getCenter() - window.getView().getSize() / 2.f, window.getView().getSize() });
	ib.setRotation(65);
	ib.setScale(1, 3);
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
			if (event.type == sf::Event::Resized)
			{
				sf::View v(sf::FloatRect(0, 0, event.size.width, event.size.height));
				ib.setBackgroundCoveringArea({ v.getCenter() - v.getSize() / 2.f, v.getSize() });
				v.setSize(v.getSize()*5.f);
				window.setView(v);
			}
			if (event.type == sf::Event::KeyPressed)
			{
				switch (event.key.code)
				{
				case sf::Keyboard::W:
					ib.move(0, -5);
					break;
				case sf::Keyboard::A:
					ib.move(-5, 0);
					break;
				case sf::Keyboard::S:
					ib.move(0, 5);
					break;
				case sf::Keyboard::D:
					ib.move(5, 0);
					break;

				}
			}
		}

		ib.update();

		window.clear();
		window.draw(ib);
		window.display();
	}
	return 0;
}