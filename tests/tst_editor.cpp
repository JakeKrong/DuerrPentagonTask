//Editor tests: the real main window, driven with mouse and keyboard events like a user would, checking that the
//canvas, the properties panel and the model always agree. ctest runs it off-screen, so no windows appear.

#include "MainWindow.h"
#include "PentagonModel.h"
#include "canvas/LabelItem.h"
#include "canvas/PolygonItem.h"
#include "canvas/VertexHandle.h"
#include "properties/ColorProperty.h"
#include "properties/PointProperty.h"
#include "properties/PropertiesPanel.h"

#include <QApplication>
#include <QDoubleSpinBox>
#include <QGraphicsView>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScopeGuard>
#include <QScrollBar>
#include <QTest>

#include <cmath>

namespace
{
	//Model, items and panel are kept in sync exactly. Mouse positions are whole pixels on a scaled view,
	//so where a test moves the mouse by some distance, the result can be about a pixel off.
	constexpr double MouseTolerance = 1.5;

	bool near(const QPointF& a, const QPointF& b, double tolerance = 1e-6)
	{
		return QLineF(a, b).length() <= tolerance;
	}

	bool near(const QPolygonF& a, const QPolygonF& b, double tolerance)
	{
		if (a.size() != b.size())
			return false;
		for (qsizetype i = 0; i < a.size(); ++i)
			if (!near(a[i], b[i], tolerance))
				return false;
		return true;
	}

	//Moved, but not reshaped
	bool sameShape(const QPolygonF& now, const QPolygonF& before)
	{
		return near(now, before.translated(now[0] - before[0]), 1e-6);
	}

	bool touchesAnEdge(const QRectF& box)
	{
		const QRectF bounds = PentagonModel::Bounds;
		return qFuzzyCompare(box.left(), bounds.left()) || qFuzzyCompare(box.right(), bounds.right())
		       || qFuzzyCompare(box.top(), bounds.top()) || qFuzzyCompare(box.bottom(), bounds.bottom());
	}
}

class TestEditor : public QObject
{
	Q_OBJECT

private slots:
	//init() and cleanup() run before and after every test, so each test starts with a fresh window
	void init();
	void cleanup();

	void canvasFitsTheWindow();
	void labelTextIsReadableWithADarkTheme();
	void startsConsistent();
	void modelChangesShowEverywhere();

	void draggingTheBodyMovesTheShape();
	void otherDragsDoNothing();
	void draggingAHandleMovesOnePoint();
	void bodyDragStopsAtTheEdgeAndKeepsTheGrabPoint();
	void labelMovesAboveTheShapeAtTheBottom();
	void handleStopsAtTheEdgeAndStaysInSync();

	void labelIsEditedOnTheCanvas();
	void emptyLabelCanStillBeEdited();

	void editingAPointInThePanel();
	void pointBoxesAreLimitedToTheCanvas();
	void editingTheAreaInThePanel();
	void editingTheColorInThePanel();
	void editingTheTextInThePanel();
	void preciseValuesAreNotRoundedByThePanel();
	void areaIsDisabledForAFlatShape();
	void spinBoxStepsSuitTheValues();
	void resetButtonPutsThePolygonBack();

private:
	//Mouse events are sent to the view directly, so that move events carry the held button the way a real drag does
	void mouse(QEvent::Type type, const QPointF& scenePos, Qt::MouseButton button, Qt::MouseButtons buttons);
	void drag(const QPointF& from, const QPointF& by, Qt::MouseButton button = Qt::LeftButton);
	void click(const QPointF& at);
	void doubleClick(const QPointF& at);
	//What a user does in a spin box: select the number, type a new one, press Enter
	void typeInBox(QDoubleSpinBox* box, const QString& text);

	//A panel field, found by the label next to it (QFormLayout makes each label's buddy its field)
	QWidget* field(const QString& labelText) const;
	QDoubleSpinBox* pointBox(int index, const QString& prefix) const;

	bool wholeCanvasVisible() const;
	bool handlesMatchModel() const;
	bool panelShowsPoints() const;
	bool allInSync() const;
	bool labelCentredBelowShape() const;

	MainWindow* m_window = nullptr;
	QGraphicsView* m_view = nullptr;
	PentagonModel* m_model = nullptr;
	PolygonItem* m_polygon = nullptr;
	LabelItem* m_label = nullptr;
	QList<VertexHandle*> m_handles;
	QDoubleSpinBox* m_area = nullptr;
	ColorProperty* m_color = nullptr;
	QLineEdit* m_hex = nullptr;
	QPlainTextEdit* m_text = nullptr;
};

