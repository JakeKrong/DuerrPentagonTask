//Editor for one point: X and Y spin boxes side by side.
//PropertiesPanel connects it to the model

#pragma once

#include <QPointF>
#include <QRectF>
#include <QWidget>

class QDoubleSpinBox;

class PointProperty : public QWidget
{
	Q_OBJECT

public:
	explicit PointProperty(QWidget* parent = nullptr);

	//The range the X and Y boxes allow
	void setBounds(const QRectF& bounds);

	QPointF point() const { return m_point; }
	//Shows a point. Doesn't emit pointEdited, since the change didn't come from the user.
	void setPoint(const QPointF& point);

signals:
	//Only for changes the user makes in the spin boxes (like QLineEdit's textEdited, as opposed to textChanged)
	void pointEdited(const QPointF& point);

private:
	QDoubleSpinBox* m_x = nullptr;
	QDoubleSpinBox* m_y = nullptr;
	QPointF m_point;  //Full precision; the spin boxes only show 2 decimals
};
