#include "canvas/CanvasView.h"

#include <QPainter>

CanvasView::CanvasView(QGraphicsScene* scene, QWidget* parent)
	: QGraphicsView(scene, parent)
{
	setRenderHint(QPainter::Antialiasing);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void CanvasView::resizeEvent(QResizeEvent* event)
{
	QGraphicsView::resizeEvent(event);
	//Scale view so  whole drawing area is visible and keeping its proportions
	//Also runs when the dock next to it is resized, since that resizes this widget too
	fitInView(sceneRect(), Qt::KeepAspectRatio);
}

void CanvasView::drawBackground(QPainter* painter, const QRectF& rect)
{
	//rect is the part being redrawn, in scene coordinates: grey margin everywhere, then the drawing area in white
	painter->fillRect(rect, palette().color(QPalette::Window));
	const QRectF canvas = sceneRect();
	painter->fillRect(canvas, Qt::white);

	QPen border(Qt::gray, 1);
	border.setCosmetic(true);
	painter->setPen(border);
	painter->drawRect(canvas);
}
