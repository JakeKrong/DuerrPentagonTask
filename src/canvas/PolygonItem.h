//The filled polygon on the canvas. Draws the model's 5 points in its fill colour;
//dragging the body moves all 5 points in the model.

#pragma once

#include <QGraphicsPolygonItem>
#include <QObject>

class PentagonModel;

//Also a QObject, so the item itself can receive the model's signals. Qt then breaks those connections
//automatically when the item is deleted. QGraphicsObject is built the same way; QObject has to be the first base.
class PolygonItem : public QObject, public QGraphicsPolygonItem
{
	Q_OBJECT

public:
	explicit PolygonItem(PentagonModel* model, QGraphicsItem* parent = nullptr);

private:
	void updateShape();
	void updateColor();

	PentagonModel* m_model = nullptr;
};
