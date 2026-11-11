//The properties list: fill colour, area, the 5 points and the label text, grouped in a form.
//Connects each property widget to the model in both directions.

#pragma once

#include <QList>
#include <QWidget>

class ColorProperty;
class PentagonModel;
class PointProperty;
class QDoubleSpinBox;
class QPlainTextEdit;

class PropertiesPanel : public QWidget
{
	Q_OBJECT

public:
	explicit PropertiesPanel(PentagonModel* model, QWidget* parent = nullptr);

private:
	void updatePoints();
	void updateText(const QString& text);

	PentagonModel* m_model = nullptr;
	ColorProperty* m_fillColor = nullptr;
	QDoubleSpinBox* m_area = nullptr;
	QList<PointProperty*> m_points;
	QPlainTextEdit* m_text = nullptr;
};
