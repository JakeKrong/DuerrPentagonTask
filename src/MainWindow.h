// MainWindow.h : top-level window, with the canvas in the centre and the properties list docked on the right.

#pragma once

#include <QMainWindow>

class PentagonModel;
class QGraphicsScene;
class QGraphicsView;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget* parent = nullptr);

private:
	PentagonModel* m_model = nullptr;
	QGraphicsScene* m_scene = nullptr;
	QGraphicsView* m_view = nullptr;
};
