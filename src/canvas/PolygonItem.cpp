#include "canvas/PolygonItem.h"

#include "PentagonModel.h"

PolygonItem::PolygonItem(PentagonModel* model, QGraphicsItem* parent)
	: QGraphicsPolygonItem(parent)
	, m_model(model)
{
}
