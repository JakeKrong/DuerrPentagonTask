#include "properties/PointProperty.h"

#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QSignalBlocker>

namespace
{
	QDoubleSpinBox* createCoordinateBox(const QString& prefix, QWidget* parent)
	{
		auto* box = new QDoubleSpinBox(parent);
		box->setPrefix(prefix);
		box->setDecimals(2);  //The range comes from setBounds()
		box->setSingleStep(5.0);  //Arrow keys, arrow buttons and the mouse wheel move the point 5 units at a time
		//Report a typed value on Enter or when leaving the box, not after every keystroke
		box->setKeyboardTracking(false);
		return box;
	}
}

PointProperty::PointProperty(QWidget* parent)
	: QWidget(parent)
	, m_x(createCoordinateBox(tr("X "), this))
	, m_y(createCoordinateBox(tr("Y "), this))
{
	auto* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(m_x);
	layout->addWidget(m_y);

	//Change only the coordinate that was edited. Taking the other one from its spin box would apply that box's
	//rounding to 2 decimals and nudge the point slightly.
	connect(m_x, &QDoubleSpinBox::valueChanged, this, [this](double x) {
		m_point.setX(x);
		emit pointEdited(m_point);
	});
	connect(m_y, &QDoubleSpinBox::valueChanged, this, [this](double y) {
		m_point.setY(y);
		emit pointEdited(m_point);
	});
}

void PointProperty::setBounds(const QRectF& bounds)
{
	//setRange() can change a box's value to fit (emitting valueChanged), and that mustn't look like a user edit
	const QSignalBlocker blockX(m_x);
	const QSignalBlocker blockY(m_y);
	m_x->setRange(bounds.left(), bounds.right());
	m_y->setRange(bounds.top(), bounds.bottom());
}

void PointProperty::setPoint(const QPointF& point)
{
	m_point = point;

	//setValue() emits valueChanged just like a user edit would, so block the boxes' signals while updating them
	const QSignalBlocker blockX(m_x);
	const QSignalBlocker blockY(m_y);
	m_x->setValue(point.x());
	m_y->setValue(point.y());
}
