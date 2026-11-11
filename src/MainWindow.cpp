// MainWindow.cpp : builds the window layout.

#include "MainWindow.h"

#include "PentagonModel.h"
#include "canvas/CanvasView.h"
#include "canvas/LabelItem.h"
#include "canvas/PolygonItem.h"
#include "canvas/VertexHandle.h"
#include "properties/PropertiesPanel.h"

#include <QDockWidget>
#include <QGraphicsScene>
#include <QGraphicsView>

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
	, m_model(new PentagonModel(this))
	, m_scene(new QGraphicsScene(this))
	, m_view(new CanvasView(m_scene, this))
{
	setWindowTitle(tr("Pentagon Editor"));

	// Canvas: the scene holds the items, the view is the widget that displays (part of) the scene.
	// Scene takes ownership of items added to it and deletes them with itself.
	// Scene rect is the model's bounds, so the drawing area the view shows is exactly where the shape can go.
	m_scene->setSceneRect(PentagonModel::Bounds);
	setCentralWidget(m_view);

	m_scene->addItem(new PolygonItem(m_model));

	for (int i = 0; i < PentagonModel::PointCount; ++i)
		m_scene->addItem(new VertexHandle(m_model, i));

	m_scene->addItem(new LabelItem(m_model));

	// Properties list, docked on the right. Not closable, since there is no menu to bring it back.
	QDockWidget* dock = new QDockWidget(tr("Properties"), this);
	dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
	dock->setWidget(new PropertiesPanel(m_model, dock));
	addDockWidget(Qt::RightDockWidgetArea, dock);

	resize(1100, 700);
}
