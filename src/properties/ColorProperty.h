//Editor for a colour: a swatch button that opens QColorDialog, plus a hex field.
//Doesn't know about the model; PropertiesPanel connects it.

#pragma once

#include <QWidget>

class ColorProperty : public QWidget
{
	Q_OBJECT

public:
	explicit ColorProperty(QWidget* parent = nullptr);
};
