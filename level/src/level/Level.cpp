#include "Level.hpp"
#include <type_traits>
#include <numbers>
#include <cmath>

#include <imgui.h>

#include <common/DebugImGui.hpp>

SimpleView::SimpleView()
{
	reset({ {0, 0}, {1000, 1000} });
}

SimpleView::SimpleView(const sf::View& view):
	m_center(view.getCenter()),
	m_size(view.getSize()),
	m_rotation(view.getRotation()),
	m_viewport(view.getViewport())
{}

SimpleView::SimpleView(const FloatRect& rectangle)
{
	reset(rectangle);
}

SimpleView::SimpleView(const Vector2f& center, const Vector2f& size):
	m_center(center),
	m_size(size)
{}

void SimpleView::setCenter(float x, float y)
{
	setCenter({ x, y });
}

void SimpleView::setCenter(const Vector2f& center)
{
	m_center = center;
}

void SimpleView::setSize(float width, float height)
{
	setSize({ width, height });
}

void SimpleView::setSize(const Vector2f& size)
{
	m_size = size;
}

void SimpleView::setRotation(float angle)
{
	m_rotation = std::fmodf(angle, 360);
}

void SimpleView::setViewport(const FloatRect& viewport)
{
	m_viewport = viewport;
}

void SimpleView::reset(const FloatRect& rectangle)
{
	m_center.x = rectangle.left + rectangle.width / 2.f;
	m_center.y = rectangle.top + rectangle.height / 2.f;
	m_size.x = rectangle.width;
	m_size.y = rectangle.height;
	m_rotation = 0;
}

const sf::Vector2f& SimpleView::getCenter() const
{
	return m_center;
}

const sf::Vector2f& SimpleView::getSize() const
{
	return m_size;
}

float SimpleView::getRotation() const
{
	return m_rotation;
}

const sf::FloatRect& SimpleView::getViewport() const
{
	return m_viewport;
}

void SimpleView::move(float offsetX, float offsetY)
{
	move({ offsetX, offsetY });
}

void SimpleView::move(const Vector2f& offset)
{
	setCenter(m_center + offset);
}

void SimpleView::rotate(float angle)
{
	setRotation(m_rotation + angle);
}

void SimpleView::zoom(float factor)
{
	setSize(m_size * factor);
}

sf::View SimpleView::asSFMLView() const
{
	sf::View res;
	res.setCenter(m_center);
	res.setRotation(m_rotation);
	res.setSize(m_size);
	res.setViewport(m_viewport);
	return res;
}

std::string ViewScrolling::getName() const
{
    return m_name;
}

sf::Time ViewScrolling::getDuration() const
{
	return m_duration;
}

SimpleView ViewScrolling::lerp(const SimpleView& start, const SimpleView& end, float alpha)
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


	SimpleView view;

	view.setCenter(lerp(start.getCenter(), end.getCenter(), alpha));
	view.setRotation(std::lerp(start.getRotation(), end.getRotation() + std::round((start.getRotation() - end.getRotation()) / 360) * 360, alpha));
	view.setSize(lerp(start.getSize(), end.getSize(), alpha));
	view.setViewport(lerp(start.getViewport(), end.getViewport(), alpha));

	return view;
}

ViewScrolling::ViewScrolling(sf::Time duration):
	m_duration(duration)
{}

Level::Object::Object(std::shared_ptr<const size_t*> level) :
	level(level),
	identifier(identifier_counter++)
{}

Level::Object::Object(const Object& other):
	level(other.level),
	drawable_ptr(other.drawable_ptr),
	update(other.update),
	identifier(other.identifier)
{}

Level::Object Level::Object::get_duplicate()
{
	Object obj(*this);
	obj.identifier = Object::identifier_counter++;
	return obj;
}

bool Level::Object::operator==(Object other) const
{
	return std::tie(level, drawable_ptr, identifier) == std::tie(other.level, other.drawable_ptr, other.identifier);
}

Level::Level()	
{
	setScrollingType(InstantScrolling());
}

Level::~Level()
{
	for (auto& obj : m_objects) (*obj.level) = nullptr;
}

void Level::setWindow(sf::RenderWindow * window)
{
	m_window_ptr = window;
	m_view = m_view_destination = window->getDefaultView();
}

sf::RenderWindow* Level::getWindow() const
{
	return m_window_ptr;
}

size_t Level::Object::Hasher::operator()(const Object& obj) const
{
	return std::_Hash_array_representation<char>((char*)&obj, sizeof(Object));
}

bool Level::removeObject(Object obj)
{
	if (!isMyObject(obj)) return false;
	(*obj.level) = nullptr;
	if(!removeFromUpdateList(obj)) return false;
	if(!removeFromDrawList(obj)) return false;;
	m_objects.erase(obj);
	return true;
}

bool Level::moveObjectUpInUpdateOrder(Object obj, int count)
{
	if (!isMyObject(obj)) return false;
	size_t pos = getObjectsItrForUpdateList(obj) - m_update_order.begin();
	if (pos == m_update_order.size()) return false;

	m_update_order.erase(m_update_order.begin() + pos);
	pos += count;
	pos = std::clamp<size_t>(pos, 0, m_update_order.size());
	m_update_order.insert(m_update_order.begin() + pos, obj);
	return true;
}

