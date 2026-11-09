//The filled polygon on the canvas. Draws the model's 5 points in its fill colour;
//dragging the body moves all 5 points in the model.

#pragma once

#include <QGraphicsPolygonItem>

class PentagonModel;

class PolygonItem : public QGraphicsPolygonItem
{
public:
	explicit PolygonItem(PentagonModel* model, QGraphicsItem* parent = nullptr);

private:
	PentagonModel* m_model = nullptr;
};
