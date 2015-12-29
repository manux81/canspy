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


#ifndef QCANSOCKET_H
#define QCANSOCKET_H

#include "canbus/can_state.h"

#include <QAbstractSocket>
#include <QString>
#include <stdint.h>
#include <QSemaphore>

class QCanSocket : public QAbstractSocket
{
	Q_OBJECT
	Q_PROPERTY(SocketState state READ state)
public:
	explicit QCanSocket(QString &dev, unsigned bitrate, QObject *parent = 0);
	explicit QCanSocket(const char *dev, unsigned bitrate, QObject *parent = 0);
	~QCanSocket();

	bool isSequential();

	int connect();
	int disconnect(void);

	int setBitrate(unsigned bitrate);
	int start();
	int stop();

	int getCanBusState(qcan_state_t *status);

	size_t send(unsigned id, uint8_t dlc, void *data);
	size_t recv(unsigned *id, uint8_t *dlc, void *data, int64_t *sec, int64_t *usec);

	SocketState state() const;

protected:
	/* Check the current can bus state and performs a reset if needed */
	int checkCurrentCanBusState(void);

signals:

public slots:

private:
	int skt;
	SocketState status;
	QSemaphore m_semaphore;
	QString m_dev;
	unsigned m_bitrate;
};

#endif // QCANSOCKET_H
