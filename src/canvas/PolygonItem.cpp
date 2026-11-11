#include "canvas/PolygonItem.h"

#include "PentagonModel.h"

#include <QBrush>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include <QPen>

PolygonItem::PolygonItem(PentagonModel* model, QGraphicsItem* parent)
	: QGraphicsPolygonItem(parent)
	, m_model(model)
{
	//Winding fill keeps a self-intersecting shape solid; the default odd-even rule leaves holes where it overlaps itself
	setFillRule(Qt::WindingFill);
	setCursor(Qt::OpenHandCursor);

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

void PolygonItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	//Only left-button drags move the shape. Accepting the press makes this item the scene's "mouse grabber",
	//so it receives the move and release events that follow, even if the mouse moves off the shape.
	if (event->button() != Qt::LeftButton)
	{
		event->ignore();
		return;
	}
	setCursor(Qt::ClosedHandCursor);
	m_dragStart = event->scenePos();
	m_pointAtDragStart = m_model->point(0);
	event->accept();
}

void PolygonItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	//The shape should have moved as far as the mouse has since the press. The model may have stopped it at the
	//canvas edge, so move it by what's still missing, measured from where it actually is, rather than adding up
	//the mouse's small steps. That way the same spot of the shape is back under the cursor when the mouse returns.
	//The model then signals, and this item and the handles redraw themselves from it.
	const QPointF wanted = event->scenePos() - m_dragStart;
	const QPointF movedSoFar = m_model->point(0) - m_pointAtDragStart;
	m_model->translate(wanted - movedSoFar);
}

void PolygonItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	setCursor(Qt::OpenHandCursor);
	QGraphicsPolygonItem::mouseReleaseEvent(event);
}
