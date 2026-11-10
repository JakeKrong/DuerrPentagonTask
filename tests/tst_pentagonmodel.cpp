//Unit tests for PentagonModel: area, scaling and change signals.

#include "PentagonModel.h"

#include <QLineF>
#include <QSignalSpy>
#include <QTest>
#include <QtMath>

#include <cmath>

namespace
{
	QPointF averageOf(const QPolygonF& points)
	{
		QPointF sum;
		for (const QPointF& p : points)
			sum += p;
		return sum / points.size();
	}
}

class TestPentagonModel : public QObject
{
	Q_OBJECT

private slots:
	//Each private slot is one test case; QTest runs them in order.
	//A slot named X_data supplies rows of input for test X, which then runs once per row.
	void startsAsRegularPentagon();
	void polygonArea_data();
	void polygonArea();
	void setPointMovesOnlyThatPoint();
	void translateMovesEveryPoint();
	void setAreaScalesAboutCentre();
	void setAreaIgnoresImpossibleValues();
	void setFillColorNotifiesWithNewColor();
	void setTextNotifiesWithNewText();
	void settingSameValuesEmitsNothing();
};

void TestPentagonModel::startsAsRegularPentagon()
{
	PentagonModel model;
	const QPolygonF points = model.polygon();
	QCOMPARE(points.size(), qsizetype(PentagonModel::PointCount));

	//Regular: every corner the same distance from the centre, and every side the same length
	const QPointF centre = averageOf(points);
	const double radius = QLineF(centre, points[0]).length();
	const double side = QLineF(points[0], points[1]).length();
	for (int i = 0; i < PentagonModel::PointCount; ++i)
	{
		QCOMPARE(QLineF(centre, points[i]).length(), radius);
		QCOMPARE(QLineF(points[i], points[(i + 1) % PentagonModel::PointCount]).length(), side);
	}

	//A regular pentagon with corners r from the centre has area (5/2) r^2 sin(72 degrees)
	QCOMPARE(model.area(), 2.5 * radius * radius * std::sin(qDegreesToRadians(72.0)));
	QVERIFY(model.fillColor().isValid());
	QVERIFY(!model.text().isEmpty());
}

void TestPentagonModel::polygonArea_data()
{
	QTest::addColumn<QPolygonF>("polygon");
	QTest::addColumn<double>("expected");

	QTest::newRow("square") << QPolygonF{ {0, 0}, {10, 0}, {10, 10}, {0, 10} } << 100.0;
	QTest::newRow("square, other way round") << QPolygonF{ {0, 0}, {0, 10}, {10, 10}, {10, 0} } << 100.0;
	QTest::newRow("right triangle") << QPolygonF{ {0, 0}, {4, 0}, {0, 3} } << 6.0;
	QTest::newRow("concave L shape") << QPolygonF{ {0, 0}, {20, 0}, {20, 10}, {10, 10}, {10, 20}, {0, 20} } << 300.0;
	QTest::newRow("points in a line") << QPolygonF{ {0, 0}, {5, 5}, {10, 10} } << 0.0;
	//Known limitation: the two halves of a bow tie wind in opposite directions and cancel out (filled area is 50)
	QTest::newRow("self-intersecting bow tie") << QPolygonF{ {0, 0}, {10, 10}, {10, 0}, {0, 10} } << 0.0;
}

void TestPentagonModel::polygonArea()
{
	QFETCH(QPolygonF, polygon);
	QFETCH(double, expected);

	QCOMPARE(PentagonModel::polygonArea(polygon), expected);
}

void TestPentagonModel::setPointMovesOnlyThatPoint()
{
	PentagonModel model;
	const QPolygonF before = model.polygon();
	QSignalSpy spy(&model, &PentagonModel::pointsChanged);

	model.setPoint(2, QPointF(10, 20));

	QCOMPARE(spy.count(), 1);
	QCOMPARE(model.point(2), QPointF(10, 20));
	for (int i : { 0, 1, 3, 4 })
		QCOMPARE(model.point(i), before[i]);
}

