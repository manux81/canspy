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

#include "qcansocket.h"
#include "canbus/can_drv.h"
#include <QDebug>
#include <string>

QCanSocket::QCanSocket(QString &dev, unsigned bitrate, QObject *parent) :
	QAbstractSocket(UnknownSocketType, parent),
	m_semaphore(2)
{
	m_dev = dev;
	m_bitrate = bitrate;
	status = UnconnectedState;

}

QCanSocket::QCanSocket(const char *dev, unsigned bitrate, QObject *parent) :
	QAbstractSocket(UnknownSocketType, parent)
{
	QString sDev(dev);
	m_dev = sDev;
	m_bitrate = bitrate;
	status = UnconnectedState;
}

QCanSocket::~QCanSocket()
{
	if (status == ConnectedState)
		disconnect();
}

bool QCanSocket::isSequential()
{
	return true;
}

int QCanSocket::connect()
{
	status = ConnectingState;
	std::string dev_str = m_dev.toStdString();

	skt = can_ops->create(dev_str.c_str(), m_bitrate);
	if (skt < 0) {
		status = UnconnectedState;
		return skt;
	}
	setBitrate(m_bitrate);
	status = ConnectedState;
	emit connected();

	return skt;
}

int QCanSocket::disconnect()
{
	int r = can_ops->destroy(skt);
	if (r != -1) {
		skt = -1;
		status = UnconnectedState;
		emit disconnected();
	}
	return r;
}

int QCanSocket::setBitrate(unsigned bitrate)
{
	m_bitrate = bitrate;
	std::string dev_str = m_dev.toStdString();
	return can_ops->bitrate_set(dev_str.c_str(), bitrate);
}

int QCanSocket::start()
{
	std::string dev_str = m_dev.toStdString();
	return can_ops->start(dev_str.c_str());
}

int QCanSocket::stop()
{
	std::string dev_str = m_dev.toStdString();
	return can_ops->stop(dev_str.c_str());
}

size_t QCanSocket::send(unsigned id, uint8_t dlc, void *data)
{
	size_t ret;

	ret = 0U;
	checkCurrentCanBusState();
	if (skt > 0) {
		//while (! m_semaphore.tryAcquire(1, 500));
		ret = can_ops->send(skt, id, dlc, data);
		//m_semaphore.release(1);
	}
	return ret;
}

size_t QCanSocket::recv(unsigned *id, uint8_t *dlc, void *data, int64_t *sec, int64_t *usec)
{
	size_t ret;

	ret = 0U;
	checkCurrentCanBusState();
	if (skt > 0) {
		//while (! m_semaphore.tryAcquire(2, 500));
		ret = can_ops->recv(skt, id, dlc, data, sec, usec);
		//m_semaphore.release(2);
	}
	return ret;

}

QAbstractSocket::SocketState QCanSocket::state() const
{
	return this->status;
}

int QCanSocket::getCanBusState(qcan_state_t *status)
{
	std::string dev_str = m_dev.toStdString();
	int ret = can_ops->state_get(dev_str.c_str(), status);

	return ret;
}

int QCanSocket::checkCurrentCanBusState()
{
	qcan_state_t status;
	std::string dev_str;

	int r = getCanBusState(&status);
	if (r < 0) {
		disconnect();
		return r;
	}
	switch (status) {
	case QCAN_STATE_BUS_OFF:
		/* Restart the CAN bus */
		dev_str = m_dev.toStdString();
		can_ops->restart(dev_str.c_str());
		break;
	default:
		break;
	}

	return r;
}
