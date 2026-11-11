//Single source of truth for the shape: the polygon's 5 points, its fill colour and the label text.
//The canvas and the properties panel both read from this, write through its setters and update on its signals.

#pragma once

#include <QColor>
#include <QObject>
#include <QPolygonF>
#include <QRectF>
#include <QString>

class PentagonModel : public QObject
{
	Q_OBJECT

public:
	static constexpr int PointCount = 5;
	static constexpr QRectF Bounds{ -400.0, -300.0, 800.0, 600.0 }; // Every point is bounded by min & max coordinates of canvas

	//Starts as a regular pentagon centered on the scene origin
	explicit PentagonModel(QObject* parent = nullptr);

	static QPointF clampToBounds(const QPointF& point); //Nearest point inside Bounds

	//Points are in scene coordinates, so moving the shape on the canvas changes them
	QPolygonF polygon() const { return m_points; }
	QPointF point(int index) const;
	QColor fillColor() const { return m_fillColor; }
	QString text() const { return m_text; }

	//Area not stored, always worked out from the current points
	double area() const;

	static double polygonArea(const QPolygonF& polygon);

	//Setters only emit a signal when the value actually changes,
	//which stops the canvas and the panel from endlessly updating each other.
	//(A point outside Bounds is moved to the nearest point inside)
	void setPoint(int index, const QPointF& point);

	//Scales the shape about the centre of its bounding box, does not grow bigger than it fits in Bounds. 
	//Simply returns for areas <= 0, or when the current shape has no area to scale (eg. all points in a line)
	void setArea(double area);

	//Invalid colours (e.g. from a cancelled QColorDialog) are ignored
	void setFillColor(const QColor& color);
	void setText(const QString& text);

	//Moves the whole shape
	void translate(const QPointF& offset);

	//Puts the polygon back as it started: the points (so its shape, size and position) and the fill colour.
	//The label text is kept. Like the setters, it only signals for what actually changes.
	void resetPolygon();

signals:
	//Signal for changes to points (area changes with them, so don't have it's own signal)
	void pointsChanged();
	void fillColorChanged(const QColor& color);
	void textChanged(const QString& text);

private:
	QPolygonF m_points;
	QColor m_fillColor;
	QString m_text;
};