void TestPentagonModel::translateMovesEveryPoint()
{
	PentagonModel model;
	const QPolygonF before = model.polygon();
	const double areaBefore = model.area();
	QSignalSpy spy(&model, &PentagonModel::pointsChanged);

	model.translate(QPointF(30, -15));

	QCOMPARE(spy.count(), 1);  //One signal for the whole move, not one per point
	QCOMPARE(model.polygon(), before.translated(30, -15));
	QCOMPARE(model.area(), areaBefore);
}

void TestPentagonModel::setAreaScalesAboutCentre()
{
	PentagonModel model;
	model.translate(QPointF(50, 80));  //Away from the origin, so a bug that scales about (0,0) would show
	const QPolygonF before = model.polygon();
	const QPointF centre = before.boundingRect().center();
	const double areaBefore = model.area();
	QSignalSpy spy(&model, &PentagonModel::pointsChanged);

	model.setArea(areaBefore * 4);  //4x the area means 2x the size

	QCOMPARE(spy.count(), 1);
	QCOMPARE(model.area(), areaBefore * 4);
	QCOMPARE(model.polygon().boundingRect().center(), centre);
	for (int i = 0; i < PentagonModel::PointCount; ++i)
		QCOMPARE(QLineF(centre, model.point(i)).length(), 2 * QLineF(centre, before[i]).length());
}

void TestPentagonModel::setAreaIgnoresImpossibleValues()
{
	PentagonModel model;
	const QPolygonF before = model.polygon();
	QSignalSpy spy(&model, &PentagonModel::pointsChanged);

	model.setArea(0.0);
	model.setArea(-100.0);

	QCOMPARE(spy.count(), 0);
	QCOMPARE(model.polygon(), before);

	//A flat shape (all points in a line) has no area to scale from
	for (int i = 0; i < PentagonModel::PointCount; ++i)
		model.setPoint(i, QPointF(i * 10.0, 0.0));
	spy.clear();

	model.setArea(500.0);

	QCOMPARE(spy.count(), 0);
	QCOMPARE(model.area(), 0.0);
}

void TestPentagonModel::setFillColorNotifiesWithNewColor()
{
	PentagonModel model;
	QSignalSpy spy(&model, &PentagonModel::fillColorChanged);

	model.setFillColor(Qt::red);
	model.setFillColor(QColor());  //Invalid, like a cancelled QColorDialog: ignored

	QCOMPARE(spy.count(), 1);
	QCOMPARE(spy.at(0).at(0).value<QColor>(), QColor(Qt::red));  //First emission, first argument
	QCOMPARE(model.fillColor(), QColor(Qt::red));
}

void TestPentagonModel::setTextNotifiesWithNewText()
{
	PentagonModel model;
	QSignalSpy spy(&model, &PentagonModel::textChanged);

	model.setText(QStringLiteral("Hello\nworld"));

	QCOMPARE(spy.count(), 1);
	QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("Hello\nworld"));
	QCOMPARE(model.text(), QStringLiteral("Hello\nworld"));
}

//The guard that stops the canvas and the panel from updating each other in a loop
void TestPentagonModel::settingSameValuesEmitsNothing()
{
	PentagonModel model;
	QSignalSpy pointsSpy(&model, &PentagonModel::pointsChanged);
	QSignalSpy colorSpy(&model, &PentagonModel::fillColorChanged);
	QSignalSpy textSpy(&model, &PentagonModel::textChanged);

	model.setPoint(0, model.point(0));
	model.translate(QPointF(0, 0));
	model.setArea(model.area());
	model.setFillColor(model.fillColor());
	model.setText(model.text());

	QCOMPARE(pointsSpy.count(), 0);
	QCOMPARE(colorSpy.count(), 0);
	QCOMPARE(textSpy.count(), 0);
}

//The model needs no widgets or windows, so a QCoreApplication is enough (no platform plugin needed)
QTEST_GUILESS_MAIN(TestPentagonModel)

//The Q_OBJECT class above is declared in this .cpp, so its moc output has to be included here
#include "tst_pentagonmodel.moc"
