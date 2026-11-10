//Single source of truth for the shape: the polygon's 5 points, its fill colour and the label text.
//The canvas and the properties panel both read from it, write through its setters and update on its signals;
//they never talk to each other directly.

#pragma once

#include <QColor>
#include <QObject>
#include <QPolygonF>
#include <QString>

class PentagonModel : public QObject
{
	Q_OBJECT

public:
	static constexpr int PointCount = 5;

	//Starts as a regular pentagon centred on the scene origin
	explicit PentagonModel(QObject* parent = nullptr);

	//Points are in scene coordinates, so moving the shape on the canvas changes them
	QPolygonF polygon() const { return m_points; }
	QPointF point(int index) const;
	QColor fillColor() const { return m_fillColor; }
	QString text() const { return m_text; }

	//Area isn't stored: it's always worked out from the current points
	double area() const;
	//Shoelace formula. For a self-intersecting polygon the parts that wind in opposite directions cancel out,
	//so the result is less than the area you see filled
	static double polygonArea(const QPolygonF& polygon);

	//Setters only emit a signal when the value actually changes. That's what stops the canvas and the panel
	//from endlessly updating each other.
	void setPoint(int index, const QPointF& point);
	void translate(const QPointF& offset);
	//Scales the shape about the centre of its bounding box. Does nothing for areas <= 0, or when the current
	//shape has no area to scale (all points in a line)
	void setArea(double area);
	//Invalid colours (e.g. from a cancelled QColorDialog) are ignored
	void setFillColor(const QColor& color);
	void setText(const QString& text);

signals:
	//One signal for any change to the points; area changes with them, so it has no signal of its own
	void pointsChanged();
	void fillColorChanged(const QColor& color);
	void textChanged(const QString& text);

private:
	QPolygonF m_points;  //Always exactly PointCount points
	QColor m_fillColor;
	QString m_text;
};
