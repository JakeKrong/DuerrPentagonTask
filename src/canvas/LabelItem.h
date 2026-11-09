//The text label that goes with the polygon. Sits below the polygon and follows it;
//double-click to edit the text in place, which writes it to the model.

#pragma once

#include <QGraphicsTextItem>

class PentagonModel;

//QGraphicsTextItem is a QObject (via QGraphicsObject), so unlike the other items it can use Q_OBJECT
class LabelItem : public QGraphicsTextItem
{
	Q_OBJECT

public:
	explicit LabelItem(PentagonModel* model, QGraphicsItem* parent = nullptr);

private:
	PentagonModel* m_model = nullptr;
};
