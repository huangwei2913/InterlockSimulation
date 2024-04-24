#include "Trackview.h"
#include "SceneEclipse.h"
#include "SceneText.h"
#include "BlinkingSquare.h"
#include "DataAccessLayer.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector2D>
#include "Configurations.h"
#include <QtConcurrent/qtconcurrentmap.h>
TrackView::TrackView(QGraphicsScene* scene, QWidget* parent)
	: QGraphicsView(parent), scene(scene)
{
	setScene(scene);
	pathLines.clear();
	InitialSignalStatus();
}
void TrackView::addEclipsesToLines(const QString& lineName, bool above)
{
	QPair<QPointF, QPointF> positions = linePositions.value(lineName);
	QPointF start = positions.first;
	QPointF end = positions.second;
	qreal offset = 20;  // 这是你想要的距离，你可以根据需要修改它
	// 如果 above 为 true，则在线段上方添加 SceneEclipse 对象，否则在下方添加
	qreal y = above ? start.y() - offset : start.y() + offset;
	QPointF testloc = mapToScene(start.x(), y);
	SceneEclipse* startEclipse = new SceneEclipse(testloc.x(), testloc.y(), scene);
	QPointF testloc1 = mapToScene(end.x(), y);
	SceneEclipse* endEclipse = new SceneEclipse(testloc1.x(), testloc1.y(), scene);
	scene->addItem(startEclipse);
	scene->addItem(endEclipse);
}
void TrackView::addEclipsesToLineEnds(const QString& lineName, bool startOrEnd, bool leftOrRight)
{
	QPair<QPointF, QPointF> positions = linePositions.value(lineName);
	QPointF start = positions.first;
	QPointF end = positions.second;
	qreal offset = 20;  // 这是你想要的距离，你可以根据需要修改它
	// 如果 startOrEnd 为 true，则在线段开头添加 SceneEclipse 对象，否则在结尾添加
	QPointF positionInView = startOrEnd ? start : end;
	// 如果 leftOrRight 为 true，则左对齐，否则右对齐
	qreal x = leftOrRight ? positionInView.x() : positionInView.x() + offset;
	QPointF positionInScene1 = mapToScene(x, positionInView.y());
	SceneEclipse* eclipse1 = new SceneEclipse(positionInScene1.x(), positionInScene1.y(), scene);
	scene->addItem(eclipse1);
	QPointF positionInScene2 = mapToScene(x + offset, positionInView.y());
	SceneEclipse* eclipse2 = new SceneEclipse(positionInScene2.x(), positionInScene2.y(), scene);
	scene->addItem(eclipse2);
}
//下一步是增加信号灯
void TrackView::drawTrackSection()
{
	int t = height() / 10;  // 将视图的高度平均分为10等份
	int m = width() / 12;  // 将视图的宽度平均分为12等份
	// 在"3-11DG"线段前添加一个长度为10的灰色线段
	int downlinkHeightLoc = 4 * t;  //下行线陆在视图中开始的高度位置
	int uplinkHeightLoc = 6 * t;   //上行线陆在视图中开始的高度位置
	int isolationLength = 10;
	QPointF dividerStart1 = mapToScene(0.7 * m - isolationLength, downlinkHeightLoc);
	QPointF dividerEnd1 = mapToScene(0.7 * m, downlinkHeightLoc);
	SceneLine* divider1 = new SceneLine(dividerStart1, dividerEnd1, "", false);
	divider1->setColor(Qt::gray);  // 设置分割线的颜色为灰色
	scene->addItem(divider1);
	QPointF start1 = mapToScene(0.7 * m, downlinkHeightLoc);  // 将视图坐标转换为场景坐标
	QPointF end1 = mapToScene(3.2 * m, downlinkHeightLoc);  // 将视图坐标转换为场景坐标
	SceneLine* line1 = new SceneLine(start1, end1, "3-11DG", false);
	line1->setTextVisible(false);
	linePositions.insert("3-11DG", qMakePair(QPointF(0.7 * m, downlinkHeightLoc), QPointF(3.2 * m, downlinkHeightLoc)));
	m_lineMap.insert(line1->getTracksectionName(), line1);  // 将 SceneLine 对象添加到 QMap 中
	dal.insertViewTrackSectionData(line1->getTracksectionName(), line1->getColor());
	scene->addItem(line1);
	// 在"3-11DG"线段后添加一个长度为10的灰色线段
	QPointF dividerStart2 = end1;
	QPointF dividerEnd2 = mapToScene(3.2 * m + isolationLength, downlinkHeightLoc);
	SceneLine* divider2 = new SceneLine(dividerStart2, dividerEnd2, "", false);
	divider2->setColor(Qt::gray);  // 设置分割线的颜色为灰色
	scene->addItem(divider2);
	// 定义线段的名称
	QStringList names = { "IG", "4DG", "X1LQG", "1101G","1103G","1105G","1107G" };
	// 初始化开始位置
	double startPos = 3.2 * m + isolationLength;
	// 遍历名称并添加线段和分割线
	for (int i = 0; i < names.size(); i++) {
		// 添加线段
		QPointF start = mapToScene(startPos, downlinkHeightLoc);
		QPointF end = mapToScene(startPos + 1.199 * m, downlinkHeightLoc);
		SceneLine* line = new SceneLine(start, end, names[i], false);
		if (names[i].contains("DG")) {
			line->setTextVisible(false);
		}
		scene->addItem(line);
		linePositions.insert(names[i], qMakePair(QPointF(startPos, downlinkHeightLoc), QPointF(startPos + 1.199 * m, downlinkHeightLoc)));
		m_lineMap.insert(line->getTracksectionName(), line);  // 将 SceneLine 对象添加到 QMap 中
		dal.insertViewTrackSectionData(line->getTracksectionName(), line->getColor());
		// 在线段后添加一个长度为10的灰色分割线
		QPointF dividerStart = end;
		QPointF dividerEnd = mapToScene(startPos + 1.199 * m + isolationLength, downlinkHeightLoc);
		SceneLine* divider = new SceneLine(dividerStart, dividerEnd, "", false);
		divider->setColor(Qt::gray);  // 设置分割线的颜色为灰色
		scene->addItem(divider);
		// 更新开始位置以便于下一次循环
		startPos += 1.199 * m + isolationLength;
	}
	//上行进路
	QPointF dividerStart11 = mapToScene(0.7 * m - 10, uplinkHeightLoc);
	QPointF dividerEnd11 = mapToScene(0.7 * m, uplinkHeightLoc);
	SceneLine* divider11 = new SceneLine(dividerStart11, dividerEnd11, "", false);
	divider11->setColor(Qt::gray);  // 设置分割线的颜色为灰色
	scene->addItem(divider11);
	QPointF start11 = mapToScene(0.7 * m, uplinkHeightLoc);  // 将视图坐标转换为场景坐标
	QPointF end11 = mapToScene(3.2 * m, uplinkHeightLoc);  // 将视图坐标转换为场景坐标
	SceneLine* line11 = new SceneLine(start11, end11, "1-9DG", false);
	line11->setTextVisible(false);
	scene->addItem(line11);
	linePositions.insert("1-9DG", qMakePair(QPointF(0.7 * m, uplinkHeightLoc), QPointF(3.2 * m, uplinkHeightLoc)));
	m_lineMap.insert(line11->getTracksectionName(), line11);  // 将 SceneLine 对象添加到 QMap 中
	dal.insertViewTrackSectionData(line11->getTracksectionName(), line11->getColor());
	// 在"3-11DG"线段后添加一个长度为10的灰色线段
	QPointF dividerStart22 = end11;
	QPointF dividerEnd22 = mapToScene(3.2 * m + isolationLength, uplinkHeightLoc);
	SceneLine* divider22 = new SceneLine(dividerStart22, dividerEnd22, "", false);
	divider22->setColor(Qt::gray);  // 设置分割线的颜色为灰色
	scene->addItem(divider22);
	// 定义线段的名称
	QStringList namess = { "IIG", "2DG", "1102G","1104G","1106G","1108G","S1LQG" };
	// 初始化开始位置
	double startPoss = 3.2 * m + isolationLength;
	// 遍历名称并添加线段和分割线
	for (int i = 0; i < namess.size(); i++) {
		// 添加线段
		QPointF start = mapToScene(startPoss, uplinkHeightLoc);
		QPointF end = mapToScene(startPoss + 1.199 * m, uplinkHeightLoc);
		SceneLine* line = new SceneLine(start, end, namess[i], false);
		if (namess[i].contains("DG")) {
			line->setTextVisible(false);
		}
		scene->addItem(line);
		linePositions.insert(namess[i], qMakePair(QPointF(startPoss, uplinkHeightLoc), QPointF(startPoss + 1.199 * m, uplinkHeightLoc)));
		m_lineMap.insert(line->getTracksectionName(), line);  // 将 SceneLine 对象添加到 QMap 中
		dal.insertViewTrackSectionData(line->getTracksectionName(), line->getColor());
		// 在线段后添加一个长度为10的灰色分割线
		QPointF dividerStart = end;
		QPointF dividerEnd = mapToScene(startPoss + 1.199 * m + 10, uplinkHeightLoc);
		SceneLine* divider = new SceneLine(dividerStart, dividerEnd, "", false);
		divider->setColor(Qt::gray);  // 设置分割线的颜色为灰色
		scene->addItem(divider);
		// 更新开始位置以便于下一次循环
		startPoss += 1.199 * m + isolationLength;
	}
	QPair<QPointF, QPointF> positions = linePositions.value("IG");
	QPointF startIG = positions.first;
	QPointF endIG = positions.second;
	QPointF start3G = mapToScene(startIG.x(), startIG.y() - 1 * t);
	QPointF end3G = mapToScene(endIG.x(), endIG.y() - 1 * t);
	SceneLine* lineNew = new SceneLine(start3G, end3G, "3G", true);
	scene->addItem(lineNew);
	linePositions.insert("3G", qMakePair(QPointF(startIG.x(), startIG.y() - 1 * t), QPointF(endIG.x(), endIG.y() - 1 * t)));
	m_lineMap.insert(lineNew->getTracksectionName(), lineNew);  // 将 SceneLine 对象添加到 QMap 中
	dal.insertViewTrackSectionData(lineNew->getTracksectionName(), lineNew->getColor());
	auto positionss = linePositions.value("IIG");
	QPointF startIIG = positionss.first;
	QPointF endIIG = positionss.second;
	QPointF start4G = mapToScene(startIIG.x(), startIIG.y() + 1 * t);
	QPointF end4G = mapToScene(endIIG.x(), endIIG.y() + 1 * t);
	SceneLine* lineNew1 = new SceneLine(start4G, end4G, "4G", true);
	scene->addItem(lineNew1);
	linePositions.insert("4G", qMakePair(QPointF(startIIG.x(), startIIG.y() + 1 * t), QPointF(endIIG.x(), endIIG.y() + 1 * t)));
	m_lineMap.insert(lineNew1->getTracksectionName(), lineNew1);  // 将 SceneLine 对象添加到 QMap 中
	dal.insertViewTrackSectionData(lineNew1->getTracksectionName(), lineNew1->getColor());
}
void TrackView::addEclipsesToLine(const QString& lineName, bool startOrEnd, bool leftOrRight, bool above) {
	QPair<QPointF, QPointF> positions = linePositions.value(lineName);
	QPointF start = positions.first;
	QPointF end = positions.second;
	qreal offset = 20;  // 这是你想要的距离，你可以根据需要修改它
	// 如果 startOrEnd 为 true，则在线段开头添加 SceneEclipse 对象，否则在结尾添加
	QPointF positionInView = startOrEnd ? start : end;
	//信号灯左对齐
	QString startSignalId = dal.getStartSignalId(lineName);
	QString endSignalId = dal.getEndSignalId(lineName);
	qDebug() << "开始，结束信号机的ID是：" << startSignalId << endSignalId << "for linename..." << lineName;
	m_eclipseMapLock.lock();
	if (leftOrRight == true && above == true) {
		auto x1 = positionInView.x();
		auto y1 = positionInView.y() - 2 * offset;
		QPointF tmp1 = mapToScene(x1, y1);
		SceneEclipse* eclipse1 = new SceneEclipse(tmp1.x(), tmp1.y(), scene);
		if (startOrEnd == true) {
			eclipse1->setSignalName(startSignalId + "-B1");
			qDebug() << "which signal ID is added..." << startSignalId + "-B1";
			m_eclipseMap.insert(startSignalId + "-B1", eclipse1);
			dal.insertViewSignalTable(startSignalId, positionInView.x(), positionInView.y());
			m_eclipseViewCodinates.insert(startSignalId + "-B1", QPointF(x1, y1));
			scene->addItem(eclipse1);
			dal.updateViewSignalB1(startSignalId, eclipse1->getColorState());
		}
		else {
			eclipse1->setSignalName(endSignalId + "-B1");
			m_eclipseMap.insert(endSignalId + "-B1", eclipse1);
			qDebug() << "which signal ID is added..." << endSignalId + "-B1";
			m_eclipseViewCodinates.insert(endSignalId + "-B1", QPointF(x1, y1));
			//this->signalPositionMap.insert(endSignalId, QPointF(positionInView.x(), positionInView.y()));
			dal.insertViewSignalTable(endSignalId, positionInView.x(), positionInView.y());
			dal.updateViewSignalB1(endSignalId, eclipse1->getColorState());
			scene->addItem(eclipse1);
		}
		auto x2 = positionInView.x() + offset;
		auto y2 = positionInView.y() - 2 * offset;
		QPointF tmp2 = mapToScene(x2, y2);
		SceneEclipse* eclipse2 = new SceneEclipse(tmp2.x(), tmp2.y(), scene);
		if (startOrEnd == true) {
			eclipse2->setSignalName(startSignalId + "-B2");
			m_eclipseMap.insert(startSignalId + "-B2", eclipse1);
			qDebug() << "which signal ID is added..." << startSignalId + "-B2";
			m_eclipseViewCodinates.insert(startSignalId + "-B2", QPointF(x2, y2));
			dal.insertViewSignalTable(startSignalId, positionInView.x(), positionInView.y());
			dal.updateViewSignalB2(startSignalId, eclipse2->getColorState());
			scene->addItem(eclipse2);
		}
		else {
			eclipse2->setSignalName(endSignalId + "-B2");
			m_eclipseMap.insert(endSignalId + "-B2", eclipse1);
			qDebug() << "which signal ID is added..." << endSignalId + "-B2";
			m_eclipseViewCodinates.insert(endSignalId + "-B2", QPointF(x2, y2));
			dal.insertViewSignalTable(endSignalId, positionInView.x(), positionInView.y());
			dal.updateViewSignalB2(endSignalId, eclipse2->getColorState());
			scene->addItem(eclipse2);
		}
		//在画了之后，也更新这个表中的B1和B2两个字段
	}
	if (leftOrRight == true && above == false) {
		auto x1 = positionInView.x();
		auto y1 = positionInView.y() + 1 * offset;
		QPointF tmp1 = mapToScene(x1, y1);
		SceneEclipse* eclipse1 = new SceneEclipse(tmp1.x(), tmp1.y(), scene);
		if (startOrEnd == true) {
			eclipse1->setSignalName(startSignalId + "-B1");
			m_eclipseMap.insert(startSignalId + "-B1", eclipse1);
			qDebug() << "which signal ID is added..." << startSignalId + "-B1";
			m_eclipseViewCodinates.insert(startSignalId + "-B1", QPointF(x1, y1));
			dal.insertViewSignalTable(startSignalId, positionInView.x(), positionInView.y());
			dal.updateViewSignalB1(startSignalId, eclipse1->getColorState());
			scene->addItem(eclipse1);
		}
		else {
			eclipse1->setSignalName(endSignalId + "-B1");
			m_eclipseMap.insert(endSignalId + "-B1", eclipse1);
			qDebug() << "which signal ID is added..." << endSignalId + "-B1";
			dal.insertViewSignalTable(endSignalId, positionInView.x(), positionInView.y());
			m_eclipseViewCodinates.insert(endSignalId + "-B1", QPointF(x1, y1));
			dal.updateViewSignalB1(endSignalId, eclipse1->getColorState());
			scene->addItem(eclipse1);
		}
		auto x2 = positionInView.x() + offset;
		auto y2 = positionInView.y() + 1 * offset;
		QPointF tmp2 = mapToScene(x2, y2);
		SceneEclipse* eclipse2 = new SceneEclipse(tmp2.x(), tmp2.y(), scene);
		if (startOrEnd == true) {
			eclipse2->setSignalName(startSignalId + "-B2");
			m_eclipseMap.insert(startSignalId + "-B2", eclipse2);
			qDebug() << "which signal ID is added..." << startSignalId + "-B2";
			m_eclipseViewCodinates.insert(startSignalId + "-B2", QPointF(x2, y2));
			dal.insertViewSignalTable(startSignalId, positionInView.x(), positionInView.y());
			dal.updateViewSignalB2(startSignalId, eclipse2->getColorState());
			scene->addItem(eclipse2);
		}
		else {
			eclipse2->setSignalName(endSignalId + "-B2");
			m_eclipseMap.insert(endSignalId + "-B2", eclipse2);
			qDebug() << "which signal ID is added..." << endSignalId + "-B2";
			dal.insertViewSignalTable(endSignalId, positionInView.x(), positionInView.y());
			m_eclipseViewCodinates.insert(endSignalId + "-B2", QPointF(x2, y2));
			scene->addItem(eclipse2);
			dal.updateViewSignalB2(endSignalId, eclipse2->getColorState());
		}
	}
	if (leftOrRight == false && above == true) {
		auto x1 = positionInView.x() - offset;
		auto y1 = positionInView.y() - 2 * offset;
		QPointF tmp1 = mapToScene(x1, y1);
		SceneEclipse* eclipse1 = new SceneEclipse(tmp1.x(), tmp1.y(), scene);
		if (startOrEnd == true) {
			eclipse1->setSignalName(startSignalId + "-B1");
			m_eclipseMap.insert(startSignalId + "-B1", eclipse1);
			dal.insertViewSignalTable(startSignalId, positionInView.x(), positionInView.y());
			dal.updateViewSignalB1(startSignalId, eclipse1->getColorState());
			scene->addItem(eclipse1);
			qDebug() << "which signal ID is added..." << startSignalId + "-B1";
			m_eclipseViewCodinates.insert(startSignalId + "-B1", QPointF(x1, y1));
		}
		else {
			eclipse1->setSignalName(endSignalId + "-B1");
			m_eclipseMap.insert(endSignalId + "-B1", eclipse1);
			//this->signalPositionMap.insert(endSignalId, QPointF(positionInView.x(), positionInView.y()));
			dal.insertViewSignalTable(endSignalId, positionInView.x(), positionInView.y());
			dal.updateViewSignalB1(endSignalId, eclipse1->getColorState());
			scene->addItem(eclipse1);
			qDebug() << "which signal ID is added..." << endSignalId + "-B1";
			m_eclipseViewCodinates.insert(endSignalId + "-B1", QPointF(x1, y1));
		}
		auto x2 = positionInView.x() - 2 * offset;
		auto y2 = positionInView.y() - 2 * offset;
		QPointF tmp2 = mapToScene(x2, y2);
		SceneEclipse* eclipse2 = new SceneEclipse(tmp2.x(), tmp2.y(), scene);
		if (startOrEnd == true) {
			eclipse2->setSignalName(startSignalId + "-B2");
			m_eclipseMap.insert(startSignalId + "-B2", eclipse2);
			//this->signalPositionMap.insert(startSignalId, QPointF(positionInView.x(), positionInView.y()));
			dal.insertViewSignalTable(startSignalId, positionInView.x(), positionInView.y());
			dal.updateViewSignalB2(startSignalId, eclipse2->getColorState());
			scene->addItem(eclipse2);
			qDebug() << "which signal ID is added..." << startSignalId + "-B2";
			m_eclipseViewCodinates.insert(startSignalId + "-B2", QPointF(x2, y2));
		}
		else {
			eclipse2->setSignalName(endSignalId + "-B2");
			m_eclipseMap.insert(endSignalId + "-B2", eclipse2);
			dal.insertViewSignalTable(endSignalId, positionInView.x(), positionInView.y());
			dal.updateViewSignalB2(endSignalId, eclipse2->getColorState());
			scene->addItem(eclipse2);
			qDebug() << "which signal ID is added..." << endSignalId + "-B2";
			m_eclipseViewCodinates.insert(endSignalId + "-B2", QPointF(x2, y2));
		}
	}
	if (leftOrRight == false && above == false) {
		qDebug() << "we debugging in here..........";
		auto x1 = positionInView.x() - offset;
		auto y1 = positionInView.y() + 1 * offset;
		QPointF tmp1 = mapToScene(x1, y1);
		SceneEclipse* eclipse1 = new SceneEclipse(tmp1.x(), tmp1.y(), scene);
		if (startOrEnd == true) {
			eclipse1->setSignalName(startSignalId + "-B1");
			m_eclipseMap.insert(startSignalId + "-B1", eclipse1);
			dal.insertViewSignalTable(startSignalId, positionInView.x(), positionInView.y());
			dal.updateViewSignalB1(startSignalId, eclipse1->getColorState());
			scene->addItem(eclipse1);
			qDebug() << "which signal ID is added..." << startSignalId + "-B1";
			m_eclipseViewCodinates.insert(startSignalId + "-B1", QPointF(x1, y1));
		}
		else {
			eclipse1->setSignalName(endSignalId + "-B1");
			m_eclipseMap.insert(endSignalId + "-B1", eclipse1);
			dal.insertViewSignalTable(endSignalId, positionInView.x(), positionInView.y());
			dal.updateViewSignalB1(endSignalId, eclipse1->getColorState());
			scene->addItem(eclipse1);
			qDebug() << "which signal ID is added..." << endSignalId + "-B1";
			m_eclipseViewCodinates.insert(endSignalId + "-B1", QPointF(x1, y1));
		}
		auto x2 = positionInView.x() - 2 * offset;
		auto y2 = positionInView.y() + 1 * offset;
		QPointF tmp2 = mapToScene(x2, y2);
		SceneEclipse* eclipse2 = new SceneEclipse(tmp2.x(), tmp2.y(), scene);
		if (startOrEnd == true) {
			eclipse2->setSignalName(startSignalId + "-B2");
			m_eclipseMap.insert(startSignalId + "-B2", eclipse2);
			dal.insertViewSignalTable(startSignalId, positionInView.x(), positionInView.y());
			dal.updateViewSignalB2(startSignalId, eclipse2->getColorState());
			scene->addItem(eclipse2);
			m_eclipseViewCodinates.insert(startSignalId + "-B2", QPointF(x2, y2));
			qDebug() << "which signal ID is added..." << startSignalId + "-B2";
		}
		else {
			eclipse2->setSignalName(endSignalId + "-B2");
			m_eclipseMap.insert(endSignalId + "-B2", eclipse2);
			dal.insertViewSignalTable(endSignalId, positionInView.x(), positionInView.y());
			dal.updateViewSignalB2(endSignalId, eclipse2->getColorState());
			scene->addItem(eclipse2);
			qDebug() << "which signal ID is added..." << endSignalId + "-B2";
			m_eclipseViewCodinates.insert(endSignalId + "-B2", QPointF(x2, y2));
		}
	}
	m_eclipseMapLock.unlock();
}
void TrackView::drawSignals() {
	addEclipsesToLine("4G", true, false, false);  // 在 "3G" 线段的开头左对齐地添加两个 SceneEclipse 对象，这两个对象位于线段上方
	addEclipsesToLine("4G", false, true, true);  // 在 "3G" 线段的开头左对齐地添加两个 SceneEclipse 对象，这两个对象位于线段上方
	addEclipsesToLine("3G", true, false, false);  // 在 "3G" 线段的开头左对齐地添加两个 SceneEclipse 对象，这两个对象位于线段上方
	addEclipsesToLine("3G", false, true, true);  // 在 "3G" 线段的开头左对齐地添加两个 SceneEclipse 对象，这两个对象位于线段上方
	addEclipsesToLine("3-11DG", true, true, true);  // 在 "3G" 线段的开头左对齐地添加两个 SceneEclipse 对象，这两个对象位于线段上方
	addEclipsesToLine("IG", true, false, false);  // 在 "3G" 线段的开头左对齐地添加两个 SceneEclipse 对象，这两个对象位于线段上方
	addEclipsesToLine("4DG", true, true, true);  // 在 "3G" 线段的开头左对齐地添加两个 SceneEclipse 对象，这两个对象位于线段上方
	addEclipsesToLine("X1LQG", true, false, true);  // 在 "3G" 线段的开头左对齐地添加两个 SceneEclipse 对象，这两个对象位于线段上方
	addEclipsesToLine("1101G", true, true, true);  // 在 "3G" 线段的开头左对齐地添加两个 SceneEclipse 对象，这两个对象位于线段上方
	addEclipsesToLine("1103G", true, true, true);  // 在 "3G" 线段的开头左对齐地添加两个 SceneEclipse 对象，这两个对象位于线段上方
	addEclipsesToLine("1105G", true, true, true);  // 在 "3G" 线段的开头左对齐地添加两个 SceneEclipse 对象，这两个对象位于线段上方
	addEclipsesToLine("1107G", true, true, true);  // 在 "3G" 线段的开头左对齐地添加两个 SceneEclipse 对象，这两个对象位于线段上方
	addSignalLightsToLine("1108G");
	addSignalLightsToLine("1106G");
	addSignalLightsToLine("1104G");
	addSignalLightsToLine("1102G");
	addSignalLightsToLine("2DG");
	//addSignalLightsToLine("IIG");
	addSignalLightsToLine("1-9DG");
	auto x = linePositions.value("1-9DG").first.x();
	auto y = linePositions.value("1-9DG").first.y();
	auto sx = x;
	auto sy = y;
	y = linePositions.value("1-9DG").first.y() + 20;
	x = x;
	auto z1 = mapToScene(x, y);
	SceneEclipse* eclipse1 = new SceneEclipse(z1.x(), z1.y(), scene);
	eclipse1->setSignalName("XN-B1");
	m_eclipseMapLock.lock();
	m_eclipseMap.insert("XN-B1", eclipse1);
	//this->signalPositionMap.insert("XN", QPointF(sx, sy));
	dal.insertViewSignalTable("XN", sx, sy);
	m_eclipseViewCodinates.insert("XN-B1", QPointF(x, y));
	scene->addItem(eclipse1);
	x = x + 20;
	auto z2 = mapToScene(x, y);
	SceneEclipse* eclipse2 = new SceneEclipse(z2.x(), z2.y(), scene);
	eclipse1->setSignalName("XN-B2");
	m_eclipseMap.insert("XN-B2", eclipse2);
	m_eclipseViewCodinates.insert("XN-B2", QPointF(x, y));
	scene->addItem(eclipse2);
	x = linePositions.value("IIG").second.x();
	y = linePositions.value("IIG").second.y();
	sx = x;
	sy = y;
	y = linePositions.value("IIG").second.y() - 40;
	x = x + 10;
	auto z3 = mapToScene(x, y);
	SceneEclipse* eclipse3 = new SceneEclipse(z3.x(), z3.y(), scene);
	eclipse3->setSignalName("XII-B1");
	m_eclipseMap.insert("XII-B1", eclipse3);
	//this->signalPositionMap.insert("XII", QPointF(sx, sy));
	dal.insertViewSignalTable("XII", sx, sy);
	m_eclipseViewCodinates.insert("XII-B1", QPointF(x, y));
	scene->addItem(eclipse3);
	x = x + 20;
	auto z4 = mapToScene(x, y);
	SceneEclipse* eclipse4 = new SceneEclipse(z4.x(), z4.y(), scene);
	eclipse4->setSignalName("XII-B2");
	m_eclipseMap.insert("XII-B2", eclipse4);
	m_eclipseMapLock.unlock();
	m_eclipseViewCodinates.insert("XII-B2", QPointF(x, y));
	scene->addItem(eclipse4);
	InitialSignalColor();


}
void TrackView::drawSwitchs() {
	addTextToLine("3-11DG", "5", 0.25, false);  // 在 "3G" 线段的中间上方添加一个 SceneText 对象
	addTextToLine("3-11DG", "3", 0.5, false);  // 在 "3G" 线段的中间上方添加一个 SceneText 对象
	addTextToLine("3-11DG", "11", 0.75, true);  // 在 "3G" 线段的中间上方添加一个 SceneText 对象
	addTextToLine("4DG", "4", 0.5, true);  // 在 "3G" 线段的中间上方添加一个 SceneText 对象
	addTextToLine("1-9DG", "1", 0.25, true);  // 在 "3G" 线段的中间上方添加一个 SceneText 对象
	addTextToLine("1-9DG", "7", 0.5, true);  // 在 "3G" 线段的中间上方添加一个 SceneText 对象
	addTextToLine("1-9DG", "9", 0.75, false);  // 在 "3G" 线段的中间上方添加一个 SceneText 对象
	addTextToLine("2DG", "2", 0.5, false);  // 在 "3G" 线段的中间上方添加一个 SceneText 对象
	return;
}
void TrackView::addTextToLine(const QString& lineName, const QString& text, qreal positionRatio, bool above)
{
	QPair<QPointF, QPointF> positions = linePositions.value(lineName);
	QPointF start = positions.first;
	QPointF end = positions.second;
	qreal offset = 5;  // 这是你想要的距离，你可以根据需要修改它
	// 计算线段上的位置
	qreal x = start.x() + positionRatio * (end.x() - start.x());
	qreal y = start.y();
	//保留一份
	qreal xpp = start.x() + positionRatio * (end.x() - start.x());
	qreal ypp = start.y();
	//qreal y = start.y() + positionRatio * (end.y() - start.y());
	if (above == true) {  //在线之上
		y = y - 30;
	}
	else {	//在线之下
		y = y + offset;
	}
	dal.insertViewTurnoutTable(text, xpp, ypp, "Green");
	QPointF positionInScene = mapToScene(x, y);
	SceneText* sceneText = new SceneText(text, positionInScene, Qt::green);
	m_textMap.insert(text, sceneText);
	qDebug() << "we start writ text...." << text;
	scene->addItem(sceneText);
	// 如果 above 为 true，则在线段上方添加 SceneText 对象，否则在下方添加
	//switchPositionMapLock.lock();
	//// 使用 mapToScene 方法将视图坐标转换为场景坐标
	//switchPositionMap.insert(text, QPointF(xpp, ypp));
	//QPointF positionInScene = mapToScene(x, y);
	//// 创建 SceneText 对象并添加到场景中
	//SceneText* sceneText = new SceneText(text, positionInScene,Qt::green);
	//m_textMap.insert(text, sceneText);
	//qDebug() << "we start writ text...." << text;
	//scene->addItem(sceneText);
	//switchPositionMapLock.unlock()
}
void TrackView::addBlinkingSquare(const QString& squareName, const QPointF& loc, const QColor& color, const qreal size)
{
	QPointF positionInScene = mapToScene(loc.x(), loc.y());
	BlinkingSquare* square = new BlinkingSquare(squareName, positionInScene, color, size);
	m_blinksquareMap.insert(squareName, square);
	scene->addItem(square);
}
void TrackView::drawInrouteButton() {
	//在界面上画X处的按钮X-A1
	QPair<QPointF, QPointF> positions = linePositions.value("3-11DG");
	QPointF start = positions.first;
	qreal x = start.x();
	qreal y = start.y();
	qreal offset = 40;
	y = start.y() - offset;
	x = x - offset;
	qreal size = 20;
	addBlinkingSquare("X-A1", QPointF(x, y), Qt::green, size);
	x = x - 20;
	addBlinkingSquare("X-A2", QPointF(x, y), Qt::blue, size);
	x = x - 20;
	addBlinkingSquare("X-A3", QPointF(x, y), Qt::green, size);
	//在界面上画X处的按钮XN
	x = linePositions.value("1-9DG").first.x();
	y = linePositions.value("1-9DG").first.y();
	y = linePositions.value("1-9DG").first.y() + 20;
	x = x - offset;
	addBlinkingSquare("XN-A1", QPointF(x, y), Qt::green, size);
	x = x - 20;
	addBlinkingSquare("XN-A2", QPointF(x, y), Qt::blue, size);
	x = linePositions.value("3G").second.x();
	y = linePositions.value("3G").second.y();
	y = linePositions.value("3G").second.y() - size * 2;
	x = x - 1.5 * size;
	addBlinkingSquare("X3-A1", QPointF(x, y), Qt::green, size);
	x = linePositions.value("4G").second.x();
	y = linePositions.value("4G").second.y();
	y = linePositions.value("4G").second.y() - size * 2;
	x = x - 1.5 * size;
	addBlinkingSquare("X4-A1", QPointF(x, y), Qt::green, size);
	x = linePositions.value("IG").second.x();
	y = linePositions.value("IG").second.y();
	y = linePositions.value("IG").second.y() - size * 2;
	x = x - 1 * size;
	addBlinkingSquare("XI-A1", QPointF(x, y), Qt::green, size);
	x = linePositions.value("IIG").second.x();
	y = linePositions.value("IIG").second.y();
	y = linePositions.value("IIG").second.y() - size * 2;
	x = x - 1 * size;
	addBlinkingSquare("XII-A1", QPointF(x, y), Qt::green, size);
	x = linePositions.value("3G").first.x();
	y = linePositions.value("3G").first.y();
	y = linePositions.value("3G").first.y() + size;
	x = x + 0.5 * size;
	addBlinkingSquare("S3-A1", QPointF(x, y), Qt::green, size);
	x = linePositions.value("IG").first.x();
	y = linePositions.value("IG").first.y();
	y = linePositions.value("IG").first.y() + size;
	x = x + 0.5 * size;
	addBlinkingSquare("SI-A1", QPointF(x, y), Qt::green, size);
	x = linePositions.value("IIG").first.x();
	y = linePositions.value("IIG").first.y();
	y = linePositions.value("IIG").first.y() + size;
	x = x + 0.5 * size;
	addBlinkingSquare("SII-A1", QPointF(x, y), Qt::green, size);
	x = linePositions.value("4G").first.x();
	y = linePositions.value("4G").first.y();
	y = linePositions.value("4G").first.y() + size;
	x = x + 0.5 * size;
	addBlinkingSquare("S4-A1", QPointF(x, y), Qt::green, size);
	//
	x = linePositions.value("X1LQG").first.x();
	y = linePositions.value("X1LQG").first.y();
	y = linePositions.value("X1LQG").first.y() - 40;
	x = x + 0.5 * 20;
	addBlinkingSquare("SN-A1", QPointF(x, y), Qt::green, size);
	x = x + 20;
	addBlinkingSquare("SN-A2", QPointF(x, y), Qt::blue, size);
	x = linePositions.value("1102G").first.x();
	y = linePositions.value("1102G").first.y();
	y = linePositions.value("1102G").first.y() + 20;
	x = x + 0.5 * 20;
	addBlinkingSquare("S-A1", QPointF(x, y), Qt::green, size);
	x = x + 20;
	addBlinkingSquare("S-A2", QPointF(x, y), Qt::blue, size);
	x = x + 20;
	addBlinkingSquare("S-A3", QPointF(x, y), Qt::green, size);
	return;
}
void TrackView::drawTextAtPosition(const QPointF& positionInView, const QString& text, const QColor& color, const QFont& font)
{
	// 将视图坐标转换为场景坐标
	QPoint positionInViewPoint = positionInView.toPoint();
	QPointF positionInScene = mapToScene(positionInViewPoint);
	// 创建一个 QGraphicsTextItem 对象
	QGraphicsTextItem* textItem = new QGraphicsTextItem(text);
	// 设置文本颜色
	textItem->setDefaultTextColor(color);
	// 设置文本字体
	textItem->setFont(font);
	// 设置文本位置
	textItem->setPos(positionInScene);
	// 将文本项添加到场景中
	scene->addItem(textItem);
}
void TrackView::drawSignallables() {
	//画X
	qreal x = linePositions.value("3-11DG").first.x();
	qreal y = linePositions.value("3-11DG").first.y();
	QFont font("Arial", 10);
	drawTextAtPosition(QPointF(x - 20, y - 70), "X", Qt::yellow, font);
	drawTextAtPosition(QPointF(x - 90, y - 70), "XTA", Qt::yellow, font);
	x = linePositions.value("1-9DG").first.x();
	y = linePositions.value("1-9DG").first.y();
	drawTextAtPosition(QPointF(x - 30, y + 30), "XN", Qt::yellow, font);
	x = linePositions.value("1-9DG").first.x();
	y = linePositions.value("1-9DG").first.y();
	drawTextAtPosition(QPointF(x - 30, y + 30), "XN", Qt::yellow, font);
	x = linePositions.value("3G").first.x();
	y = linePositions.value("3G").first.y();
	drawTextAtPosition(QPointF(x - 10, y + 30), "S3", Qt::yellow, font);
	x = linePositions.value("3G").second.x();
	y = linePositions.value("3G").second.y();
	drawTextAtPosition(QPointF(x - 20, y - 60), "X3", Qt::yellow, font);
	x = linePositions.value("IG").first.x();
	y = linePositions.value("IG").first.y();
	drawTextAtPosition(QPointF(x - 10, y + 30), "SI", Qt::yellow, font);
	x = linePositions.value("IG").second.x();
	y = linePositions.value("IG").second.y();
	drawTextAtPosition(QPointF(x - 10, y - 60), "XI", Qt::yellow, font);
	x = linePositions.value("IIG").first.x();
	y = linePositions.value("IIG").first.y();
	drawTextAtPosition(QPointF(x - 10, y + 30), "SII", Qt::yellow, font);
	x = linePositions.value("IIG").second.x();
	y = linePositions.value("IIG").second.y();
	drawTextAtPosition(QPointF(x - 10, y - 60), "XII", Qt::yellow, font);
	x = linePositions.value("4G").first.x();
	y = linePositions.value("4G").first.y();
	drawTextAtPosition(QPointF(x - 10, y + 30), "S4", Qt::yellow, font);
	x = linePositions.value("4G").second.x();
	y = linePositions.value("4G").second.y();
	drawTextAtPosition(QPointF(x - 20, y - 60), "X4", Qt::yellow, font);
	x = linePositions.value("X1LQG").first.x();
	y = linePositions.value("X1LQG").first.y();
	drawTextAtPosition(QPointF(x - 20, y - 70), "SN", Qt::yellow, font);
	x = linePositions.value("1102G").first.x();
	y = linePositions.value("1102G").first.y();
	drawTextAtPosition(QPointF(x - 10, y + 30), "S", Qt::yellow, font);
	drawTextAtPosition(QPointF(x + 40, y + 30), "STA", Qt::yellow, font);
	x = linePositions.value("1104G").first.x();
	y = linePositions.value("1104G").first.y();
	drawTextAtPosition(QPointF(x - 20, y + 30), "1102", Qt::yellow, font);
	x = linePositions.value("1106G").first.x();
	y = linePositions.value("1106G").first.y();
	drawTextAtPosition(QPointF(x - 20, y + 30), "1104", Qt::yellow, font);
	x = linePositions.value("1108G").first.x();
	y = linePositions.value("1108G").first.y();
	drawTextAtPosition(QPointF(x - 20, y + 30), "1106", Qt::yellow, font);
	x = linePositions.value("S1LQG").first.x();
	y = linePositions.value("S1LQG").first.y();
	drawTextAtPosition(QPointF(x - 20, y + 30), "1108", Qt::yellow, font);
	x = linePositions.value("1101G").first.x();
	y = linePositions.value("1101G").first.y();
	drawTextAtPosition(QPointF(x - 30, y - 70), "1101", Qt::yellow, font);
	x = linePositions.value("1103G").first.x();
	y = linePositions.value("1103G").first.y();
	drawTextAtPosition(QPointF(x - 30, y - 70), "1103", Qt::yellow, font);
	x = linePositions.value("1105G").first.x();
	y = linePositions.value("1105G").first.y();
	drawTextAtPosition(QPointF(x - 30, y - 70), "1105", Qt::yellow, font);
	x = linePositions.value("1107G").first.x();
	y = linePositions.value("1107G").first.y();
	drawTextAtPosition(QPointF(x - 30, y - 70), "1107", Qt::yellow, font);
	return;
}
void TrackView::drawPath(const QString& pathName, const QPointF& start, const QPointF& end, const QColor& color)
{
	QPointF startScenePos = mapToScene(start.x(), start.y());
	QPointF endScenePos = mapToScene(end.x(), end.y());
	QGraphicsLineItem* line = new QGraphicsLineItem(startScenePos.x(), startScenePos.y(), endScenePos.x(), endScenePos.y());
	line->setPen(QPen(color, 2));
	line->setToolTip(pathName);
	scene->addItem(line);
	pathLines[pathName] = line;
}
//专门为道岔操作定操和反操还有失去表示时候用到的函数
void TrackView::drawPathForSwitch(const QString& pathName, const QPointF& start, const QPointF& end, const QColor& color)
{
	QPointF startScenePos = mapToScene(start.x(), start.y());
	QPointF endScenePos = mapToScene(end.x(), end.y());
	// 计算直线的长度
	qreal distance = QLineF(startScenePos, endScenePos).length();
	// 如果直线长度小于等于 10,则直接绘制整条直线
	if (distance <= 10)
	{
		QGraphicsLineItem* line = new QGraphicsLineItem(startScenePos.x(), startScenePos.y(), endScenePos.x(), endScenePos.y());
		line->setPen(QPen(color, 2));
		line->setToolTip(pathName);
		scene->addItem(line);
		pathLines[pathName] = line;
	}
	// 否则绘制固定长度的线段
	else
	{
		// 计算线段的长度
		qreal segmentLength = 10;
		// 计算线段的方向
		QLineF linePath(startScenePos, endScenePos);
		QPointF direction = linePath.unitVector().p2();
		// 绘制线段
		QPointF currentPos = startScenePos;
		QGraphicsLineItem* line = new QGraphicsLineItem(currentPos.x(), currentPos.y(), currentPos.x() + direction.x() * segmentLength, currentPos.y() + direction.y() * segmentLength);
		line->setPen(QPen(color, 2));
		line->setToolTip(pathName);
		scene->addItem(line);
		pathLines[pathName] = line;
	}
}
void TrackView::removePath(const QString& pathName)
{
	if (pathLines.contains(pathName))
	{
		QGraphicsLineItem* line = pathLines[pathName];
		scene->removeItem(line);
		pathLines.remove(pathName);
		delete line;
	}
}
//这个地方要修改一下，因为一条进路不一定全是
void TrackView::drawPathBetweenSignals(const QString& fromSignal, const QString& toSignal, QColor color) {
	QPointF fromPos = this->dal.getSignalPositionInView(fromSignal);
	QPointF toPos = this->dal.getSignalPositionInView(toSignal);
	QString pathName = fromSignal + "->" + toSignal;
	QPointF startScenePos = mapToScene(fromPos.x(), fromPos.y());
	QPointF endScenePos = mapToScene(toPos.x(), toPos.y());
	QGraphicsLineItem* line = new QGraphicsLineItem(startScenePos.x(), startScenePos.y(), endScenePos.x(), endScenePos.y());
	line->setPen(QPen(color, 2));
	line->setToolTip(pathName);
	scene->addItem(line);
	pathLines[pathName] = line;
}
void TrackView::InitialSignalStatus() {
	{
		// 将所有 instationSignals 中的信号机设置为关闭状态
		for (const QString& signalId : Configurations::instationSignals)
		{
			this->signalStatusMap.insert(signalId, SignalStatus::Closed);
		}
		// 将所有 outstationSignals 中的信号机设置为开放状态
		for (const QString& signalId : Configurations::outstationSignals)
		{
			this->signalStatusMap.insert(signalId, SignalStatus::Open);
		}
	}
}
//将信号机状态设置为关闭状态 
void TrackView::SetSignalStatusToClose(QString signalName) {
	this->signalStatusMap.insert(signalName, SignalStatus::Closed);
}
//将信号机状态设置为开放状态 
void TrackView::SetSignalStatusToOpend(QString signalName) {
	this->signalStatusMap.insert(signalName, SignalStatus::Open);
}
void TrackView::InitialSignalColor() {
	setSignalLightColorInScene("1107", "Yellow");
	setSignalLightColorInScene("1105", "GreenYellow");
	setSignalLightColorInScene("1103", "Green");
	setSignalLightColorInScene("1101", "Green");
	setSignalLightColorInScene("1108", "Green");
	setSignalLightColorInScene("1106", "Green");
	setSignalLightColorInScene("1104", "GreenYellow");
	setSignalLightColorInScene("1102", "Yellow");
	// 设置所有站内信号机为红色
	for (const QString& signalId : Configurations::instationSignals)
	{
		setSignalLightColorInScene(signalId, "Red");
	}
}

