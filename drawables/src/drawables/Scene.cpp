#include "Scene.hpp"
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

Scene::Object::Object(std::shared_ptr<const size_t*> scene) :
	scene(scene),
	identifier(identifier_counter++)
{}

Scene::Object::Object(const Object& other):
	scene(other.scene),
	drawable_ptr(other.drawable_ptr),
	update(other.update),
	identifier(other.identifier)
{}

Scene::Object Scene::Object::get_duplicate() const
{
	Object obj(*this);
	obj.identifier = Object::identifier_counter++;
	return obj;
}

bool Scene::Object::operator==(Object other) const
{
	return std::tie(scene, drawable_ptr, identifier) == std::tie(other.scene, other.drawable_ptr, other.identifier);
}

Scene::Scene()	
{
	setScrollingType(InstantScrolling());
}

Scene::~Scene()
{
	for (auto& obj : m_objects) (*obj.scene) = nullptr;
}

void Scene::setWindow(sf::RenderWindow * window)
{
	m_window_ptr = window;
	m_view = m_view_destination = window->getDefaultView();
}

sf::RenderWindow* Scene::getWindow() const
{
	return m_window_ptr;
}

size_t Scene::Object::Hasher::operator()(const Object& obj) const
{
	return std::_Hash_array_representation<char>((char*)&obj, sizeof(Object));
}

bool Scene::removeObject(Object obj)
{
	if (!isMyObject(obj)) return false;
	if(!removeFromUpdateList(obj)) return false;
	if(!removeFromDrawList(obj)) return false;;
	(*obj.scene) = nullptr;
	m_objects.erase(obj);
	return true;
}

bool Scene::moveObjectUpInUpdateOrder(Object obj, int count)
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

bool Scene::moveObjectDownInUpdateOrder(Object obj, int count)
{
	return moveObjectUpInUpdateOrder(obj, -count);
}

bool Scene::updateFirst(Object obj)
{
	return moveObjectDownInUpdateOrder(obj, m_update_order.size());
}

bool Scene::updateLast(Object obj)
{
	return moveObjectUpInUpdateOrder(obj, m_update_order.size());
}

bool Scene::addToUpdateList(Object obj, int position)
{
	if (!isMyObject(obj)) return false;
	if(isInUpdateList(obj)) return false;
	if (position < 0) position += m_update_order.size() + 1;
	position = std::clamp<int>(position, 0, m_update_order.size());
	m_update_order.insert(m_update_order.begin() + position, obj); 
	return true;
}

bool Scene::isInUpdateList(Object obj)
{
	return getObjectsItrForUpdateList(obj) != m_update_order.end();
}

bool Scene::removeFromUpdateList(Object obj)
{
	if (!isMyObject(obj)) return false;
	auto itr = getObjectsItrForUpdateList(obj);
	if (itr == m_update_order.end()) return false;
	m_update_order.erase(itr);
	return true;
}

bool Scene::moveObjectUpInDrawOrder(Object obj, int count)
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

bool Scene::moveObjectDownInDrawOrder(Object obj, int count)
{
	return moveObjectUpInDrawOrder(obj, -count);
}

bool Scene::drawFirst(Object obj)
{
	return moveObjectDownInDrawOrder(obj, m_draw_order.size());
}

bool Scene::drawLast(Object obj)
{
	return moveObjectUpInDrawOrder(obj, m_draw_order.size());
}

bool Scene::addToDrawList(Object obj, int position)
{
	if (!isMyObject(obj)) return false;
	if (isInDrawList(obj))return false;
	if (position < 0) position += m_draw_order.size() + 1;
	position = std::clamp<int>(position, 0, m_draw_order.size());
	m_draw_order.insert(m_draw_order.begin() + position, obj);
	return true;
}

bool Scene::isInDrawList(Object obj)
{
	return getObjectsItrForDrawList(obj) != m_draw_order.end();
}