void TestEditor::init()
{
	m_window = new MainWindow;
	m_window->show();
	m_window->activateWindow();
	//Keyboard focus only works in the active window
	QVERIFY(QTest::qWaitForWindowActive(m_window));

	m_view = m_window->findChild<QGraphicsView*>();
	m_model = m_window->findChild<PentagonModel*>();
	QVERIFY(m_view && m_model);
	m_view->setFocus();

	m_handles.clear();
	for (QGraphicsItem* item : m_view->scene()->items())
	{
		if (auto* polygon = dynamic_cast<PolygonItem*>(item))
			m_polygon = polygon;
		else if (auto* label = dynamic_cast<LabelItem*>(item))
			m_label = label;
		else if (auto* handle = dynamic_cast<VertexHandle*>(item))
			m_handles << handle;
	}
	QVERIFY(m_polygon && m_label);
	QCOMPARE(m_handles.size(), qsizetype(PentagonModel::PointCount));

	m_area = qobject_cast<QDoubleSpinBox*>(field(QStringLiteral("Area (px") + QChar(0x00B2) + QChar(')')));
	m_color = qobject_cast<ColorProperty*>(field(QStringLiteral("Fill colour")));
	m_hex = m_color ? m_color->findChild<QLineEdit*>() : nullptr;
	m_text = qobject_cast<QPlainTextEdit*>(field(QStringLiteral("Text")));
	QVERIFY(m_area && m_color && m_hex && m_text);
	for (int i = 0; i < PentagonModel::PointCount; ++i)
		QVERIFY(pointBox(i, QStringLiteral("X ")) && pointBox(i, QStringLiteral("Y ")));
}

void TestEditor::cleanup()
{
	delete m_window;
	m_window = nullptr;
}

//--- The canvas view ---

void TestEditor::canvasFitsTheWindow()
{
	QVERIFY(wholeCanvasVisible());

	const int widthBefore = m_view->width();
	m_window->resize(700, 480);
	QTRY_VERIFY(m_view->width() < widthBefore);
	QVERIFY2(wholeCanvasVisible(), "after shrinking the window");

	m_window->resize(1400, 900);
	QTRY_VERIFY(m_view->width() > widthBefore);
	QVERIFY2(wholeCanvasVisible(), "after growing the window");
}

void TestEditor::labelTextIsReadableWithADarkTheme()
{
	//A dark Windows theme gives Qt a palette with white text, but the canvas is always white. Text colours taken
	//from the palette would vanish. (The window from init() was made with the normal palette, so make another.)
	const QPalette original = QApplication::palette();
	const auto restorePalette = qScopeGuard([&] { QApplication::setPalette(original); });
	QPalette dark = original;
	dark.setColor(QPalette::Text, Qt::white);
	dark.setColor(QPalette::WindowText, Qt::white);
	QApplication::setPalette(dark);

	MainWindow window;
	LabelItem* label = nullptr;
	for (QGraphicsItem* item : window.findChild<QGraphicsView*>()->scene()->items())
		if (auto* l = dynamic_cast<LabelItem*>(item))
			label = l;
	QVERIFY(label);
	QCOMPARE(label->defaultTextColor(), QColor(Qt::black));
}

void TestEditor::startsConsistent()
{
	QVERIFY(panelShowsPoints());
	QCOMPARE(m_color->color(), m_model->fillColor());
	QCOMPARE(m_hex->text(), m_model->fillColor().name());
	QCOMPARE(m_text->toPlainText(), m_model->text());
	QCOMPARE(m_label->toPlainText(), m_model->text());
	QVERIFY(handlesMatchModel());
	QCOMPARE(m_polygon->polygon(), m_model->polygon());
	QVERIFY(labelCentredBelowShape());
	QCOMPARE(m_label->textInteractionFlags(), Qt::NoTextInteraction);

	//Showing the model mustn't change it (e.g. by a spin box sending its rounded value back)
	QCOMPARE(m_model->polygon(), PentagonModel().polygon());
}

