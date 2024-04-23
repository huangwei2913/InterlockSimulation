QT       += core gui sql serialport concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    BlinkingSquare.cpp \
    DataAccessLayer.cpp \
    Phy_Signal.cpp \
    Phy_TrackSection.cpp \
    Phy_turnout.cpp \
    ProjectInitializer.cpp \
    SceneEclipse.cpp \
    SceneText.cpp \
    SerialPortThread.cpp \
    TrackView.cpp \
    TracktologyBuilder.cpp \
    main.cpp \
    mainwindow.cpp \
    sceneline.cpp

HEADERS += \
    BlinkingSquare.h \
    DataAccessLayer.h \
    Phy_Signal.h \
    Phy_TrackSection.h \
    Phy_turnout.h \
    ProjectInitializer.h \
    SceneEclipse.h \
    SceneText.h \
    SerialPortThread.h \
    TrackView.h \
    TracktologyBuilder.h \
    mainwindow.h \
    sceneline.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    C:/Users/大哥/PycharmProjects/pythonProject/paths.txt \
    C:/Users/大哥/PycharmProjects/pythonProject/printpath.py
