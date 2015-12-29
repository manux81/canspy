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

#include "qcanrecvthread.h"
#include "os_utils.h"
#include "canbus/can_drv.h"

#include <QDebug>

QCanRecvThread::QCanRecvThread(QCanSocket *sk, QObject *parent) :
	QThread(parent)
{
	this->sk = sk;

	moveToThread(this);
	m_stop = false;
}

QCanRecvThread::~QCanRecvThread()
{
	QList<ConnectionFilter *>::const_iterator it;

	for (it = m_filter_list.begin(); it != m_filter_list.end(); ++it)
		delete (*it);
}

void QCanRecvThread::run()
{
	int r;
	can_packet_t packet;

	while (!m_stop) {
		r = sk->recv(&packet.id, &packet.dlc, (void *) packet.data, &packet.tv_sec,
		             &packet.tv_usec);
		if (r <= 0) {
			m_stop = 1;
			continue;
		}
		packet.direction = DIRECTION_RX;
		QList<ConnectionFilter *>::iterator it;

		for (it = m_filter_list.begin(); it != m_filter_list.end(); ++it) {
			if (!(*it)->consumer->filterCallback(&packet))
				continue;

			QCanBuffer *buffer = (*it)->buffer;
			buffer->packetRecvFromThread(packet);
		}
		if (packet.id & ERR_FLAG)
			QThread::usleep(2000);

		emit packetReceived(packet);
	}
	QThread::run();
}

void QCanRecvThread::stop()
{
	m_stop = true;
}

void QCanRecvThread::restart()
{
	m_stop = false;
	run();
}

void QCanRecvThread::linkPacketConsumer(QCanPacketConsumer *pkt_consumer)
{
	QCanBuffer *buffer = new QCanBuffer;

	connect(buffer, SIGNAL(packetReceived(can_packet_t)), pkt_consumer, SLOT(canPacketRecv(can_packet_t)),
	        Qt::QueuedConnection);
	m_map_buffers[pkt_consumer] = buffer;
	addFilterRule(pkt_consumer, buffer);
}

void QCanRecvThread::unlinkPacketConsumer(QCanPacketConsumer *pkt_consumer)
{
	if (! m_map_buffers.contains(pkt_consumer))
		return;

	QCanBuffer *buffer = m_map_buffers[pkt_consumer];

	disconnect(buffer);
	delete buffer;
	m_map_buffers.remove(pkt_consumer);
	removeFilterRule(pkt_consumer);
}

void QCanRecvThread::addFilterRule(QCanPacketConsumer *consumer, QCanBuffer *buffer)
{
	ConnectionFilter *filter = new ConnectionFilter(consumer, buffer);
	m_map_filters[consumer] = filter;
	m_filter_list.append(filter);
}

void QCanRecvThread::removeFilterRule(QCanPacketConsumer *consumer)
{
	if (! m_map_filters.contains(consumer))
		return;

	ConnectionFilter *filter = m_map_filters[consumer];
	delete filter;
	m_map_filters.remove(consumer);
	m_filter_list.removeOne(filter);
}