void TestEditor::modelChangesShowEverywhere()
{
	m_model->setPoint(2, QPointF(200, 150));
	QVERIFY(allInSync());
	QVERIFY(labelCentredBelowShape());

	m_model->setFillColor(QColor(10, 20, 30));
	QCOMPARE(m_polygon->brush().color(), QColor(10, 20, 30));
	QCOMPARE(m_color->color(), QColor(10, 20, 30));
	QCOMPARE(m_hex->text(), QStringLiteral("#0a141e"));

	m_model->setText(QStringLiteral("From the model"));
	QCOMPARE(m_label->toPlainText(), QStringLiteral("From the model"));
	QCOMPARE(m_text->toPlainText(), QStringLiteral("From the model"));
	QVERIFY2(labelCentredBelowShape(), "the label re-centres for the new text width");
}

//--- Dragging on the canvas ---

void TestEditor::draggingTheBodyMovesTheShape()
{
	const QPolygonF before = m_model->polygon();

	drag(before.boundingRect().center(), QPointF(100, 50));

	QVERIFY(near(m_model->polygon(), before.translated(100, 50), MouseTolerance));
	QVERIFY(sameShape(m_model->polygon(), before));
	QCOMPARE(m_polygon->pos(), QPointF(0, 0));  //The points moved, not the item
	QVERIFY(allInSync());
	QVERIFY(labelCentredBelowShape());
}

void TestEditor::otherDragsDoNothing()
{
	const QPolygonF before = m_model->polygon();

	drag(before.boundingRect().center(), QPointF(-60, 40), Qt::RightButton);
	QCOMPARE(m_model->polygon(), before);

	drag(PentagonModel::Bounds.topLeft() + QPointF(15, 15), QPointF(50, 50));  //Empty canvas
	QCOMPARE(m_model->polygon(), before);
}

void TestEditor::draggingAHandleMovesOnePoint()
{
	const QPolygonF before = m_model->polygon();

	drag(before[1], QPointF(20, 25));

	QVERIFY(near(m_model->point(1), before[1] + QPointF(20, 25), MouseTolerance));
	for (int i : { 0, 2, 3, 4 })
		QCOMPARE(m_model->point(i), before[i]);
	QVERIFY(allInSync());
	QVERIFY(labelCentredBelowShape());
}

void TestEditor::bodyDragStopsAtTheEdgeAndKeepsTheGrabPoint()
{
	const QPolygonF before = m_model->polygon();
	const QPointF grab = before.boundingRect().center();

	mouse(QEvent::MouseButtonPress, grab, Qt::LeftButton, Qt::LeftButton);
	mouse(QEvent::MouseMove, grab + QPointF(2000, 0), Qt::NoButton, Qt::LeftButton);

	QCOMPARE(m_model->polygon().boundingRect().right(), PentagonModel::Bounds.right());
	QVERIFY(sameShape(m_model->polygon(), before));
	QVERIFY(allInSync());
	QVERIFY(PentagonModel::Bounds.contains(m_label->sceneBoundingRect()));

	//Back to where the drag began: the shape is back exactly where it started
	mouse(QEvent::MouseMove, grab, Qt::NoButton, Qt::LeftButton);
	mouse(QEvent::MouseButtonRelease, grab, Qt::LeftButton, Qt::NoButton);
	QVERIFY(near(m_model->polygon(), before, 1e-6));
}

void TestEditor::labelMovesAboveTheShapeAtTheBottom()
{
	drag(m_model->polygon().boundingRect().center(), QPointF(0, 2000));

	const QRectF shape = m_model->polygon().boundingRect();
	const QRectF label = m_label->sceneBoundingRect();
	QCOMPARE(shape.bottom(), PentagonModel::Bounds.bottom());
	QVERIFY(label.bottom() <= shape.top());
	QVERIFY(PentagonModel::Bounds.contains(label));
}

void TestEditor::handleStopsAtTheEdgeAndStaysInSync()
{
	drag(m_model->point(0), QPointF(0, -2000));
	QCOMPARE(m_model->point(0).y(), PentagonModel::Bounds.top());
	QVERIFY(allInSync());

	//Further out while already on the edge: the model doesn't change, so the handle must stop itself
	drag(m_model->point(0), QPointF(30, -100));
	QCOMPARE(m_model->point(0).y(), PentagonModel::Bounds.top());
	QVERIFY(allInSync());
}

//--- Editing the label on the canvas ---

