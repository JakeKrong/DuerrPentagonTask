//Editor for one point: X and Y spin boxes side by side.
//Doesn't know about the model; PropertiesPanel connects it.

#pragma once

#include <QWidget>

class PointProperty : public QWidget
{
	Q_OBJECT

public:
	explicit PointProperty(QWidget* parent = nullptr);
};
