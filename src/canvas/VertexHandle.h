//A small draggable dot on one corner of the polygon. Dragging it moves that one point in the model.

#pragma once

#include <QGraphicsEllipseItem>

class PentagonModel;

class VertexHandle : public QGraphicsEllipseItem
{
public:
	VertexHandle(PentagonModel* model, int index, QGraphicsItem* parent = nullptr);

private:
	PentagonModel* m_model = nullptr;
	int m_index = 0;  //Which of the 5 points this handle controls
};
