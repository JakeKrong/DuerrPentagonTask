//A small draggable dot on one corner of the polygon. Dragging it moves that one point in the model.

#pragma once

#include <QGraphicsEllipseItem>
#include <QObject>

class PentagonModel;

//A QObject for the same reason as PolygonItem: so it can receive the model's signals
class VertexHandle : public QObject, public QGraphicsEllipseItem
{
	Q_OBJECT

public:
	VertexHandle(PentagonModel* model, int index, QGraphicsItem* parent = nullptr);

protected:
	//Qt calls this whenever something about the item changes, including its position
	QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
	void updatePosition();

	PentagonModel* m_model = nullptr;
	int m_index = 0;  //Which of the 5 points this handle controls
};
