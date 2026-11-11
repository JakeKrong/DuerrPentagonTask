//Editor for a colour: a swatch button that opens QColorDialog, plus a hex field.
//Doesn't know about the model; PropertiesPanel connects it.

#pragma once

#include <QColor>
#include <QWidget>

class QLineEdit;
class QToolButton;

class ColorProperty : public QWidget
{
	Q_OBJECT

public:
	explicit ColorProperty(QWidget* parent = nullptr);

	QColor color() const { return m_color; }
	//Shows a colour. Doesn't emit colorEdited, since the change didn't come from the user.
	void setColor(const QColor& color);

signals:
	//Only for colours the user picks in the dialog or types in the hex field
	void colorEdited(const QColor& color);

private:
	void chooseColor();
	void applyHexText(const QString& text);

	QToolButton* m_swatch = nullptr;
	QLineEdit* m_hex = nullptr;
	QColor m_color;
};
