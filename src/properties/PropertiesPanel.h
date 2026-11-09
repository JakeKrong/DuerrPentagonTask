//The properties list: fill colour, area, the 5 points and the label text, grouped in a form.
//Connects each property widget to the model in both directions.

#pragma once

#include <QWidget>

class PentagonModel;

class PropertiesPanel : public QWidget
{
	Q_OBJECT

public:
	explicit PropertiesPanel(PentagonModel* model, QWidget* parent = nullptr);

private:
	PentagonModel* m_model = nullptr;
};
