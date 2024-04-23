#include "PhyDeviceManager.h"
#include "DataAccessLayer.h"

//这个类负责以统一的方式管理所有硬件设备，并使用数据库保持这些硬件设备的状态是异性的
PhyDeviceManager::~PhyDeviceManager()
{}

void PhyDeviceManager::getUpdatedStatusForSignals() {
	auto allSignalIds = da.getAllSignalIds();
    for (const QString& signalId : allSignalIds)
    {
        auto signal= new Phy_Signal();
        signal->setSignalID(signalId);
        physignal2map[signalId] = signal;
        
    }
}

void PhyDeviceManager::getUpdateStatusForSections() {
    
    da.populatePhyTrackSectionMap(this->physectionmap);

}



void PhyDeviceManager::handlerTurnoutStatusUpdatedMessage(QString id, QString status) {

    qDebug() << "CCCCCCCCCCCCCCCCCCCCCCCCCCCCC...." << id << status;
    // 检查 phyTurnoutMap 中是否已经存在该转辙机
    if (phyTurnoutMap.contains(id)) {

        // 如果存在,则获取对应的 Phy_Turnout 对象
        Phy_Turnout* turnout = phyTurnoutMap[id];

        // 检查状态是否发生变化
        if (turnout->getStatus() != status) {
            // 更新 Phy_Turnout 对象的状态
            turnout->setStatus(status);
            // 发出转辙机状态变化消息
            emit turnoutStatusChanged(id, status);
        }
    }
    else {
        // 如果不存在,则创建一个新的 Phy_Turnout 对象并添加到 phyTurnoutMap 中
        qDebug() << "我们在内存中新加了一个Phy_Turnout信息....";
        Phy_Turnout* newTurnout = new Phy_Turnout(id);
        newTurnout->setStatus(status);
        phyTurnoutMap[id] = newTurnout;
        // 发出转辙机状态变化消息
        emit turnoutStatusChanged(id, status);
    }
}

void PhyDeviceManager::handlerTrackSingaleUpdatedMessage(QString id, QString colorstatus) {

    // 检查 physignal2map 中是否已经存在该信号机
    if (physignal2map.contains(id)) {
        // 如果存在,则获取对应的 Phy_Signal 对象
        Phy_Signal* signal = physignal2map[id];
        QString the_colorstatus = colorstatus.split("|").first() + colorstatus.split("|").last();
        // 检查状态是否发生变化
        if (signal->getColorStatus() != the_colorstatus) {
            // 更新 Phy_Signal 对象的状态
            signal->settColorStatus(colorstatus);
            signal->settLight1(colorstatus.split("|").first().trimmed());
            signal->settLight2(colorstatus.split("|").last().trimmed());
            // 发出信号机颜色状态变化消息
            emit trackSignalColorStatusChanged(id, colorstatus);
        }
    }
    else {
        // 如果不存在,则创建一个新的 Phy_Signal 对象并添加到 physignal2map 中
        Phy_Signal* newSignal = new Phy_Signal();
        newSignal->setSignalID(id);
        QString the_colorstatus = colorstatus.split("|").first().trimmed() + colorstatus.split("|").last().trimmed();
        newSignal->settColorStatus(the_colorstatus);
        newSignal->settLight1(colorstatus.split("|").first().trimmed());
        newSignal->settLight2(colorstatus.split("|").last().trimmed());
        physignal2map[id] = newSignal;
        // 发出信号机颜色状态变化消息
        emit trackSignalColorStatusChanged(id, colorstatus);
    }
    return;
}




//这一段是没有运行的，也还是需要MainWindow转发
void PhyDeviceManager::handlertrackSectionUpdatedMessage(QStringList trackSectionIds, QStringList statuses) {
    //
    if (trackSectionIds.size() != statuses.size()) {
        qDebug() << "Error: The size of trackSectionIds does not match the size of statuses.";
        return;
    }

    for (int i = 0; i < trackSectionIds.size(); ++i) {
        QString id = trackSectionIds[i];
        QString status = statuses[i];

        if (physectionmap.contains(id)) {
            Phy_TrackSection* section = physectionmap[id];
            if (section->getStatus() != status) {
                section->setStatus(status);
                da.updateTrackSectionStatus(id, status);
                emit trackSectionStatusChanged(id, status);
            }
        }
        else {
            Phy_TrackSection* newSection = new Phy_TrackSection();
            newSection->setTrackSectionId(id);
            newSection->setStatus(status);
            physectionmap.insert(id, newSection);
            emit trackSectionStatusChanged(id, status);
        }
    }
}



QString PhyDeviceManager::updateAndFetchTrackSectionStatus(const QString& trackSectionId) {
    // Fetch the current status from the database
    QString dbStatus = da.getTrackSectionStatus(trackSectionId);

    // Fetch the current status from the map
    QString mapStatus = "Unknown";
    if (physectionmap.contains(trackSectionId)) {
        mapStatus = physectionmap[trackSectionId]->getStatus();
    }

    // Compare the statuses
    if (dbStatus != mapStatus) {
        // Update the database status
        da.updateTrackSectionStatus(trackSectionId, mapStatus);
    }

    // Return the map status
    return mapStatus;
}


Phy_TrackSection* PhyDeviceManager::getTrackSectionBySignalIds(const QString& startSignalId, const QString& endSignalId) {
    // Try to find the track section with the given start and end signal IDs
    for (Phy_TrackSection* section : physectionmap.values()) {
        if ((section->getStartSignalId() == startSignalId && section->getEndSignalId() == endSignalId) ||
            (section->getStartSignalId() == endSignalId && section->getEndSignalId() == startSignalId)) {
            return section;
        }
    }

    // If no matching track section is found, return nullptr
    return nullptr;
}



//总是返回最新的信号机状态给
Phy_Signal* PhyDeviceManager::getLastedColorForSignal(QString tsignal) {
    for (Phy_Signal* the_signal : physignal2map.values()) {
        if (the_signal->getSignalID() == tsignal) {
            if (the_signal->getLight1() == "" || the_signal->getLight2() == "") {
                //the_signal->query();
                auto light1light2 = da.queryPhySignalTable(tsignal);
                the_signal->settLight1(light1light2.first);
                the_signal->settLight2(light1light2.second);
                return the_signal;
            }
        }
    }
    // If no matching track section is found, return nullptr
    return nullptr;

}



QString PhyDeviceManager::getPhyTurnoutStatus(QString turnoutId) {
    // 检查 phyTurnoutMap 中是否已经存在该转辙机
    if (phyTurnoutMap.contains(turnoutId)) {
        // 如果存在,则获取对应的 Phy_Turnout 对象并返回其状态
        return phyTurnoutMap[turnoutId]->getStatus();
    }
    else {
        // 如果不存在,则返回一个默认值或抛出异常
        return "Unknown";
    }
}



QString PhyDeviceManager::getPhySignalColorStatusById(QString signalID) {
    auto the_stats = this->physignal2map.value(signalID);
    return the_stats->getColorStatus();
}