void TestEditor::labelIsEditedOnTheCanvas()
{
	const QPointF label = m_label->sceneBoundingRect().center();

	click(label);
	QCOMPARE(m_label->textInteractionFlags(), Qt::NoTextInteraction);

	doubleClick(label);
	QCOMPARE(m_label->textInteractionFlags(), Qt::TextEditorInteraction);
	QVERIFY(m_label->hasFocus());

	QTest::keyClick(m_view, Qt::Key_A, Qt::ControlModifier);
	QTest::keyClicks(m_view, QStringLiteral("Canvas"));
	//In order: if the label's text were reset after each keystroke, the cursor would jump back and give "savnaC"
	QCOMPARE(m_model->text(), QStringLiteral("Canvas"));

	QTest::keyClick(m_view, Qt::Key_Return);
	QTest::keyClicks(m_view, QStringLiteral("Two lines"));
	QCOMPARE(m_model->text(), QStringLiteral("Canvas\nTwo lines"));
	QCOMPARE(m_text->toPlainText(), m_model->text());
	QVERIFY(labelCentredBelowShape());

	QTest::keyClick(m_view, Qt::Key_Escape);
	QCOMPARE(m_label->textInteractionFlags(), Qt::NoTextInteraction);
	QVERIFY(!m_label->hasFocus());
	QCOMPARE(m_model->text(), QStringLiteral("Canvas\nTwo lines"));

	doubleClick(m_label->sceneBoundingRect().center());
	QCOMPARE(m_label->textInteractionFlags(), Qt::TextEditorInteraction);
	click(PentagonModel::Bounds.topLeft() + QPointF(15, 15));
	QCOMPARE(m_label->textInteractionFlags(), Qt::NoTextInteraction);
}

void TestEditor::emptyLabelCanStillBeEdited()
{
	m_model->setText(QString());
	const QRectF empty = m_label->sceneBoundingRect();
	QVERIFY2(empty.width() > 100, "an empty label keeps a clickable width");
	QVERIFY(labelCentredBelowShape());

	doubleClick(empty.center());
	QTest::keyClicks(m_view, QStringLiteral("Back"));
	QTest::keyClick(m_view, Qt::Key_Escape);

	QCOMPARE(m_model->text(), QStringLiteral("Back"));
	QVERIFY(m_label->sceneBoundingRect().width() < empty.width());
}

//--- Editing in the properties panel ---

void TestEditor::editingAPointInThePanel()
{
	const QPointF before = m_model->point(0);

	typeInBox(pointBox(0, QStringLiteral("X ")), QStringLiteral("123.5"));

	QCOMPARE(m_model->point(0).x(), 123.5);
	QVERIFY2(m_model->point(0).y() == before.y(), "Y is left exactly as it was, not rounded to the box's 2 decimals");
	QVERIFY(allInSync());
	QVERIFY(labelCentredBelowShape());
}

void TestEditor::pointBoxesAreLimitedToTheCanvas()
{
	const QRectF bounds = PentagonModel::Bounds;
	QCOMPARE(pointBox(0, QStringLiteral("X "))->minimum(), bounds.left());
	QCOMPARE(pointBox(0, QStringLiteral("X "))->maximum(), bounds.right());
	QCOMPARE(pointBox(0, QStringLiteral("Y "))->minimum(), bounds.top());
	QCOMPARE(pointBox(0, QStringLiteral("Y "))->maximum(), bounds.bottom());

	//The box refuses a keystroke that would take it past the edge, so "999" stops at "99"
	typeInBox(pointBox(2, QStringLiteral("X ")), QStringLiteral("999"));
	QCOMPARE(m_model->point(2).x(), 99.0);

	typeInBox(pointBox(2, QStringLiteral("X ")), QStringLiteral("400"));
	QCOMPARE(m_model->point(2).x(), bounds.right());
	QVERIFY(allInSync());
}

void TestEditor::editingTheAreaInThePanel()
{
	const double target = std::round(m_model->area() * 1.5 * 100) / 100;
	typeInBox(m_area, QString::number(target, 'f', 2));
	QVERIFY2(std::abs(m_model->area() - target) < 1e-6, qPrintable(QString("area %1, wanted %2").arg(m_model->area()).arg(target)));
	QVERIFY(allInSync());

	//An area the model can't use is ignored, and the box goes back to the real area
	const QPolygonF before = m_model->polygon();
	typeInBox(m_area, QStringLiteral("0"));
	QCOMPARE(m_model->polygon(), before);
	QVERIFY(panelShowsPoints());

	//Too big for the canvas: it grows until it touches an edge, and the box shows the area it reached
	typeInBox(m_area, QStringLiteral("400000"));
	QVERIFY(PentagonModel::Bounds.contains(m_model->polygon().boundingRect()));
	QVERIFY(touchesAnEdge(m_model->polygon().boundingRect()));
	QVERIFY(allInSync());
}

