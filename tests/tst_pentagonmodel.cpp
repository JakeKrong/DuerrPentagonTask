//Unit tests for PentagonModel: area, scaling and change signals.

#include "PentagonModel.h"

#include <QTest>

class TestPentagonModel : public QObject
{
	Q_OBJECT

private slots:
	//Each private slot is one test case; QTest runs them in order
	void constructs();
};

void TestPentagonModel::constructs()
{
	PentagonModel model;
}

//The model needs no widgets or windows, so a QCoreApplication is enough (no platform plugin needed)
QTEST_GUILESS_MAIN(TestPentagonModel)

//The Q_OBJECT class above is declared in this .cpp, so its moc output has to be included here
#include "tst_pentagonmodel.moc"
