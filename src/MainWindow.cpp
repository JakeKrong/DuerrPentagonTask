// MainWindow.cpp : builds the window layout.

#include "MainWindow.h"

#include "PentagonModel.h"
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
	, m_view(new QGraphicsView(m_scene, this))
{
	setWindowTitle(tr("Pentagon Editor"));

	// Canvas: the scene holds the items, the view is the widget that displays (part of) the scene.
	// The scene takes ownership of items added to it and deletes them with itself.
	m_scene->setSceneRect(-400, -300, 800, 600);
	m_view->setRenderHint(QPainter::Antialiasing);
	setCentralWidget(m_view);

	m_scene->addItem(new PolygonItem(m_model));
	for (int i = 0; i < PentagonModel::PointCount; ++i)
		m_scene->addItem(new VertexHandle(m_model, i));

	// Properties list, docked on the right. Not closable, since there is no menu to bring it back.
	auto* dock = new QDockWidget(tr("Properties"), this);
	dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
	dock->setWidget(new PropertiesPanel(m_model, dock));
	addDockWidget(Qt::RightDockWidgetArea, dock);

	resize(1100, 700);
}
