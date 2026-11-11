#include "PentagonModel.h"

#include <QtMath>

#include <algorithm>
#include <cmath>

namespace
{
	constexpr double DefaultRadius = 120.0;

	//To contruct default shape (when initialising or resetting)
	//Corners evenly spaced round a circle, 72 degrees apart, starting straight up (scene y grows downwards, so -90 degrees is up).
	QPolygonF defaultPolygon()
	{
		QPolygonF points;
		for (int i = 0; i < PentagonModel::PointCount; ++i)
		{
			const double angle = qDegreesToRadians(-90.0 + i * 360.0 / PentagonModel::PointCount);
			points << QPointF(DefaultRadius * std::cos(angle), DefaultRadius * std::sin(angle));
		}
		return points;
	}

	QColor defaultFillColor()
	{
		return QColor(0x4a, 0x90, 0xd9); //Default blue colour
	}
}

PentagonModel::PentagonModel(QObject* parent)
	: QObject(parent)
	, m_points(defaultPolygon())
	, m_fillColor(defaultFillColor())
	, m_text(tr("Pentagon"))
{}

QPointF PentagonModel::clampToBounds(const QPointF& point)
{
	return QPointF(qBound(Bounds.left(), point.x(), Bounds.right()), qBound(Bounds.top(), point.y(), Bounds.bottom()));
}

QPointF PentagonModel::point(int index) const
{
	Q_ASSERT(index >= 0 && index < PointCount);
	return m_points.at(index);
}

double PentagonModel::area() const
{
	return polygonArea(m_points);
}

double PentagonModel::polygonArea(const QPolygonF& polygon)
{
	//Shoelace formula: A = |sum of (x_i * y_(i+1) - x_(i+1) * y_i)| / 2, where the last point pairs with the first
	double twiceSignedArea = 0.0;
	for (qsizetype i = 0; i < polygon.size(); ++i)
	{
		const QPointF& a = polygon.at(i);
		const QPointF& b = polygon.at((i + 1) % polygon.size());
		twiceSignedArea += a.x() * b.y() - b.x() * a.y();
	}
	//The sign only says which way round the points go (clockwise or anticlockwise), so drop it
	return std::abs(twiceSignedArea) / 2.0;
}

void PentagonModel::setPoint(int index, const QPointF& point)
{
	Q_ASSERT(index >= 0 && index < PointCount);
	const QPointF clamped = clampToBounds(point);
	//QPointF's == is a fuzzy compare, so tiny rounding differences don't count as a change
	if (m_points.at(index) == clamped)
		return;

	m_points[index] = clamped;
	emit pointsChanged();
}

void PentagonModel::translate(const QPointF& offset)
{
	//Stop at the edge of the canvas, limiting the offset so the shape's bounding box stays inside Bounds
	const QRectF box = m_points.boundingRect();
	const QPointF allowed(qBound(Bounds.left() - box.left(), offset.x(), Bounds.right() - box.right()),
	                      qBound(Bounds.top() - box.top(), offset.y(), Bounds.bottom() - box.bottom()));
	if (allowed.isNull())
		return;

	for (QPointF& p : m_points)
		p = clampToBounds(p + allowed);  //The clamp only removes rounding errors right at the edge
	emit pointsChanged();
}

void PentagonModel::setArea(double newArea)
{
	const double oldArea = area();
	if (newArea <= 0.0 || qFuzzyIsNull(oldArea) || qFuzzyCompare(newArea, oldArea))
		return;

	//Area grows with the square of the size, so lengths scale by the square root of the area ratio.
	//Scaling about any fixed point gives the same area; the centre only decides which spot stays put.
	double factor = std::sqrt(newArea / oldArea);
	const QRectF box = m_points.boundingRect();
	const QPointF centre = box.center();

	//Grow no bigger than fits: the room from the centre to the nearest canvas edge, over half the box's size
	const double roomX = std::min(centre.x() - Bounds.left(), Bounds.right() - centre.x());
	const double roomY = std::min(centre.y() - Bounds.top(), Bounds.bottom() - centre.y());
	factor = std::min({ factor, roomX / (box.width() / 2), roomY / (box.height() / 2) });
	if (qFuzzyCompare(factor, 1.0))
		return;  //Already as big as the canvas allows

	for (QPointF& p : m_points)
		p = clampToBounds(centre + (p - centre) * factor);  //The clamp only removes rounding errors at the edge
	emit pointsChanged();
}

void PentagonModel::resetPolygon()
{
	const QPolygonF original = defaultPolygon();
	if (m_points != original)
	{
		m_points = original;
		emit pointsChanged();
	}
	setFillColor(defaultFillColor());
}

void PentagonModel::setFillColor(const QColor& color)
{
	if (!color.isValid() || color == m_fillColor)
		return;

	m_fillColor = color;
	emit fillColorChanged(m_fillColor);
}

void PentagonModel::setText(const QString& text)
{
	if (text == m_text)
		return;

	m_text = text;
	emit textChanged(m_text);
}
