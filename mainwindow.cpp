#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ProjectInitializer.h"
#include <QScreen>
#include <QSerialPortInfo>
#include <QGraphicsScene>
#include <QGraphicsView>
#include "BlinkingSquare.h"
#include "SceneEclipse.h"
#include "sceneline.h"
#include "SceneText.h"
#include "TrackView.h"
#include "InterlockControl.h"
#include <QObject>
#include <QObject>
#include <QStateMachine>
#include <QState>
#include <QFinalState>
#include "InterlockingRouteManager.h"
#include "ViewLogicalControl.h"
#include "TrackComander.h"
#include "PhyDeviceManager.h"
#include "TracktologyBuilder.h"
#include "MenuPushButton.h"
#include <QSerialPort>
#include "dataport.h"
MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ProjectInitializer initializer("D:/myDb.db");
	initializer.initializeDatabase();
	initializer.constructRouteTable(); //该进路表，只是记录用户
	showMaximized();
	auto scene_size = qApp->primaryScreen()->availableSize();
	this->scen_size = scene_size;
	this->setFixedSize(scene_size);
	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowCloseButtonHint);
	da = new DataAccessLayer();
	mtoplogyscene = new QGraphicsScene(this);
	TrackView* trackview = new TrackView(mtoplogyscene, this);
	trackview->setBackgroundBrush(QBrush(Qt::black));
	trackview->setGeometry(QRect(0, 0, scene_size.width(), scene_size.height())); //QGraphicsView的大小和位置不随主窗口变化？？？
	//是把视图对象和场景对象关联起来，这样在用maptoscene的时候是依据这种关联的方式最终找到对应的场景中的坐标位置
	trackview->setScene(mtoplogyscene);
	trackview->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	trackview->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	trackview->setRenderHint(QPainter::Antialiasing);
	trackview->setStyleSheet("border:none; background:black;");
	trackview->setMouseTracking(true);
	//也就说如果将场景的集合中心设置为视图的宽高度一半，高度和宽度设置为视图的高度和宽度，这场景对象的中心就对准了视图中心位置了
	mtoplogyscene->setSceneRect(0 - (trackview->width() / 2),
		0 - (trackview->height() / 2),
		trackview->width(), trackview->height()); //设置中心点的坐标，然后要找出内圈和外圈半圆的位置，都以中心来定位
	trackview->drawTrackSection();
	trackview->drawSignals();
	trackview->drawSwitchs();
	trackview->drawInrouteButton();
	trackview->drawSignallables();
	//trackview->InitialSignalStatus();
	trackview->show();
	// 在QGraphicsView中，坐标系统的原点(0, 0)默认是在视图的左上角，然后向右是x轴的正方向，向下是y轴的正方向。所以，当你在视图中指定一个坐标(x, y)时，这个坐标是相对于视图的左上角来计算的。
	// 在QGraphicsScene中，坐标系统的原点默认是在场景的中心，也就是说，场景的宽度和高度都是从负值开始，向右和向下增加。所以，当你在场景中指定一个坐标(x, y)时，这个坐标是相对于场景的中心来计算的。
	// 这就是为什么在默认情况下，如果你在场景中创建一个新的图形项并将其位置设置为(0, 0)，这个图形项会出现在视图的中心，而不是左上角。因为(0, 0)在场景中表示的是场景的中心，在视图中表示的是视图的左上角。
	// 连接 BlinkingSquare 的 clicked 信号到自定义的槽函数
	//connect(square, &BlinkingSquare::clicked, this, &MainWindow::onSquareClicked);
	//解决串口通信问题
	myserialobj = new DataPort(DataPort_Type::SERIAL_PORT);
	connect(myserialobj, SIGNAL(sig_received(QByteArray)), this, SLOT(handleSerialReceivedData(QByteArray)));
	connect(myserialobj, SIGNAL(sig_error(QString)), this, SLOT(handleSerialError(QString)));
	connect(myserialobj, SIGNAL(sig_opened()), this, SLOT(handleSerialOpened()));
	connect(myserialobj, SIGNAL(sig_closed()), this, SLOT(handleSerialClosed()));
	m_interlockcontrol = new InterlockControl(this, trackview);
	routeManager = new InterlockingRouteManager(this);
	this->viewlogicalController = new ViewLogicalControl(this);
	this->m_trackComander = new TrackComander(this);
	this->m_hardwareagent = new TracktologyBuilder(this);
	this->m_deviceManager = new PhyDeviceManager(this);
	viewlogicalController->setFriend(m_deviceManager);
	m_trackComander->setDeviceManager(m_deviceManager);
	m_interlockcontrol->setFriend(m_deviceManager);
	startState = new QState(); //联锁控制逻辑的开始状态
	//关联连锁对象去监控
	differentButtonState = new QState();
	sameButtonState = new QState();
	endState = new QFinalState();
	turnoutCheckedState = new QState();
	// 遍历m_blinksquareMap,将所有按钮的cliced事件进行联锁控制绑定

	for (auto it = trackview->m_blinksquareMap.begin(); it != trackview->m_blinksquareMap.end();
		++it) {
		// 获取BlinkingSquare对象的指针
		BlinkingSquare* square = it.value();
		qDebug() << "BlinkingSquare* square.... " << square->getbuttonName();
		// 将BlinkingSquare的clicked信号连接到MyObject的buttonClicked槽函数
		QObject::connect(square, &BlinkingSquare::clicked, m_interlockcontrol, &InterlockControl::buttonClicked);
		QObject::connect(square, &BlinkingSquare::clicked, this, &MainWindow::handleBlinkSquareClicked);
	}

	//
	for (auto it = trackview->m_textMap.begin(); it != trackview->m_textMap.end();
		++it) {
		// 获取SceneText对象的指针
		SceneText* square = it.value();
		qDebug() << "swich text.... " << square->getSwitchName();
		// 将switch的clicked信号连接到MyObject的buttonClicked槽函数,驱动道岔定表返表
		QObject::connect(square, &SceneText::clicked, this, &MainWindow::handleSwitchClicked);
	}





	startState->addTransition(m_interlockcontrol, &InterlockControl::differentButtonClicked, differentButtonState);
	startState->addTransition(m_interlockcontrol, &InterlockControl::sameButtonClicked, sameButtonState);
	sameButtonState->addTransition(sameButtonState, SIGNAL(entered()), endState);
	QObject::connect(differentButtonState, &QState::entered, routeManager, &InterlockingRouteManager::handleRouteReuqest);
	QObject::connect(routeManager, &InterlockingRouteManager::routeRequestPassChecked, m_interlockcontrol, &InterlockControl::operateTurnouts);
	QObject::connect(m_interlockcontrol, &InterlockControl::notifyViewControllerLockSwitch, viewlogicalController, &ViewLogicalControl::handlerSwitchLockingRequet);
	//我们需要在后面加入，点击开始和结束按钮的时候，
	QObject::connect(m_interlockcontrol, &InterlockControl::notifyTrackCommanderSwitchOperation, m_trackComander, &TrackComander::handlerSwitchOperationRequest);
	QObject::connect(m_trackComander, &TrackComander::turnoutOperationComandSend, m_interlockcontrol, &InterlockControl::checkSwitchLocation);
	//当对进路的所有转辙机进行检查通过后，我们给视图控制层发送消息
	QObject::connect(m_interlockcontrol, &InterlockControl::turnoutPassChecked, viewlogicalController, &ViewLogicalControl::handlerInroutePathReuest);
	QObject::connect(m_interlockcontrol, &InterlockControl::needValidateTrackSectionDirection, m_trackComander, &TrackComander::handlerTrackDirectionCheckRequet);
	QObject::connect(m_interlockcontrol, &InterlockControl::sectionDirectionPassChecked, viewlogicalController, &ViewLogicalControl::handlerSignalDrawRequestForInroutePath);
	//QObject::connect(m_hardwareagent, &TracktologyBuilder::trackSectionUpdated, m_deviceManager, &PhyDeviceManager::handlertrackSectionUpdatedMessage);
	//QObject::connect(m_hardwareagent, &TracktologyBuilder::turoutStatusUpdated, m_deviceManager, &PhyDeviceManager::handlerTurnoutStatusUpdatedMessage);
	QObject::connect(m_hardwareagent, &TracktologyBuilder::trackSignalStatusUpdated, m_deviceManager, &PhyDeviceManager::handlerTrackSingaleUpdatedMessage);


	//当从下位机获得这些消息的时候，需要进行一些处理
	QObject::connect(m_deviceManager, &PhyDeviceManager::turnoutStatusChanged, viewlogicalController, &ViewLogicalControl::handlerTurnoutStatusUpdatedEvent);
	QObject::connect(m_deviceManager, &PhyDeviceManager::trackSignalColorStatusChanged, viewlogicalController, &ViewLogicalControl::handlerSignalColorUpdatedEvent);
	QObject::connect(m_deviceManager, &PhyDeviceManager::trackSectionStatusChanged, viewlogicalController, &ViewLogicalControl::handlerTrackSectionStatusUpdatedEvent);


	QObject::connect(viewlogicalController, &ViewLogicalControl::checklightcolorForInroute, m_trackComander, &TrackComander::handlerSignalColorOperaion);
	//我们继续让m_interlockcontrol帮忙检查进路所涉及到的信号机的颜色是否已经修改到了指定状态
	QObject::connect(m_trackComander, &TrackComander::checkSignalColorStatusForRoute, m_interlockcontrol, &InterlockControl::checkSignalColorStatus);
	QObject::connect(m_interlockcontrol, &InterlockControl::InterlockingFaMA, m_trackComander, &TrackComander::handlerFama);
	QObject::connect(m_interlockcontrol, &InterlockControl::switchDirectOperationRequested, m_trackComander, &TrackComander::handlerSwitchDingcaoOperation);
	QObject::connect(m_interlockcontrol, &InterlockControl::switchReverseOperationRequested, m_trackComander, &TrackComander::handlerSwitchReverseOperation);


	QObject::connect(m_trackComander, &TrackComander::newSwitchDirectCommandStored, this, &MainWindow::handleNewSwitchDirectCommand);
	QObject::connect(this, &MainWindow::switchDirectOperationSucceeded, m_interlockcontrol, &InterlockControl::handleSwitchDirectOperationSuccess);
	QObject::connect(this, &MainWindow::switchDirectOperationFailed, m_interlockcontrol, &InterlockControl::handleSwitchDirectOperationFailure);

	//继续绑定
	QObject::connect(m_trackComander, &TrackComander::newSwitchReverseCommandStored, this, &MainWindow::handleNewSwitchReverseCommand);
	QObject::connect(this, &MainWindow::switchReverseOperationSucceeded, m_interlockcontrol, &InterlockControl::handleSwitchReverseOperationSuccess);
	QObject::connect(this, &MainWindow::switchReverseOperationFailed, m_interlockcontrol, &InterlockControl::handleSwitchReverseOperationFailure);

	QObject::connect(m_interlockcontrol, &InterlockControl::drawSwitchDirectedInScene, viewlogicalController, &ViewLogicalControl::handlerSwitchDirectedRequest);
	QObject::connect(m_interlockcontrol, &InterlockControl::drawSwitchReverseInScene, viewlogicalController, &ViewLogicalControl::handlerSwitchReverseRequest);
	
	QObject::connect(m_interlockcontrol, &InterlockControl::drawSwitchLostInScene, viewlogicalController, &ViewLogicalControl::handlerSwitchLostRequest);
	QObject::connect(this, &MainWindow::DrivePhySignalStatusRequest, m_interlockcontrol, &InterlockControl::handlerDrivePhySignalStatusRequest);
	QObject::connect(m_interlockcontrol, &InterlockControl::notifyTrackCommanderForDrivingSignalChange, m_trackComander, &TrackComander::DrivePhySignalStatus);
	//使某个进路无效
	QObject::connect(m_interlockcontrol, &InterlockControl::notifyViewControllerRemoveInroutePath, viewlogicalController, &ViewLogicalControl::handlerInoutePathClearRequest);






	//视图逻辑控制器发出通知，通知继续向实物信号机发送驱动指令 //检查是否可以进行发码
	QObject::connect(viewlogicalController, &ViewLogicalControl::notifySignalStatusInconsistence, m_trackComander, &TrackComander::DriverSpecifiedSignalToSpecifiedColor);
	QObject::connect(viewlogicalController, &ViewLogicalControl::notifySignalStatusCheckandPossibleFama, m_interlockcontrol, &InterlockControl::handlerSignalStatusCheckandPossibleFama);
	QObject::connect(viewlogicalController, &ViewLogicalControl::notifySegmentClearEvent, m_interlockcontrol, &InterlockControl::handlerSegmentClearEvent);
	QObject::connect(viewlogicalController, &ViewLogicalControl::notifySegmentTakenEvent, m_interlockcontrol, &InterlockControl::handlerSegmmentTakenEvent);



	
	
	

	QObject::connect(m_interlockcontrol, &InterlockControl::notifyViewControllerSectionColorShoudChange, viewlogicalController, &ViewLogicalControl::controlTrackSectionColor);
	QObject::connect(m_interlockcontrol, &InterlockControl::driverSignalColorInShapan, m_trackComander, &TrackComander::DriverSpecifiedSignalToSpecifiedColor);



	QObject::connect(viewlogicalController, &ViewLogicalControl::drawPathForTurnoutChange, trackview, &TrackView::handlerdrawPathForTurnoutChange);
	QObject::connect(viewlogicalController, &ViewLogicalControl::drawTurnoutTextForTurnoutChange, trackview, &TrackView::handlerdrawTurnoutTextForTurnoutChange);
	QObject::connect(viewlogicalController, &ViewLogicalControl::changeTracksectionColorRequest, trackview, &TrackView::handlerchangeTracksectionColorRequest);


	



	
	machine.addState(startState);
	machine.addState(differentButtonState);
	machine.addState(sameButtonState);
	machine.addState(endState);
	machine.setInitialState(startState);
	machine.start();
	//startState->addTransition(m_interlockcontrol, &InterlockControl::differentButtonClicked, differentButtonState);
	//startState->addTransition(m_interlockcontrol, &MyObject::sameButtonClicked, sameButtonState);
	InitialMenuButtonGroupAndArrangeTheirPosition();
	InitialSerialPortCommnication(initializer.databasePath_);
	this->tracktopstore = new TracktologyBuilder();

	//auto signal2colormap = generateSignalColorMap();
	//m_trackComander->DrivePhySignalStatus(signal2colormap);  //初始化发码
	m_trackComander->InitalPhyFaMa();  //初始化发码
	emit DrivePhySignalStatusRequest();    //通知视图控制层向数据库发送信息

	bindSwitchOperationEvent();
	bindserialEventsToTrackGraphStore();


	//auto byte1 = convertStrToByte(binaryStr.left(8));

}
//最终，我们将界面状态也进行存储
//初始化信号机状态，包括实物和非实物信号机状态，将这些信号机状态直接发送给轨道拓扑信号进行管理
void MainWindow::bindserialEventsToTrackGraphStore() {
	connect(this, &MainWindow::phySwithStatus, this->tracktopstore, &TracktologyBuilder::updateRealSwithStatus);
	connect(this, &MainWindow::PhySignalStatus, this->tracktopstore, &TracktologyBuilder::updateRealSignalStatus);
	connect(this, &MainWindow::VirtualSwithStatus, this->tracktopstore, &TracktologyBuilder::updateFakeSwitchStatus);
	connect(this, &MainWindow::FackSignalstatus, this->tracktopstore, &TracktologyBuilder::updateFakeSignalStatus);
	connect(this, &MainWindow::TrackSectionStatus, this->tracktopstore, &TracktologyBuilder::updatTrackSectionStatus);
	connect(tracktopstore, &TracktologyBuilder::turoutStatusUpdated, this, &MainWindow::transferturoutStatusToPhyDeviceManager);
	connect(this, &MainWindow::turoutStatusTrasnferinng, m_deviceManager, &PhyDeviceManager::handlerTurnoutStatusUpdatedMessage);

	//QObject::connect(m_hardwareagent, &TracktologyBuilder::trackSectionUpdated, m_deviceManager, &PhyDeviceManager::handlertrackSectionUpdatedMessage);

	connect(tracktopstore, &TracktologyBuilder::trackSectionUpdated, this, &MainWindow::transtrackSectionStatusToPhyDeviceManager);
	connect(this, &MainWindow::trackSectionStatusTransfering, m_deviceManager, &PhyDeviceManager::handlertrackSectionUpdatedMessage);


	

	

}