bool Scene::removeFromDrawList(Object obj)
{
	if (!isMyObject(obj)) return false;
	auto itr = getObjectsItrForDrawList(obj);
	if (itr == m_draw_order.end()) return false;
	m_draw_order.erase(itr);
	return true;
}

void Scene::scrollUp(float offset, bool instant)
{
	scroll({ 0, -offset }, instant);
}

void Scene::scrollDown(float offset, bool instant)
{
	scroll({ 0, offset }, instant);
}

void Scene::scrollLeft(float offset, bool instant)
{
	scroll({ -offset, 0 }, instant);
}

void Scene::scrollRight(float offset, bool instant)
{
	scroll({ offset, 0 }, instant);
}

void Scene::scroll(float offset_x, float offset_y, bool instant)
{
	scroll({ offset_x, offset_y }, instant);
}

void Scene::scroll(sf::Vector2f offset, bool instant)
{
	m_in_scroll = true;
	if (!instant) m_view = getCurrentView();
	m_scroll_timer.restart();
	m_view_destination.move(offset);
	if (instant) m_view = m_view_destination;
}

void Scene::zoomX(float scale, bool instant)
{
	zoom({ scale, 0 }, instant);
}

void Scene::zoomY(float scale, bool instant)
{
	zoom({ 0, scale }, instant);
}

void Scene::zoom(float scale, bool instant)
{
	zoom({ scale, scale }, instant);
}

void Scene::zoom(float scale_x, float scale_y, bool instant)
{
	zoom({ scale_x, scale_y }, instant);
}

void Scene::zoom(sf::Vector2f scale, bool instant)
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

void Scene::rotate(float angle, bool instant)
{
	
	m_in_scroll = true;
	if (!instant) m_view = getCurrentView();
	m_scroll_timer.restart();
	m_view_destination.rotate(angle);
	if (instant) m_view = m_view_destination;
}

std::string Scene::getScrollingTypeName() const
{
	return m_scrolling_type_ptr->getName();
}

void Scene::updateObjects(sf::Time dt)
{
	for (int i = 0; i < m_update_order.size(); i++)
	{
		if (isMyObject(m_update_order[i])) m_update_order[i].update(dt);
		else m_update_order.erase(m_update_order.begin() + i--);
	}
}

void Scene::updateScrolling()
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

SimpleView Scene::getCurrentView() const
{
	return m_scrolling_type_ptr->getView(m_scroll_timer.getElapsedTime(), m_view, m_view_destination);

}

std::deque<Scene::Object>::iterator Scene::getObjectsItrForUpdateList(Object obj)
{
	if (!isMyObject(obj)) return m_update_order.end();
	return std::find(m_update_order.begin(), m_update_order.end(), obj);
}

std::deque<Scene::Object>::iterator Scene::getObjectsItrForDrawList(Object obj)
{
	if (!isMyObject(obj)) return m_draw_order.end();
	return std::find(m_draw_order.begin(), m_draw_order.end(), obj);
}

std::deque<Scene::Object>::const_iterator Scene::getObjectsItrForUpdateList(Object obj) const
{
	if (!isMyObject(obj)) return m_update_order.end();
	return std::find(m_update_order.begin(), m_update_order.end(), obj);
}

std::deque<Scene::Object>::const_iterator Scene::getObjectsItrForDrawList(Object obj) const
{
	if (!isMyObject(obj)) return m_draw_order.end();
	return std::find(m_draw_order.begin(), m_draw_order.end(), obj);
}

bool Scene::isMyObject(Object obj) const
{
	return *obj.scene != nullptr && **obj.scene == m_identifier;
}

void Scene::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	for (int i = 0; i < m_draw_order.size(); i++)
	{
		if (isMyObject(m_draw_order[i])) target.draw(*m_draw_order[i].drawable_ptr, states);
		else m_draw_order.erase(m_draw_order.begin() + i--);
	}
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


