#ifndef DATAPORT_H
#define DATAPORT_H

#include "dataport_global.h"
#include "serialdataport.h"
#include <QSerialPort>
#include <QObject>

enum DataPort_Type{
	NULL_PORT,
	SERIAL_PORT,
};

class DataPort : public QObject
{
	Q_OBJECT
public:

	DataPort(DataPort_Type);
	~DataPort();

	int getPortType();

	void open(const QString& strAddress, const int& number);
	void write(const QByteArray& data);
	void close();

signals:
	//对外
	void sig_received(QByteArray data);
	void sig_error(QString);
	void sig_opened();
	void sig_closed();
	//对内
	void sig_wirte(QByteArray data);
	void sig_open(QString str, int number);
	void sig_close();
	void sig_quiting();

private:
	QThread* m_thread;
	DataPort_Type m_portType;
	SerialDataPort* m_serialDataPort;
};

#endif // DATAPORT_H
