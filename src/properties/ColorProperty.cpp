#include "properties/ColorProperty.h"

#include <QColorDialog>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPixmap>
#include <QRegularExpressionValidator>
#include <QToolButton>

namespace
{
	//Hex digits only, with an optional leading #; returns an invalid colour unless all 6 digits are there
	QColor colorFromHex(QString text)
	{
		text.remove(QLatin1Char('#'));
		if (text.size() != 6)
			return {};
		return QColor::fromString(QLatin1Char('#') + text);
	}
}

ColorProperty::ColorProperty(QWidget* parent)
	: QWidget(parent)
	, m_swatch(new QToolButton(this))
	, m_hex(new QLineEdit(this))
{
	m_swatch->setToolTip(tr("Choose a colour"));
	m_swatch->setIconSize(QSize(32, 16));

	//Only lets #RRGGBB be typed; half-typed values are allowed while editing
	m_hex->setValidator(new QRegularExpressionValidator(QRegularExpression(QStringLiteral("#?[0-9A-Fa-f]{0,6}")), m_hex));
	m_hex->setToolTip(tr("Colour as #RRGGBB"));

	auto* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(m_swatch);
	layout->addWidget(m_hex);

	connect(m_swatch, &QToolButton::clicked, this, &ColorProperty::chooseColor);

	//textEdited, not textChanged: only the user's typing, not setText() calls from setColor()
	connect(m_hex, &QLineEdit::textEdited, this, &ColorProperty::applyHexText);

	//Leaving the field with a half-typed colour puts the current one back
	connect(m_hex, &QLineEdit::editingFinished, this, [this] { m_hex->setText(m_color.name()); });
}

void ColorProperty::setColor(const QColor& color)
{
	m_color = color;

	QPixmap swatch(m_swatch->iconSize());
	swatch.fill(color);
	m_swatch->setIcon(swatch);

	//When the change came from typing in the field, it already shows this colour. Rewriting it would move the
	//cursor to the end (and change the user's capitals to lower case) in the middle of typing.
	if (colorFromHex(m_hex->text()) != color)
		m_hex->setText(color.name());
}

void ColorProperty::chooseColor()
{
	//getColor() returns an invalid colour if the dialog is cancelled
	const QColor chosen = QColorDialog::getColor(m_color, this, tr("Fill colour"));
	if (!chosen.isValid() || chosen == m_color)
		return;

	setColor(chosen);
	emit colorEdited(chosen);
}

void ColorProperty::applyHexText(const QString& text)
{
	//Applied as soon as all 6 digits are typed, so the canvas updates without needing Enter
	const QColor typed = colorFromHex(text);
	if (!typed.isValid() || typed == m_color)
		return;

	setColor(typed);
	emit colorEdited(typed);
}
