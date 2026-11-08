// DurrPentagonTask.cpp : Defines the entry point for the application.
//

#include "DurrPentagonTask.h"

#include <QApplication>
#include <QLabel>

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);

	QLabel label(QStringLiteral("Hello from Qt %1").arg(QString::fromLatin1(qVersion())));
	label.setAlignment(Qt::AlignCenter);
	label.resize(320, 120);
	label.show();

	return app.exec();
}