bool Level::moveObjectDownInUpdateOrder(Object obj, int count)
{
	return moveObjectUpInUpdateOrder(obj, -count);
}

bool Level::updateFirst(Object obj)
{
	return moveObjectDownInUpdateOrder(obj, m_update_order.size());
}

bool Level::updateLast(Object obj)
{
	return moveObjectUpInUpdateOrder(obj, m_update_order.size());
}

bool Level::addToUpdateList(Object obj, int position)
{
	if (!isMyObject(obj)) return false;
	if(isInUpdateList(obj)) return false;
	if (position < 0) position += m_update_order.size() + 1;
	position = std::clamp<int>(position, 0, m_update_order.size());
	m_update_order.insert(m_update_order.begin() + position, obj); 
	return true;
}

bool Level::isInUpdateList(Object obj)
{
	return getObjectsItrForUpdateList(obj) != m_update_order.end();
}

bool Level::removeFromUpdateList(Object obj)
{
	if (!isMyObject(obj)) return false;
	auto itr = getObjectsItrForUpdateList(obj);
	if (itr == m_update_order.end()) return false;
	m_update_order.erase(itr);
	return true;
}

bool Level::moveObjectUpInDrawOrder(Object obj, int count)
{
	if (!isMyObject(obj)) return false;
	size_t pos = getObjectsItrForDrawList(obj) - m_draw_order.begin();
	if (pos == m_draw_order.size()) return false;

	m_draw_order.erase(m_draw_order.begin() + pos);
	pos += count;
	pos = std::clamp<size_t>(pos, 0, m_draw_order.size());
	m_draw_order.insert(m_draw_order.begin() + pos, obj);
	return true;
}

bool Level::moveObjectDownInDrawOrder(Object obj, int count)
{
	return moveObjectUpInDrawOrder(obj, -count);
}

bool Level::drawFirst(Object obj)
{
	return moveObjectDownInDrawOrder(obj, m_draw_order.size());
}

bool Level::drawLast(Object obj)
{
	return moveObjectUpInDrawOrder(obj, m_draw_order.size());
}

bool Level::addToDrawList(Object obj, int position)
{
	if (!isMyObject(obj)) return false;
	if (isInDrawList(obj))return false;
	if (position < 0) position += m_draw_order.size() + 1;
	position = std::clamp<int>(position, 0, m_draw_order.size());
	m_draw_order.insert(m_draw_order.begin() + position, obj);
	return true;
}

bool Level::isInDrawList(Object obj)
{
	return getObjectsItrForDrawList(obj) != m_draw_order.end();
}

bool Level::removeFromDrawList(Object obj)
{
	if (!isMyObject(obj)) return false;
	auto itr = getObjectsItrForDrawList(obj);
	if (itr == m_draw_order.end()) return false;
	m_draw_order.erase(itr);
	return true;
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

void Level::update(sf::Time dt)
{
	updateScrolling();
	for (auto& obj : m_update_order) obj.update(dt);
}

void Level::updateScrolling()
{
	if (m_in_scroll)
	{
		sf::Time passed_time = m_scroll_timer.getElapsedTime();
		SimpleView view = getCurrentView();
		if (passed_time > m_scrolling_type_ptr->getDuration()) m_in_scroll = false;
		m_window_ptr->setView(view.asSFMLView());
	}
	else m_view = m_view_destination;
}

SimpleView Level::getCurrentView() const
{
	return m_scrolling_type_ptr->getView(m_scroll_timer.getElapsedTime(), m_view, m_view_destination);

}

std::deque<Level::Object>::iterator Level::getObjectsItrForUpdateList(Object obj)
{
	if (!isMyObject(obj)) return m_update_order.end();
	return std::find(m_update_order.begin(), m_update_order.end(), obj);
}

std::deque<Level::Object>::iterator Level::getObjectsItrForDrawList(Object obj)
{
	if (!isMyObject(obj)) return m_draw_order.end();
	return std::find(m_draw_order.begin(), m_draw_order.end(), obj);
}

bool Level::isMyObject(Object obj)
{
	return **obj.level == m_identifier;
}

void Level::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	for (auto& obj : m_draw_order) target.draw(*obj.drawable_ptr, states);
}

InstantScrolling::InstantScrolling() :
	ViewScrolling(sf::Time::Zero)
{}

SimpleView InstantScrolling::getView(sf::Time passed_time, const SimpleView & start_view, const SimpleView & end_view) const
{
	return end_view;
}

LinearScrolling::LinearScrolling(sf::Time duration):
	ViewScrolling(duration)
{}

SimpleView LinearScrolling::getView(sf::Time passed_time, const SimpleView & start_view, const SimpleView & end_view) const
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

SimpleView ExponentialScrolling::getView(sf::Time passed_time, const SimpleView & start_view, const SimpleView & end_view) const
{
	float alpha = passed_time / m_duration;
	alpha = 1 - ((alpha < m_turning_point) ? std::pow(m_exponent_base, -alpha) : m_polynomial_scale * std::pow(alpha - 1, m_polynolial_power));
	DebugImGui::add_imgui_call("alpha", ImGui::Text, "Exponential alpha: %f, %f", alpha,m_turning_point);
	return lerp(start_view, end_view, alpha);
}


