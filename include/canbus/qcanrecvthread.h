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


#ifndef QCanRecvThread_H
#define QCanRecvThread_H

#include "base/canbus/qcansocket.h"
#include "base/canbus/can_packet.h"
#include "base/canbus/qcanbuffer.h"
#include "base/canbus/qcanpacketconsumer.h"

#include <tr1/functional>
#include <QList>
#include <QThread>


Q_DECLARE_METATYPE(can_packet_t)

class QCanRecvThread : public QThread
{
	Q_OBJECT
public:
	explicit QCanRecvThread(QCanSocket *sk, QObject *parent = 0);
	~QCanRecvThread(void);

	void linkPacketConsumer(QCanPacketConsumer *pkt_consumer);
	void stop(void);
	void run(void);
	
signals:
	void packetReceived(can_packet_t packet);

protected:
	void addFilterRule(QCanPacketConsumer *consumer, QCanBuffer *buffer);

private:

	class ConnectionFilter {
	public:
		ConnectionFilter(QCanPacketConsumer *consumer, QCanBuffer *buffer) {
			this->consumer = consumer;
			this->buffer = buffer;
		}

		QCanPacketConsumer *consumer;
		QCanBuffer *buffer;
	};

	QCanSocket *sk;

	bool m_stop;
	QList<ConnectionFilter *> m_filter_list;
};

#endif // QCanRecvThread_H
