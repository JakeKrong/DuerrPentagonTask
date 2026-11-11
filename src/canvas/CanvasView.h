//The canvas widget: always shows the whole drawing area (the scene rect), scaled to fit, on a grey background.

#pragma once

#include <QGraphicsView>

class CanvasView : public QGraphicsView
{
	Q_OBJECT

public:
	explicit CanvasView(QGraphicsScene* scene, QWidget* parent = nullptr);

protected:
	void resizeEvent(QResizeEvent* event) override;
	void drawBackground(QPainter* painter, const QRectF& rect) override;
};