void TestEditor::editingTheColorInThePanel()
{
	m_hex->selectAll();
	QTest::keyClicks(m_hex, QStringLiteral("#00FF00"));
	QCOMPARE(m_model->fillColor(), QColor(0, 255, 0));
	QCOMPARE(m_polygon->brush().color(), QColor(0, 255, 0));
	QCOMPARE(m_hex->text(), QStringLiteral("#00FF00"));  //Not rewritten under the cursor while typing

	m_hex->selectAll();
	QTest::keyClicks(m_hex, QStringLiteral("#12"));
	QCOMPARE(m_model->fillColor(), QColor(0, 255, 0));  //Half-typed: not applied
	QTest::keyClick(m_hex, Qt::Key_Return);
	QCOMPARE(m_hex->text(), QStringLiteral("#00ff00"));  //...and Enter puts the current colour back
}

void TestEditor::editingTheTextInThePanel()
{
	QTest::keyClick(m_text, Qt::Key_A, Qt::ControlModifier);
	QTest::keyClicks(m_text, QStringLiteral("Panel text"));

	QCOMPARE(m_model->text(), QStringLiteral("Panel text"));
	QCOMPARE(m_label->toPlainText(), QStringLiteral("Panel text"));
	QVERIFY(labelCentredBelowShape());
}

void TestEditor::preciseValuesAreNotRoundedByThePanel()
{
	const QPointF precise(1.23456789, -2.3456789);
	m_model->setPoint(0, precise);

	QVERIFY(m_model->point(0).x() == precise.x() && m_model->point(0).y() == precise.y());
	QCOMPARE(pointBox(0, QStringLiteral("X "))->value(), 1.23);  //Only the display is rounded
}

void TestEditor::areaIsDisabledForAFlatShape()
{
	for (int i = 0; i < PentagonModel::PointCount; ++i)
		m_model->setPoint(i, QPointF(i * 10.0, 0.0));
	QVERIFY(!m_area->isEnabled());

	m_model->setPoint(2, QPointF(20, 30));
	QVERIFY(m_area->isEnabled());
}

void TestEditor::spinBoxStepsSuitTheValues()
{
	QCOMPARE(pointBox(0, QStringLiteral("X "))->singleStep(), 5.0);
	QCOMPARE(m_area->singleStep(), 1000.0);

	//The Up arrow key steps a point by 5 (from the 2-decimal value the box shows), leaving the other coordinate alone
	const QPointF before = m_model->point(1);
	QTest::keyClick(pointBox(1, QStringLiteral("X ")), Qt::Key_Up);
	QVERIFY2(std::abs(m_model->point(1).x() - (before.x() + 5)) < 0.01, qPrintable(QString("x went from %1 to %2").arg(before.x()).arg(m_model->point(1).x())));
	QVERIFY(m_model->point(1).y() == before.y());

	const double area = m_model->area();
	QTest::keyClick(m_area, Qt::Key_Up);
	QVERIFY2(std::abs(m_model->area() - (area + 1000)) < 0.01, qPrintable(QString("area went from %1 to %2").arg(area).arg(m_model->area())));
	QVERIFY(allInSync());
}

void TestEditor::resetButtonPutsThePolygonBack()
{
	QPushButton* reset = nullptr;
	for (QPushButton* button : m_window->findChildren<QPushButton*>())
		if (button->text() == QStringLiteral("Reset polygon"))
			reset = button;
	QVERIFY(reset);

	//Change everything, the ways a user would
	drag(m_model->polygon().boundingRect().center(), QPointF(120, 60));
	drag(m_model->point(3), QPointF(-40, 30));
	typeInBox(m_area, QStringLiteral("60000"));
	m_hex->selectAll();
	QTest::keyClicks(m_hex, QStringLiteral("#ff0000"));
	m_text->selectAll();
	QTest::keyClicks(m_text, QStringLiteral("My label"));
	QVERIFY(m_model->polygon() != PentagonModel().polygon());

	QTest::mouseClick(reset, Qt::LeftButton);

	const PentagonModel original;
	QCOMPARE(m_model->polygon(), original.polygon());
	QCOMPARE(m_model->fillColor(), original.fillColor());
	QCOMPARE(m_model->text(), QStringLiteral("My label"));  //The label text is kept
	QVERIFY(allInSync());
	QCOMPARE(m_polygon->brush().color(), original.fillColor());
	QCOMPARE(m_hex->text(), original.fillColor().name());
	QVERIFY(labelCentredBelowShape());
}

