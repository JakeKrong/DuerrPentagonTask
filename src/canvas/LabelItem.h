//The text label that goes with the polygon. Sits below the polygon (or above it near the bottom of the canvas) and follows it
//Double-clicking allows for editing the text in place, which writes it to the model

#pragma once

#include <QGraphicsTextItem>

class PentagonModel;

//QGraphicsTextItem already a QObject (via QGraphicsObject), so no second base class needed (like other class)
class LabelItem : public QGraphicsTextItem
{
	Q_OBJECT

public:
	explicit LabelItem(PentagonModel* model, QGraphicsItem* parent = nullptr);

	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

protected:
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;
	void focusOutEvent(QFocusEvent* event) override;

private:
	QString placeholderText() const;
	void updateText(const QString& text);
	void updateWidth();
	void updatePosition();

	PentagonModel* m_model = nullptr;
};
