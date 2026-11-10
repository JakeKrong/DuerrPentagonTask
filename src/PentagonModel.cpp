#include "PentagonModel.h"

#include <QtMath>

#include <cmath>

namespace
{
	constexpr double DefaultRadius = 120.0;  //Distance from the centre to each corner of the starting shape
}

PentagonModel::PentagonModel(QObject* parent)
	: QObject(parent)
	, m_fillColor(0x4a, 0x90, 0xd9)
	, m_text(tr("Pentagon"))
{
	//Corners evenly spaced round a circle, 72 degrees apart, starting straight up.
	//Scene y grows downwards, so -90 degrees is up.
	for (int i = 0; i < PointCount; ++i)
	{
		const double angle = qDegreesToRadians(-90.0 + i * 360.0 / PointCount);
		m_points << QPointF(DefaultRadius * std::cos(angle), DefaultRadius * std::sin(angle));
	}
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
	//QPointF's == is a fuzzy compare, so tiny rounding differences don't count as a change
	if (m_points.at(index) == point)
		return;

	m_points[index] = point;
	emit pointsChanged();
}

void PentagonModel::translate(const QPointF& offset)
{
	if (offset.isNull())
		return;

	m_points.translate(offset);
	emit pointsChanged();
}

void PentagonModel::setArea(double newArea)
{
	const double oldArea = area();
	if (newArea <= 0.0 || qFuzzyIsNull(oldArea) || qFuzzyCompare(newArea, oldArea))
		return;

	//Area grows with the square of the size, so lengths scale by the square root of the area ratio.
	//Scaling about any fixed point gives the same area; the centre only decides which spot stays put.
	const double factor = std::sqrt(newArea / oldArea);
	const QPointF centre = m_points.boundingRect().center();
	for (QPointF& p : m_points)
		p = centre + (p - centre) * factor;
	emit pointsChanged();
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
