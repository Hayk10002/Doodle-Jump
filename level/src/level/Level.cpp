#include "Level.hpp"
#include <type_traits>
#include <numbers>
#include <cmath>

#include <imgui.h>

#include <common/DebugImGui.hpp>

std::string ViewScrolling::getName() const
{
    return m_name;
}

sf::Time ViewScrolling::getDuration() const
{
	return m_duration;
}

sf::View ViewScrolling::lerp(const sf::View& start, const sf::View& end, float alpha)
{
	
	auto lerp = []<class T> requires (std::is_same_v<T, sf::Vector2f> || std::is_same_v<T, sf::FloatRect>) (const T & start, const T & end, float alpha)
	{
		if constexpr (std::is_same_v<T, sf::Vector2f>)
		{
			sf::Vector2f res;
			res.x = std::lerp(start.x, end.x, alpha);
			res.y = std::lerp(start.y, end.y, alpha);
			return res;
		}
		else
		{
			sf::FloatRect res;
			res.left = std::lerp(start.left, end.left, alpha);
			res.top = std::lerp(start.top, end.top, alpha);
			res.width = std::lerp(start.width, end.width, alpha);
			res.height = std::lerp(start.height, end.height, alpha);
			return res;
		}
	};


	sf::View view;

	view.setCenter(lerp(start.getCenter(), end.getCenter(), alpha));
	view.setRotation(std::lerp(start.getRotation(), end.getRotation() + std::round((start.getRotation() - end.getRotation()) / 360) * 360, alpha));
	view.setSize(lerp(start.getSize(), end.getSize(), alpha));
	view.setViewport(lerp(start.getViewport(), end.getViewport(), alpha));

	return view;
}

ViewScrolling::ViewScrolling(sf::Time duration):
	m_duration(duration)
{}

Level::Level()	
{
	setScrollingType(InstantScrolling());
}

void Level::setWindow(sf::RenderWindow * window)
{
	m_window_ptr = window;
	m_view = m_view_destination = window->getDefaultView();
	m_background.setBackgroundCoveringArea(window);
}

sf::RenderWindow* Level::getWindow() const
{
	return m_window_ptr;
}

void Level::setBackground(const ImageBackground& background)
{
	m_background = background;
	m_background.setBackgroundCoveringArea(m_window_ptr);
}

const ImageBackground& Level::getBackground() const
{
	return m_background;
}

void Level::scrollUp(float offset, bool instant)
{
	scroll({ 0, -offset }, instant);
}

void Level::scrollDown(float offset, bool instant)
{
	scroll({ 0, offset }, instant);
}

void Level::scrollLeft(float offset, bool instant)
{
	scroll({ -offset, 0 }, instant);
}

void Level::scrollRight(float offset, bool instant)
{
	scroll({ offset, 0 }, instant);
}

void Level::scroll(float offset_x, float offset_y, bool instant)
{
	scroll({ offset_x, offset_y }, instant);
}

void Level::scroll(sf::Vector2f offset, bool instant)
{
	m_in_scroll = true;
	if (!instant) m_view = getCurrentView();
	m_scroll_timer.restart();
	m_view_destination.move(offset);
	if (instant) m_view = m_view_destination;
}

void Level::zoomX(float scale, bool instant)
{
	zoom({ scale, 0 }, instant);
}

void Level::zoomY(float scale, bool instant)
{
	zoom({ 0, scale }, instant);
}

void Level::zoom(float scale, bool instant)
{
	zoom({ scale, scale }, instant);
}

void Level::zoom(float scale_x, float scale_y, bool instant)
{
	zoom({ scale_x, scale_y }, instant);
}

void Level::zoom(sf::Vector2f scale, bool instant)
{
	m_in_scroll = true;
	if(!instant) m_view = getCurrentView();
	m_scroll_timer.restart();
	sf::Vector2f size = m_view_destination.getSize();
	size.x /= scale.x;
	size.y /= scale.y;
	m_view_destination.setSize(size);
	if (instant) m_view = m_view_destination;
}

void Level::rotate(float angle, bool instant)
{
	
	m_in_scroll = true;
	if (!instant) m_view = getCurrentView();
	m_scroll_timer.restart();
	m_view_destination.rotate(angle);
	if (instant) m_view = m_view_destination;
}

std::string Level::getScrollingTypeName() const
{
	return m_scrolling_type_ptr->getName();
}

void Level::update()
{
	updateScrolling();
	m_background.update();
}

void Level::updateScrolling()
{
	if (m_in_scroll)
	{
		sf::Time passed_time = m_scroll_timer.getElapsedTime();
		sf::View view = getCurrentView();
		if (passed_time > m_scrolling_type_ptr->getDuration()) m_in_scroll = false;
		m_window_ptr->setView(view);
	}
	else m_view = m_view_destination;
}

sf::View Level::getCurrentView() const
{
	return m_scrolling_type_ptr->getView(m_scroll_timer.getElapsedTime(), m_view, m_view_destination);

}

void Level::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(m_background, states);
}

InstantScrolling::InstantScrolling() :
	ViewScrolling(sf::Time::Zero)
{}

sf::View InstantScrolling::getView(sf::Time passed_time, const sf::View & start_view, const sf::View & end_view) const
{
	return end_view;
}

LinearScrolling::LinearScrolling(sf::Time duration):
	ViewScrolling(duration)
{}

sf::View LinearScrolling::getView(sf::Time passed_time, const sf::View & start_view, const sf::View & end_view) const
{
	return lerp(start_view, end_view, passed_time / m_duration);
}

ExponentialScrolling::ExponentialScrolling(sf::Time duration, float power) :
	ViewScrolling(duration)
{
	m_exponent_base = std::pow(std::numbers::e_v<float>, m_polynolial_power + power);
	m_turning_point = -m_polynolial_power / std::log(m_exponent_base) + 1;
	m_polynomial_scale = std::pow(m_exponent_base, -m_turning_point) / std::pow(m_turning_point - 1, m_polynolial_power);
}

sf::View ExponentialScrolling::getView(sf::Time passed_time, const sf::View & start_view, const sf::View & end_view) const
{
	float alpha = passed_time / m_duration;
	alpha = 1 - ((alpha < m_turning_point) ? std::pow(m_exponent_base, -alpha) : m_polynomial_scale * std::pow(alpha - 1, m_polynolial_power));
	DebugImGui::add_imgui_call("alpha", ImGui::Text, "Exponential alpha: %f, %f", alpha,m_turning_point);
	return lerp(start_view, end_view, alpha);
}
