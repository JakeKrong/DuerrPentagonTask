#include "properties/PropertiesPanel.h"

#include "PentagonModel.h"
#include "properties/ColorProperty.h"
#include "properties/PointProperty.h"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSignalBlocker>
#include <QVBoxLayout>

PropertiesPanel::PropertiesPanel(PentagonModel* model, QWidget* parent)
	: QWidget(parent)
	, m_model(model)
	, m_fillColor(new ColorProperty(this))
	, m_area(new QDoubleSpinBox(this))
	, m_text(new QPlainTextEdit(this))
{
	//Area: editing it scales the polygon about its centre. It can't be bigger than the canvas.
	m_area->setRange(0.0, PentagonModel::Bounds.width() * PentagonModel::Bounds.height());
	m_area->setDecimals(2);
	m_area->setSingleStep(1000.0);  //Arrow keys, arrow buttons and the mouse wheel change the area 1000 square pixels at a time
	m_area->setKeyboardTracking(false);
	m_area->setToolTip(tr("Type a new area to scale the polygon about its centre"));

	//Label text: freeform, so several lines are allowed. Tab moves to the next field instead of typing a tab.
	m_text->setTabChangesFocus(true);
	m_text->setFixedHeight(m_text->fontMetrics().lineSpacing() * 4 + 2 * m_text->frameWidth() + 8);

	//Text boxes ask to grow vertically by default, which would stretch the whole group; this one is a fixed height
	m_text->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	auto* polygonGroup = new QGroupBox(tr("Polygon"), this);
	auto* polygonForm = new QFormLayout(polygonGroup);
	polygonForm->addRow(tr("Fill colour"), m_fillColor);
	polygonForm->addRow(tr("Area (px%1)").arg(QChar(0x00B2)), m_area); //Squared (power 2) written as code so source remains ASCII
	for (int i = 0; i < PentagonModel::PointCount; ++i)
	{
		auto* point = new PointProperty(polygonGroup);

		//The same limits as the canvas, so the boxes can show any point the model allows and nothing else
		point->setBounds(PentagonModel::Bounds);
		polygonForm->addRow(tr("Point %1").arg(i + 1), point);
		m_points << point;

		//Panel to model, for each point
		connect(point, &PointProperty::pointEdited, this, [this, i](const QPointF& p) { m_model->setPoint(i, p); });
	}

	//Under the polygon's properties, spanning both columns. The model does the work; the canvas and these fields
	//update from its signals like for any other change.
	auto* reset = new QPushButton(tr("Reset polygon"), polygonGroup);
	reset->setToolTip(tr("Put the polygon back to its starting shape, size, position and colour"));
	polygonForm->addRow(reset);
	connect(reset, &QPushButton::clicked, m_model, &PentagonModel::resetPolygon);

	auto* labelGroup = new QGroupBox(tr("Label"), this);
	auto* labelForm = new QFormLayout(labelGroup);
	labelForm->addRow(tr("Text"), m_text);

	auto* layout = new QVBoxLayout(this);
	layout->addWidget(polygonGroup);
	layout->addWidget(labelGroup);
	layout->addStretch();

	//Panel to model
	connect(m_fillColor, &ColorProperty::colorEdited, m_model, &PentagonModel::setFillColor);
	connect(m_area, &QDoubleSpinBox::valueChanged, this, [this](double area) {
		m_model->setArea(area);

		//The model ignores areas it can't use (0, or a flat shape), so show what it actually has now
		updatePoints();
	});
	connect(m_text, &QPlainTextEdit::textChanged, this, [this] { m_model->setText(m_text->toPlainText()); });

	//Model to panel
	connect(m_model, &PentagonModel::pointsChanged, this, &PropertiesPanel::updatePoints);
	connect(m_model, &PentagonModel::fillColorChanged, m_fillColor, &ColorProperty::setColor);
	connect(m_model, &PentagonModel::textChanged, this, &PropertiesPanel::updateText);

	//Signals only report changes, so show the current state once at the start
	updatePoints();
	m_fillColor->setColor(m_model->fillColor());
	updateText(m_model->text());
}

void PropertiesPanel::updatePoints()
{
	for (int i = 0; i < PentagonModel::PointCount; ++i)
		m_points[i]->setPoint(m_model->point(i));

	//Block the area box's signals while updating it. It rounds to 2 decimals, so letting it report this change
	//would send a slightly different area back to the model, which would then rescale the polygon.
	const QSignalBlocker blocker(m_area);
	const double area = m_model->area();
	m_area->setValue(area);
	m_area->setEnabled(!qFuzzyIsNull(area));  //A flat shape has no area to scale from
}

void PropertiesPanel::updateText(const QString& text)
{
	//Same as the canvas label: don't rebuild the text while the user is typing it here, or the cursor jumps
	if (text != m_text->toPlainText())
		m_text->setPlainText(text);
}
