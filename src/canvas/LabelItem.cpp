#include "canvas/LabelItem.h"

#include "PentagonModel.h"

LabelItem::LabelItem(PentagonModel* model, QGraphicsItem* parent)
	: QGraphicsTextItem(parent)
	, m_model(model)
{
}
