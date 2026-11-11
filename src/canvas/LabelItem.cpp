#include "canvas/LabelItem.h"

#include "PentagonModel.h"

#include <QAbstractTextDocumentLayout>
#include <QFont>
#include <QFontMetricsF>
#include <QKeyEvent>
#include <QPainter>
#include <QTextCursor>
#include <QTextDocument>

namespace
{
	constexpr double GapToPolygon = 12.0;
}

LabelItem::LabelItem(PentagonModel* model, QGraphicsItem* parent)
	: QGraphicsTextItem(parent)
	, m_model(model)
{
	QFont labelFont = font();
	labelFont.setPointSize(14);
	setFont(labelFont);

	//The canvas is always white (CanvasView), so the text is always black. Left to default, the colour would come
	//from the system theme instead: white with a dark Windows theme, which can't be seen on the white canvas.
	setDefaultTextColor(Qt::black);
	setToolTip(tr("Double-click to edit"));
	setPlainText(m_model->text());

	//Canvas to model: every edit made here (each keystroke) goes straight to the model
	connect(document(), &QTextDocument::contentsChanged, this, [this] { m_model->setText(toPlainText()); });
	connect(document(), &QTextDocument::contentsChanged, this, &LabelItem::updateWidth);

	//Model to canvas
	connect(m_model, &PentagonModel::textChanged, this, &LabelItem::updateText);

	//Stay centred under the polygon as it moves, and as the text gets wider or narrower
	connect(m_model, &PentagonModel::pointsChanged, this, &LabelItem::updatePosition);
	connect(document()->documentLayout(), &QAbstractTextDocumentLayout::documentSizeChanged, this, &LabelItem::updatePosition);
	updateWidth();
	updatePosition();
}

void LabelItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	QGraphicsTextItem::paint(painter, option, widget);

	//A hint in place of empty text, except while editing (the cursor shows where typing will go then)
	if (document()->isEmpty() && textInteractionFlags() == Qt::NoTextInteraction)
	{
		painter->setPen(Qt::gray);
		painter->setFont(font());
		painter->drawText(boundingRect(), Qt::AlignCenter, placeholderText());
	}
}

QString LabelItem::placeholderText() const
{
	return tr("Double-click to add text");
}

void LabelItem::updateWidth()
{
	//An empty label would shrink to a few pixels and be almost impossible to double-click. While it's empty,
	//it keeps the width of the hint that paint() draws; otherwise (-1) it's as wide as its text.
	const double hintWidth = QFontMetricsF(font()).horizontalAdvance(placeholderText()) + 2 * document()->documentMargin();
	setTextWidth(document()->isEmpty() ? hintWidth : -1);
}

void LabelItem::updateText(const QString& text)
{
	//When the edit came from typing here, the text already matches. Calling setPlainText() anyway would
	//rebuild the document and jump the cursor back to the start in the middle of typing.
	if (text != toPlainText())
		setPlainText(text);
}

void LabelItem::updatePosition()
{
	//Centred under the polygon's bounding box, or above it when there isn't room below on the canvas
	const QRectF polygonBounds = m_model->polygon().boundingRect();
	const QRectF canvas = PentagonModel::Bounds;
	const QSizeF size = boundingRect().size();

	double y = polygonBounds.bottom() + GapToPolygon;
	if (y + size.height() > canvas.bottom())
		y = polygonBounds.top() - GapToPolygon - size.height();

	//Kept on the canvas sideways too, for when the polygon is near the left or right edge
	const double x = qBound(canvas.left(), polygonBounds.center().x() - size.width() / 2, canvas.right() - size.width());
	setPos(x, y);
}

void LabelItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
	//Not editable until double-clicked, so a stray click doesn't start editing
	if (textInteractionFlags() == Qt::NoTextInteraction)
	{
		setTextInteractionFlags(Qt::TextEditorInteraction);
		setFocus(Qt::MouseFocusReason);
		update();  //Repaint without the empty-text hint
	}
	//With editing on, the base class treats the double-click as a text edit: it places the cursor and selects a word
	QGraphicsTextItem::mouseDoubleClickEvent(event);
}

void LabelItem::keyPressEvent(QKeyEvent* event)
{
	//Enter adds a new line (the text is freeform), so Escape is the key that finishes editing
	if (event->key() == Qt::Key_Escape)
	{
		clearFocus();
		return;
	}
	QGraphicsTextItem::keyPressEvent(event);
}

void LabelItem::focusOutEvent(QFocusEvent* event)
{
	//Clicking anywhere else (or Escape) ends editing. Drop the selection too, so it isn't left highlighted.
	setTextInteractionFlags(Qt::NoTextInteraction);
	QTextCursor cursor = textCursor();
	cursor.clearSelection();
	setTextCursor(cursor);
	update();  //Repaint, with the empty-text hint if the text was cleared
	QGraphicsTextItem::focusOutEvent(event);
}
