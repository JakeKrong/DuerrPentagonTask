#include "canvas/VertexHandle.h"

#include "PentagonModel.h"

#include <QBrush>
#include <QCursor>
#include <QPen>

namespace
{
	constexpr double Radius = 6.0;  //On screen pixels, since the handle ignores the view's zoom
}

VertexHandle::VertexHandle(PentagonModel* model, int index, QGraphicsItem* parent)
	: QGraphicsEllipseItem(-Radius, -Radius, 2 * Radius, 2 * Radius, parent)
	, m_model(model)
	, m_index(index)
{
	//The circle is centred on the item's origin, so the item's position is exactly the corner it controls
	setBrush(Qt::white);
	QPen outline(QColor(0x30, 0x30, 0x30), 1.5);
	outline.setCosmetic(true);
	setPen(outline);
	setCursor(Qt::SizeAllCursor);
	setToolTip(tr("Point %1").arg(index + 1));
	setZValue(1);  //Above the polygon, so where they overlap the handle is what gets clicked

	//ItemSendsGeometryChanges is needed for itemChange() to hear about position changes.
	//ItemIgnoresTransformations keeps the handle the same size on screen if the view is zoomed.
	setFlags(ItemIsMovable | ItemSendsGeometryChanges | ItemIgnoresTransformations);

	connect(m_model, &PentagonModel::pointsChanged, this, &VertexHandle::updatePosition);
	updatePosition();
}

QVariant VertexHandle::itemChange(GraphicsItemChange change, const QVariant& value)
{
	//The handle has no parent item, so its position is in scene coordinates, the same as the model's points.
	//This also runs when updatePosition() moves the handle to match the model. The model then sees that the
	//point hasn't changed and doesn't signal again, which is what stops an endless loop.
	if (change == ItemPositionHasChanged)
		m_model->setPoint(m_index, value.toPointF());

	return QGraphicsEllipseItem::itemChange(change, value);
}

void VertexHandle::updatePosition()
{
	setPos(m_model->point(m_index));
}
