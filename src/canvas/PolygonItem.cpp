#include "canvas/PolygonItem.h"

#include "PentagonModel.h"

#include <QBrush>
#include <QPen>

PolygonItem::PolygonItem(PentagonModel* model, QGraphicsItem* parent)
	: QGraphicsPolygonItem(parent)
	, m_model(model)
{
	//Winding fill keeps a self-intersecting shape solid; the default odd-even rule leaves holes where it overlaps itself
	setFillRule(Qt::WindingFill);

	//Any member function can be connected to a signal; it doesn't have to be declared as a slot
	connect(m_model, &PentagonModel::pointsChanged, this, &PolygonItem::updateShape);
	connect(m_model, &PentagonModel::fillColorChanged, this, &PolygonItem::updateColor);

	//Signals only report changes, so pick up the current state once at the start
	updateShape();
	updateColor();
}

void PolygonItem::updateShape()
{
	//The points are in scene coordinates and the item stays at (0,0), so they can be used as they are
	setPolygon(m_model->polygon());
}

void PolygonItem::updateColor()
{
	const QColor color = m_model->fillColor();
	setBrush(color);

	//Cosmetic pen: always 2 pixels wide on screen, however far the view is zoomed
	QPen outline(color.darker(150), 2);
	outline.setCosmetic(true);
	setPen(outline);
}
