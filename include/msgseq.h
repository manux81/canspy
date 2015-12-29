/*
 *  canspy - A simple tool for users who need to interface with a device based on
 *           CAN (CAN/CANopen/J1939/NMEA2000/DeviceNet) such as motors,
 *           sensors and many other devices.
 *  Copyright (C) 2015-2016  Manuele Conti (manuele.conti@gmail.com)
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * This code is made available on the understanding that it will not be
 * used in safety-critical situations without a full and competent review.
 */


#ifndef MSGSEQ_H
#define MSGSEQ_H

#include <QMainWindow>
#include <QAction>
#include <QDropEvent>
#include <QDragEnterEvent>

#include "qcanrecvthread.h"
#include "qcansendthread.h"
#include "qcansocket.h"
#include "qcanmonitor.h"
#include "logmodel.h"


namespace Ui {
class MsgSeq;
}

class MsgSeq : public QMainWindow
{
	Q_OBJECT

public:
	explicit MsgSeq(QMainWindow *parent = NULL, QCanMonitor *monitor = NULL,
	    QCanSendThread *send = NULL);
	~MsgSeq();

private:
	void rowToCanPacket(int row, can_packet_t &packet, uint64_t *msleep);
	void loadFile(QString fileName);
	can_packet_t processLine(const QString &line);

private slots:
	void addMsgButton_clicked(void);
	void removeAllButton_clicked(void);
	void removeSelectedButton_clicked(void);
	void sendSelectedButton_clicked(void);
	void sendAllButton_clicked(void);
	void openAction_triggered(void);

signals:
	void msgEnqueue(const can_packet_t &packet);

private:
	Ui::MsgSeq *ui;
	logModel *m_logmodel;
	QCanMonitor *m_monitor;
	QCanSendThread *m_send;
};

#endif // MSGSEQ_H
