#include "properties/PropertiesPanel.h"

#include "PentagonModel.h"

PropertiesPanel::PropertiesPanel(PentagonModel* model, QWidget* parent)
	: QWidget(parent)
	, m_model(model)
{
}
