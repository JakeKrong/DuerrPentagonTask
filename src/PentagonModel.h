//Single source of truth for the shape: the polygon's 5 points, its fill colour and the label text.
//The canvas and the properties panel both read from it, write through its setters and update on its signals;
//they never talk to each other directly.

#pragma once

#include <QObject>

class PentagonModel : public QObject
{
	Q_OBJECT

public:
	explicit PentagonModel(QObject* parent = nullptr);
};
