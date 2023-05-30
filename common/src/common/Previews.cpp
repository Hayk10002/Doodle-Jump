#include "Previews.hpp"

#include <SelbaWard/Line.hpp>
#include <SelbaWard/Ring.hpp>
#include <Thor/Vectors.hpp>

void Previews::point(sf::Vector2f val, sf::Vector2f zero)
{
	if (!checkWindow()) return;
	val.y = -val.y;
	sf::CircleShape sh(4);
	sh.setOrigin(4, 4);
	sh.setPosition(val + zero);
	sh.setFillColor(sf::Color(0, 0, 0));
	window->draw(sh);
}

void Previews::height(float height, sf::Vector2f zero)
{
	if (!checkWindow()) return;
	sf::Vector2f
		start_left = zero - sf::Vector2f(25, 0),
		start_right = zero + sf::Vector2f(25, 0),
		end = zero - sf::Vector2f(0, height),
		end_left = end - sf::Vector2f(25, 0),
		end_right = end + sf::Vector2f(25, 0);
	sw::Line l1(zero, end, 2, sf::Color::Black);
	sw::Line l2(start_left, start_right, 2, sf::Color::Black);
	sw::Line l3(end_left, end_right, 2, sf::Color::Black);

	window->draw(l1);
	window->draw(l2);
	window->draw(l3);
}

void Previews::offset(sf::Vector2f offset, sf::Vector2f zero)
{
	if (!checkWindow()) return;
	offset.y = -offset.y;
	sf::Vector2f vec = thor::rotatedVector(offset, 90.f);
	if (vec != sf::Vector2f{}) thor::setLength(vec, 10.f);

	window->draw(sw::Line(zero, zero + offset, 2, sf::Color::Black));
	window->draw(sw::Line(zero, zero + vec, 2, sf::Color::Black));
	window->draw(sw::Line(zero, zero - vec, 2, sf::Color::Black));
	window->draw(sw::Line(zero + offset, zero + offset + vec, 2, sf::Color::Black));
	window->draw(sw::Line(zero + offset, zero + offset - vec, 2, sf::Color::Black));
}

void Previews::speed(sf::Vector2f speed, sf::Vector2f zero)
{
	if (!checkWindow()) return;
	speed.y = -speed.y;
	float l = std::min(10.f, thor::length(speed));
	l *= std::pow(2, 0.5);
	sf::Vector2f vec1 = speed, vec2 = speed;
	if (vec1 != sf::Vector2f{}) thor::setLength(vec1, l);
	if (vec2 != sf::Vector2f{}) thor::setLength(vec2, l);
	thor::rotate(vec1, 45.f);
	thor::rotate(vec2, -45.f);

	window->draw(sw::Line(zero, zero + speed, 2, sf::Color::Black));
	window->draw(sw::Line(zero + speed - vec1, zero + speed, 2, sf::Color::Black));
	window->draw(sw::Line(zero + speed - vec2, zero + speed, 2, sf::Color::Black));
}

void Previews::xBoundary(float boundary, sf::Vector2f zero)
{
	if (!checkWindow()) return;
	float bottom_y = window->mapPixelToCoords({ 0, (int)window->getSize().y }).y;
	float top_y = window->mapPixelToCoords({ 0, 0 }).y;

	window->draw(sw::Line({ zero.x + boundary, bottom_y }, { zero.x + boundary, top_y }, 2, sf::Color::Black));
}

void Previews::yBoundary(float boundary, sf::Vector2f zero)
{
	if (!checkWindow()) return;
	boundary = -boundary;
	float left_x = window->mapPixelToCoords({ (int)window->getSize().x, 0 }).x;
	float right_x = window->mapPixelToCoords({ 0, 0 }).x;

	window->draw(sw::Line({ left_x, zero.y + boundary }, { right_x, zero.y + boundary }, 2, sf::Color::Black));
}

void Previews::rect(sf::Vector2f half_size, sf::Vector2f zero)
{
	if (!checkWindow()) return;
	half_size.y = -half_size.y;
	
	sf::RectangleShape sh(2.f * half_size);
	sh.setOrigin(half_size);
	sh.setPosition(zero);
	sh.setFillColor(sf::Color(0, 0, 0, 80));

	window->draw(sh);
}

void Previews::circle(float radius, sf::Vector2f zero)
{
	if (!checkWindow()) return;

	sf::CircleShape sh(radius);
	sh.setOrigin(radius, radius);
	sh.setPosition(zero);
	sh.setFillColor(sf::Color(0, 0, 0, 80));

	window->draw(sh);
}

void Previews::deflect(float max_rotation, sf::Vector2f center, sf::Vector2f zero)
{
	if (!checkWindow()) return;
	if (center == sf::Vector2f{}) return;
	if (max_rotation <= 0) return;

	float l = thor::length(center);
	sw::Ring ring(l + 1.f, (l - 1.f) / (l + 1.f));
	ring.setColor(sf::Color::Black);
	ring.setOrigin(l + 1.f, l + 1.f);
	ring.setPosition(zero);
	ring.setSectorSize(max_rotation / 180);
	ring.setSectorOffset((90 - thor::polarAngle(center) - max_rotation) / 360);

	window->draw(ring);
}

bool Previews::checkWindow()
{
	return window;
}