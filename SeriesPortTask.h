#ifndef SERIESPORTTASK_H
#define SERIESPORTTASK_H

#include <QRunnable>
#include "ReceiverThread.h"
#include  "concurrentqueue.h"


class SeriesPortTask:public QRunnable
{
public:
    SeriesPortTask(QByteArray ba, ReceiverThread* seriesportthread);
    ~SeriesPortTask();
    void run() Q_DECL_OVERRIDE;

private:
    ReceiverThread* portthread;
    QByteArray ba;
};

#endif // SERIESPORTTASK_H