void MainWindow::transferturoutStatusToPhyDeviceManager(QString zz, QString mm) {
	
	emit turoutStatusTrasnferinng(zz, mm);
	return;
}

//只是作为信号传递使用
void MainWindow::transtrackSectionStatusToPhyDeviceManager(QStringList _t1, QStringList _t2) {
	emit trackSectionStatusTransfering(_t1, _t2);
}



MainWindow::~MainWindow()
{
	delete ui;
}
/*
现在我需要你辅助我设计一套铁路信号控制系统的仿真软件，我们使用QT C++实现了该仿真软件的一些组成部分，
例如，我们通过串口从硬件获取了实物信号机，转辙机，轨道区段的状态，这些状态都存储在sqllite3数据库中，我们设计了视图层，在视图层中，我们通过线段和圆来模拟和真实轨道完全相同的轨道拓扑结构中的区段以及信号机，我们还用数据库建立了一个关于联锁进路的所有进路信息表，在这个表中，我们记录了进路的起始信号和终止信号机，以及所经过的轨道区段，以及轨道区段所经过的转辙机。现在我们需要设计控制逻辑层，该控制逻辑需要控制视图层，当用户在视图层指定要办理从哪个信号机到哪个信号机的，从数据库里面获取对应各种轨道对象的状态，首先是要判断所办理进路和已有进路是否存在冲突，一般来说，我们判断是否冲突的原理是比较简单的，也就说如果所要办理的进路和已经办理过的进路存在区段和区段的重叠，就一定代表存在冲突，能办理指定进路的条件包括是否有轨道区段被占用，如果被占用了肯定是不能开进路的。如果确定打开某个进路，我们会首先尝试将进路路径中所经过的转辙机转向到指定的位置。这里转向到指定的位置是依据轨道当前运行方向，如果我们假设列车按照进路的直线方向行驶，则，进路中的所有的转辙机应该处于Normal状态，但我们知道，列车不可能永远直线行驶，因此，进路中所包含的转辙机有的状态肯定必须是Reversed状态，这样列车才能从正线转到侧线行驶，或者是其它交叉线路行驶。如果不能办理进路，例如，我们发现转辙机已经失去表示了，例如lost了，则我们应该把所有有关信号机，转辙机状态，以及在界面中的所对应的颜色都恢复到该进路排列前的状态。如果可以排列进路了，我们会根据铁道信号中的联锁发码逻辑，在对应区段上发送特定的编码，并始终依据前方空闲区段的个数，来实现对列车的联锁控制。因此，我需要你给我详细分析一下，我的控制层要如何设计，我应该存储所有界面中的有关对象的当前状态信息对么？或者你有没有明白我说的？
*/
void MainWindow::handleMenuButtonClicked(QString ButtonName) {
	menuButtonManagerName = ButtonName;
	// 重新开始计时器
	menutimer.start();
	// 设置按钮控制为有效
	isButtonControlEnabled = true;
	qDebug() << ".....................";
}
void MainWindow::handleMenuButtonTimerTimeout() {
	isButtonControlEnabled = false;
}
void MainWindow::handleBlinkSquareClicked(QString buttonName) {
	qDebug() << "is isButtonControlEnabled valid" << isButtonControlEnabled;
	if (isButtonControlEnabled == true) {
		if (menuButtonManagerName != "") {
			qDebug() << "we start processing .......route mamage";
		}
	}
}
void MainWindow::handleSwitchClicked(QString switchName) {
	qDebug() << "BBBBBBBBBBBBBBBB" << switchName;
	qDebug() << "is isButtonControlEnabled valid" << isButtonControlEnabled;
	if (isButtonControlEnabled == true) {
		if (menuButtonManagerName != "") {
			qDebug() << "we start processing .......switch mamage" << menuButtonManagerName;
			if (menuButtonManagerName == "总定位") {
				//直接判断是否被锁定
				auto switch_locked = this->viewlogicalController->getSecneSwitchLockStatus(switchName);
				qDebug() << "是否被锁定" << switch_locked;
				if (switch_locked == true) {
					return;
				}
				//第二次检查BUG2：道岔无采集时，仍然能操动道岔并在软件上有显示物理信号机是否失去表示，如果都没表示的话，也不允许操作
				auto switch_status_ = m_deviceManager->getPhyTurnoutStatus(switchName);
				if (switch_status_ == "Lost" || switch_status_ == "Unknown") {
					return;
				}
				emit requestSwitchDirectOperator(switchName);
				return;
			}
			if (menuButtonManagerName == "总反位") {
				//直接判断是否被锁定
				auto switch_locked = this->viewlogicalController->getSecneSwitchLockStatus(switchName);
				qDebug() << "是否被锁定" << switch_locked;
				if (switch_locked == true) {
					return;
				}
				auto switch_status_ = m_deviceManager->getPhyTurnoutStatus(switchName);
				if (switch_status_ == "Lost" || switch_status_ == "Unknown") {
					return;
				}
				emit requestSwitchReverseOperator(switchName);
				return;
			}

		}
	}
}
void MainWindow::handleSerialReceivedData(const QByteArray& data)
{

	qDebug() << "Received data XXXXX.............." << data.toHex() << "MBsize...."<< data.size();;
	// 处理接收到的数据
	if (data.size() == 4)
	{

		unsigned short commandId = Utilty::bytesToUShort((unsigned char)data[0], (unsigned char)data[1]);
		qDebug() << "The original number is ...." << commandId;


		if ((unsigned char)data[2] == 0x29 && (unsigned char)data[3] == 0xf5)
		{
			qDebug() << "GGGGGGGGGGGGGGGGGGGGGGGG....." << data.size() << data.toHex() << "the command id is...."<< commandId;
			// 更新数据库中对应记录的 success 字段
			da->updateCommandTableSuccess(commandId, true);
		}
		else
		{
			qDebug() << "Last two bytes do not match 0x29 0xf5, ignoring.";
		}
		return;
	}
	if (data.size() == 5) {
		uint8_t startByte = uint8_t(data.at(0));
		qDebug() << "Received data:HHHHHHHHHHHHHHHHh " << data.toHex() << "and start byte is...."<< startByte;
		if ((unsigned char)startByte >= 0xB0 && (unsigned char)startByte <= 0xBB) {
			QByteArray tmpdata = data.mid(0, 3);
			uint16_t crc = Utilty::ModbusCRC16(tmpdata);
			if (data.mid(3, 2) == QByteArray::fromRawData((char*)&crc, 2)) {
				QByteArray returnba = data.mid(1, 2);
				qDebug() << "XXXXXXXXXXXXXXXXXXXXXXXXXOKKKKKKK....." << QString::number(startByte, 16).toUpper() << returnba.toHex();
				switch (startByte) {
				case 0xB0:
					emit phySwithStatus(returnba);
					break;
				case 0xB1:
					emit PhySignalStatus(returnba);
					break;
				case 0xB2:
					emit VirtualSwithStatus(returnba);
					break;
				case 0xB3:
					emit FackSignalstatus(returnba, 1);
					break;
				case 0xB4:
					emit FackSignalstatus(returnba, 2);
					break;
				case 0xB5:
					emit FackSignalstatus(returnba, 3);
					break;
				case 0xB6:
					emit FackSignalstatus(returnba, 4);
					break;
				case 0xB7:
					emit FackSignalstatus(returnba, 5);
					break;
				case 0xB8:
					qDebug() << "" << startByte << returnba.toHex();
					emit TrackSectionStatus(returnba, 1);
					break;
				case 0xB9:
					emit TrackSectionStatus(returnba, 2);
					break;
				case 0xBA:
					emit TrackSectionStatus(returnba, 10);
					break;
				case 0xBB:
					emit TrackSectionStatus(returnba, 11);
					break;
				default:
					break;
				}
			}
		}
	}
	
}
void MainWindow::handleSerialError(const QString& error)
{
	// 处理错误信息
	qDebug() << "Error: " << error;
}
void MainWindow::handleSerialOpened()
{
	// 处理串口打开成功的情况
	qDebug() << "Serial port opened successfully.";
}
void MainWindow::handleSerialClosed()
{
	// 处理串口关闭成功的情况
	qDebug() << "Serial port closed successfully.";
}
void MainWindow::handleSerialWriteRequet(const QByteArray& data) {
	//qDebug() << "Serial port write data out...HHHHHHHHH."<<data.toHex();
	myserialobj->write(data);
}
void MainWindow::InitialMenuButtonGroupAndArrangeTheirPosition() {
	menuButtonManager = new MenuPushButtonManager(this);
	menuButton1 = std::make_unique<MenuPushButton>("总取消", this);
	menuButton2 = std::make_unique<MenuPushButton>("总人解", this);
	menuButton3 = std::make_unique<MenuPushButton>("总定位", this);
	menuButton4 = std::make_unique<MenuPushButton>("总反位", this);
	menuButton5 = std::make_unique<MenuPushButton>("清除", this);
	menuButton6 = std::make_unique<MenuPushButton>("单解", this);
	menuButton7 = std::make_unique<MenuPushButton>("单锁", this);
	menuButton8 = std::make_unique<MenuPushButton>("道岔封锁", this);
	menuButton9 = std::make_unique<MenuPushButton>("道岔解封", this);
	// 将菜单按钮添加到MenuPushButtonManager中
	menuButtonManager->addMenuPushButton(menuButton1.get());
	menuButtonManager->addMenuPushButton(menuButton2.get());
	menuButtonManager->addMenuPushButton(menuButton3.get());
	menuButtonManager->addMenuPushButton(menuButton4.get());
	menuButtonManager->addMenuPushButton(menuButton5.get());
	menuButtonManager->addMenuPushButton(menuButton6.get());
	menuButtonManager->addMenuPushButton(menuButton7.get());
	menuButtonManager->addMenuPushButton(menuButton8.get());
	menuButtonManager->addMenuPushButton(menuButton9.get());
	// 设置菜单按钮的位置和大小
	int buttonWidth = 100;
	int buttonHeight = 50;
	int buttonX = 0;
	int buttonY = this->scen_size.height() - buttonHeight;
	menuButton1->setGeometry(buttonX, buttonY, buttonWidth, buttonHeight);
	menuButton2->setGeometry(buttonX + buttonWidth, buttonY, buttonWidth, buttonHeight);
	menuButton3->setGeometry(buttonX + 2 * buttonWidth, buttonY, buttonWidth, buttonHeight);
	menuButton4->setGeometry(buttonX + 3 * buttonWidth, buttonY, buttonWidth, buttonHeight);
	menuButton5->setGeometry(buttonX + 4 * buttonWidth, buttonY, buttonWidth, buttonHeight);
	menuButton6->setGeometry(buttonX + 5 * buttonWidth, buttonY, buttonWidth, buttonHeight);
	menuButton7->setGeometry(buttonX + 6 * buttonWidth, buttonY, buttonWidth, buttonHeight);
	menuButton8->setGeometry(buttonX + 7 * buttonWidth, buttonY, buttonWidth, buttonHeight);
	menuButton9->setGeometry(buttonX + 8 * buttonWidth, buttonY, buttonWidth, buttonHeight);
	// 显示菜单按钮
	menuButton1->show();
	menuButton2->show();
	menuButton3->show();
	menuButton4->show();
	menuButton5->show();
	menuButton6->show();
	menuButton7->show();
	menuButton8->show();
	menuButton9->show();
	menutimer.setSingleShot(true); // 一次性计时器
	menutimer.setInterval(5000); // 5秒钟
	// 连接计时器的超时信号到 handleTimerTimeout 槽函数
	connect(&menutimer, SIGNAL(timeout()), this, SLOT(handleMenuButtonTimerTimeout()));
	connect(menuButtonManager, &MenuPushButtonManager::menuButtonClicked, this, &MainWindow::handleMenuButtonClicked);
}
void MainWindow::InitialSerialPortCommnication(QString databasePath_) {
	QList<QSerialPortInfo> serialPortList = QSerialPortInfo::availablePorts();
	if (serialPortList.isEmpty()) {
		qDebug() << "No serial ports available. Please connect a serial device.";
		return;
	}
	else {
		// Enumerate available serial ports
		QList<QSerialPortInfo> serialPortList = QSerialPortInfo::availablePorts();
		bool found = false;
		QString selectedPortName = "";
		foreach(const QSerialPortInfo & info, serialPortList) {
			qDebug() << "Checking port:" << info.portName();
			qDebug() << "Description:" << info.description();
			qDebug() << "Manufacturer:" << info.manufacturer();
			if (info.description().contains("CH340") || info.manufacturer().contains("CH340")) {
				selectedPortName = info.portName();
				found = true;
				break;
			}
		}
		if (found) {
			qDebug() << "CH340 port found...:" << selectedPortName;
		}
		else {
			qDebug() << "No CH340 serial ports found.";
			return;
		}
		if (selectedPortName != "" && found == true) {
			//打开串口
			myserialobj->open(selectedPortName, 9600);//串口通信对象
			m_thread_xiaweiji = new SerialPortThread(selectedPortName, databasePath_, this); //线程不断遍历数据库消息，发送消息指令
			connect(&*m_thread_xiaweiji, &SerialPortThread::writeRequest, this, &MainWindow::handleSerialWriteRequet);
			m_thread_xiaweiji->start();
		}
	}
}
void MainWindow::bindSwitchOperationEvent() {
	connect(this, &MainWindow::requestSwitchDirectOperator, m_interlockcontrol, &InterlockControl::handlerSwitchDingcaoOperationRequest);
	connect(this, &MainWindow::requestSwitchReverseOperator, m_interlockcontrol, &InterlockControl::handlerSwitchReverseOperationRequest);


}
//; //处理TrackCommander的道岔执行操作请求
void MainWindow::handleNewSwitchDirectCommand(QString switchId, QString commandId)
{
	// 查询数据库,检查命令的执行状态
	bool success = da->iscomandRecordValid(commandId.toInt());
	if (success) {
		// 发出命令执行成功的信号
		emit switchDirectOperationSucceeded(switchId, commandId);
	}
	else {
		// 发出命令执行失败的信号
		emit switchDirectOperationFailed(switchId, commandId);
	}
}

void MainWindow::handleNewSwitchReverseCommand(QString switchId, QString commandId)
{
	// 查询数据库,检查命令的执行状态
	bool success = da->iscomandRecordValid(commandId.toInt());
	if (success) {
		// 发出命令执行成功的信号
		emit switchReverseOperationSucceeded(switchId, commandId);
	}
	else {
		// 发出命令执行失败的信号
		emit switchReverseOperationFailed(switchId, commandId);
	}
}


