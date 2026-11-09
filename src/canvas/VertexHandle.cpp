#include "canvas/VertexHandle.h"

#include "PentagonModel.h"

VertexHandle::VertexHandle(PentagonModel* model, int index, QGraphicsItem* parent)
	: QGraphicsEllipseItem(parent)
	, m_model(model)
	, m_index(index)
{
}