//首次打开界面的时候，需要对界面中的信号机颜色进行初始化
void TrackView::setSignalLightColorInScene(const QString& signalId, const QString& desiredColor) {
	m_eclipseMapLock.lock();
	QString light1, light2;
	// 对于站外信号机
	if (Configurations::outstationSignals.contains(signalId))
	{
		if (desiredColor == "Red")
		{
			light1 = "Red";
			light2 = "Black";
		}
		else if (desiredColor == "Green")
		{
			light1 = "Green";
			light2 = "Black";
		}
		else if (desiredColor == "Yellow")
		{
			light1 = "Yellow";
			light2 = "Black";
		}
		else if (desiredColor == "GreenYellow")
		{
			light1 = "Green";
			light2 = "Yellow";
		}
		else {	//还有一种情况是变为全黑色，这在反向运行的时候会这样
			light1 = "Black";
			light2 = "Black";
		}
	}
	// 对于站内信号机
	else if (Configurations::instationSignals.contains(signalId))
	{
		if (desiredColor == "Yellow")
		{
			light1 = "Black";
			light2 = "Yellow";
		}
		else if (desiredColor == "GreenYellow")
		{
			light1 = "Yellow";
			light2 = "Green";
		}
		else if (desiredColor == "Green")
		{
			light1 = "Black";
			light2 = "Green";
		}
		else if (desiredColor == "YellowYellow")
		{
			light1 = "Yellow";
			light2 = "Yellow";
		}
		else if (desiredColor == "Red")
		{
			light1 = "Red";
			light2 = "Black";
		}
		else if (desiredColor == "WhiteRed")
		{
			light1 = "White";
			light2 = "Red";
		}
	}
	// 更新 m_eclipseMap 中对应信号机的两个表示灯的颜色
	if (m_eclipseMap.contains(signalId + "-B1") && m_eclipseMap.contains(signalId + "-B2"))
	{
		qDebug() << "out runin here is........" << signalId << "designed color.." << desiredColor << "contained..." << light1 << light2;
		scene->removeItem(m_eclipseMap[signalId + "-B1"]);
		scene->removeItem(m_eclipseMap[signalId + "-B2"]);
		m_eclipseMap.remove(signalId + "-B1");
		m_eclipseMap.remove(signalId + "-B2");

		dal.updateViewSignalB1(signalId, light1);
		dal.updateViewSignalB2(signalId, light2);

		auto z1 = mapToScene(this->m_eclipseViewCodinates[signalId + "-B1"].x(), this->m_eclipseViewCodinates[signalId + "-B1"].y());
		SceneEclipse* eclipse1 = new SceneEclipse(z1.x(), z1.y(), scene, light1);
		eclipse1->setSignalName(signalId + "-B1");
		m_eclipseMap.insert(signalId + "-B1", eclipse1);
		scene->addItem(eclipse1);
		auto z2 = mapToScene(this->m_eclipseViewCodinates[signalId + "-B2"].x(), this->m_eclipseViewCodinates[signalId + "-B2"].y());
		SceneEclipse* eclipse2 = new SceneEclipse(z2.x(), z2.y(), scene, light2);
		eclipse2->setSignalName(signalId + "-B2");
		m_eclipseMap.insert(signalId + "-B2", eclipse2);
		scene->addItem(eclipse2);
	}
	m_eclipseMapLock.unlock();
}
void TrackView::addSignalLightsToLine(const QString& lineName)
{
	//开始，结束信号机的ID是MBB： "SII" "XN" for linename... "1-9DG"
	QPair<QPointF, QPointF> positions = linePositions.value(lineName);
	QPointF start = positions.first;
	QPointF end = positions.second;
	QString startSignalId = dal.getStartSignalId(lineName);
	QString endSignalId = dal.getEndSignalId(lineName);
	qDebug() << "开始，结束信号机的ID是MBB：" << startSignalId << endSignalId << "for linename..." << lineName;
	QPointF positionInView = end;
	auto xCoord = positionInView.x();
	auto yCoord = positionInView.y() + 20;
	QPointF tmp1 = mapToScene(xCoord, yCoord);
	SceneEclipse* eclipse1 = new SceneEclipse(tmp1.x(), tmp1.y(), scene);
	eclipse1->setSignalName(startSignalId + "-B1");
	m_eclipseMap.insert(startSignalId + "-B1", eclipse1);
	//this->signalPositionMap.insert(startSignalId, QPointF(positionInView.x(), positionInView.y()));
	this->dal.insertViewSignalTable(startSignalId, positionInView.x(), positionInView.y());
	m_eclipseViewCodinates.insert(startSignalId + "-B1", QPointF(xCoord, yCoord));
	scene->addItem(eclipse1);
	xCoord = positionInView.x() - 20;
	yCoord = positionInView.y() + 20;
	QPointF tmp2 = mapToScene(xCoord, yCoord);
	SceneEclipse* eclipse2 = new SceneEclipse(tmp2.x(), tmp2.y(), scene);
	eclipse2->setSignalName(startSignalId + "-B2");
	m_eclipseMap.insert(startSignalId + "-B2", eclipse2);
	m_eclipseViewCodinates.insert(startSignalId + "-B2", QPointF(xCoord, yCoord));
	scene->addItem(eclipse2);
}
bool TrackView::isSignalRed(const QString& signalId) {
	SceneEclipse* light1 = m_eclipseMap.value(signalId + "-B1");
	SceneEclipse* light2 = m_eclipseMap.value(signalId + "-B2");
	if (light1 && light2) {
		QString color1 = light1->getColorState();
		QString color2 = light2->getColorState();
		return ((color1 == "Red" && color2 == "Black") || (color1 == "Black" && color2 == "Red"));
	}
	return false;
}
QString TrackView::getSignalColorByName(QString signal_name) {
	m_eclipseMapLock.lock();
	SceneEclipse* light1 = nullptr;
	SceneEclipse* light2 = nullptr;
	if (m_eclipseMap.contains(signal_name + "-B1")) {
		light1 = m_eclipseMap.value(signal_name + "-B1");
	}
	if (m_eclipseMap.contains(signal_name + "-B2")) {
		light2 = m_eclipseMap.value(signal_name + "-B2");
	}
	m_eclipseMapLock.unlock();
	QString colorState;
	if (light1) {
		colorState += light1->getColorState();
	}
	if (light2) {
		colorState += light2->getColorState();
	}
	return colorState;
}
void TrackView::handlerchangeTracksectionColorRequest(QString sectionid, QString colorsate) {
	auto mytracksection = this->m_lineMap.value(sectionid);
	mytracksection->changeColor(colorsate);
	dal.updateViewTrackSectionStatus(sectionid, colorsate);
}
void TrackView::handlerdrawPathForTurnoutChange(QString lineName, QPointF switchPos, QPointF endPos, QColor lineColor) {
	qDebug() << "我们要画什么线段。。。。";
	//drawPathForSwitch(lineName, switchPos, endPos, lineColor);
	drawPathForSwitch_modified(lineName, switchPos, endPos, lineColor);
}
void TrackView::handlerdrawTurnoutTextForTurnoutChange(QString turnoutid, QString textcolor) {
	SceneText* switchText = this->m_textMap[turnoutid];
	switchText->setColor(textcolor);
	dal.updateTurnoutColor(turnoutid, textcolor);
}
void TrackView::drawPathForSwitch_modified(const QString& pathName, const QPointF& start, const QPointF& end, const QColor& color)
{
	QPointF startScenePos = mapToScene(start.x(), start.y());
	QPointF endScenePos = mapToScene(end.x(), end.y());
	// Calculate the total distance between start and end points
	qreal totalDistance = QLineF(startScenePos, endScenePos).length();
	// Calculate the length of the segment as one-fifth of the total distance
	qreal segmentLength = totalDistance / 5;
	// Calculate the direction vector from start to end
	QLineF linePath(startScenePos, endScenePos);
	QPointF direction = linePath.unitVector().p2() - linePath.unitVector().p1();
	// Calculate the end position of the segment
	QPointF segmentEndPos = startScenePos + segmentLength * direction;
	// Create and draw the line segment
	QGraphicsLineItem* line = new QGraphicsLineItem(QLineF(startScenePos, segmentEndPos).toLine());
	line->setPen(QPen(color, 10));
	line->setToolTip(pathName);
	scene->addItem(line);
	pathLines[pathName] = line;
}