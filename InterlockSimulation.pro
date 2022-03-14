QT       += core gui
QT       += serialport
QT       += serialbus
QT       += quick
QT       += widgets


greaterThan(QT_MAJOR_VERSION, 5): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Buttonwidget.cpp \
    CLinkList.cpp \
    CppObject.cpp \
    CustomDialog.cpp \
    CustomGraphicsArrow.cpp \
    CustomItemGroup.cpp \
    Helper.cpp \
    InterFilter.cpp \
    Interlockingevent.cpp \
    LNode.cpp \
    MenuPushButton.cpp \
    MyLineItem.cpp \
    MyRectItem.cpp \
    Myellipseitem.cpp \
    Mysignal.cpp \
    ReceiverThread.cpp \
    SeriesPortTask.cpp \
    ToplogyScene.cpp \
    Tracktologybuilder.cpp \
    Triangle.cpp \
    aaaa.cpp \
    main.cpp \
    mainwindow.cpp \
    segmentcircuitdiagram.cpp

HEADERS += \
    Buttonwidget.h \
    CLinkList.h \
    CppObject.h \
    CustomDialog.h \
    CustomGraphicsArrow.h \
    CustomItemGroup.h \
    Helper.h \
    InterFilter.h \
    Interlockingevent.h \
    LNode.h \
    MenuPushButton.h \
    MyLineItem.h \
    MyRectItem.h \
    Myellipseitem.h \
    Mysignal.h \
    ReceiverThread.h \
    SeriesPortTask.h \
    ToplogyScene.h \
    Tracktologybuilder.h \
    Triangle.h \
    aaaa.h \
    concurrentqueue.h \
    mainwindow.h \
    segmentcircuitdiagram.h

FORMS += \
    mainwindow.ui \
    segmentcircuitdiagram.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    blockingsegmentcd.qml