//--- Helpers ---

void TestEditor::mouse(QEvent::Type type, const QPointF& scenePos, Qt::MouseButton button, Qt::MouseButtons buttons)
{
	const QPoint at = m_view->mapFromScene(scenePos);
	QMouseEvent event(type, at, m_view->viewport()->mapToGlobal(at), button, buttons, Qt::NoModifier);
	QApplication::sendEvent(m_view->viewport(), &event);
}

void TestEditor::drag(const QPointF& from, const QPointF& by, Qt::MouseButton button)
{
	mouse(QEvent::MouseButtonPress, from, button, button);
	mouse(QEvent::MouseMove, from + by / 2, Qt::NoButton, button);
	mouse(QEvent::MouseMove, from + by, Qt::NoButton, button);
	mouse(QEvent::MouseButtonRelease, from + by, button, Qt::NoButton);
}

void TestEditor::click(const QPointF& at)
{
	mouse(QEvent::MouseButtonPress, at, Qt::LeftButton, Qt::LeftButton);
	mouse(QEvent::MouseButtonRelease, at, Qt::LeftButton, Qt::NoButton);
}

void TestEditor::doubleClick(const QPointF& at)
{
	//The order Qt delivers a real double-click in
	click(at);
	mouse(QEvent::MouseButtonDblClick, at, Qt::LeftButton, Qt::LeftButton);
	mouse(QEvent::MouseButtonRelease, at, Qt::LeftButton, Qt::NoButton);
}

void TestEditor::typeInBox(QDoubleSpinBox* box, const QString& text)
{
	box->selectAll();
	QTest::keyClicks(box, text);
	QTest::keyClick(box, Qt::Key_Return);
}

QWidget* TestEditor::field(const QString& labelText) const
{
	for (QLabel* label : m_window->findChildren<QLabel*>())
		if (label->text() == labelText)
			return label->buddy();
	return nullptr;
}

QDoubleSpinBox* TestEditor::pointBox(int index, const QString& prefix) const
{
	auto* point = qobject_cast<PointProperty*>(field(QStringLiteral("Point %1").arg(index + 1)));
	if (!point)
		return nullptr;
	for (QDoubleSpinBox* box : point->findChildren<QDoubleSpinBox*>())
		if (box->prefix() == prefix)
			return box;
	return nullptr;
}

bool TestEditor::wholeCanvasVisible() const
{
	const QRectF visible = m_view->mapToScene(m_view->viewport()->rect()).boundingRect();
	return visible.adjusted(-1, -1, 1, 1).contains(PentagonModel::Bounds)
	       && !m_view->horizontalScrollBar()->isVisible() && !m_view->verticalScrollBar()->isVisible();
}

bool TestEditor::handlesMatchModel() const
{
	//Each point has a handle exactly on it
	for (int i = 0; i < PentagonModel::PointCount; ++i)
	{
		bool found = false;
		for (const VertexHandle* handle : m_handles)
			found = found || near(handle->pos(), m_model->point(i));
		if (!found)
			return false;
	}
	return true;
}

bool TestEditor::panelShowsPoints() const
{
	//The boxes show 2 decimals
	for (int i = 0; i < PentagonModel::PointCount; ++i)
	{
		if (std::abs(pointBox(i, QStringLiteral("X "))->value() - m_model->point(i).x()) > 0.005
		    || std::abs(pointBox(i, QStringLiteral("Y "))->value() - m_model->point(i).y()) > 0.005)
			return false;
	}
	return std::abs(m_area->value() - m_model->area()) <= 0.005;
}

bool TestEditor::allInSync() const
{
	return handlesMatchModel() && m_polygon->polygon() == m_model->polygon() && panelShowsPoints();
}

bool TestEditor::labelCentredBelowShape() const
{
	const QRectF shape = m_model->polygon().boundingRect();
	const QRectF label = m_label->sceneBoundingRect();
	return std::abs(label.center().x() - shape.center().x()) < 0.5 && label.top() > shape.bottom();
}

//Widgets need a full QApplication
QTEST_MAIN(TestEditor)

//The Q_OBJECT class above is declared in this .cpp, so its moc output has to be included here
#include "tst_editor.moc"
