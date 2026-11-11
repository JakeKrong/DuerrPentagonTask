//The filled polygon on the canvas
//Draws the model's 5 points in its fill colour, dragging the body moves all 5 points in the model

#pragma once

#include <QGraphicsPolygonItem>
#include <QObject>

class PentagonModel;

//Also a QObject, so the item itself can receive the model's signals. 
//Qt then breaks those connections automatically when the item is deleted. QGraphicsObject is built the same way
class PolygonItem : public QObject, public QGraphicsPolygonItem
{
	Q_OBJECT

public:
	explicit PolygonItem(PentagonModel* model, QGraphicsItem* parent = nullptr);

protected:
	//Dragging the body moves the model's points, not the item. The item always stays at (0,0).
	//(ItemIsMovable would move the item instead, and the points in the model would never change.)
	void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
	void updateShape();
	void updateColor();

	PentagonModel* m_model = nullptr;
	QPointF m_dragStart;        //Where the mouse was pressed, in scene coordinates
	QPointF m_pointAtDragStart; //Where the model's first point was at that moment
};
