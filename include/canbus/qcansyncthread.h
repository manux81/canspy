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


#ifndef _QCANSYNC_THREAD_H_
#define _QCANSYNC_THREAD_H_

#include "base/canbus/qcansendthread.h"
#include "base/canbus/qcansocket.h"

#include <stdint.h>
#include <QTimer>

class QCanSyncThread : public QCanSendThread {
	Q_OBJECT
	Q_PROPERTY(unsigned output READ output WRITE setOutput)

public:
	QCanSyncThread(QCanSocket *sk, QObject *parent = 0);
	~QCanSyncThread();

	unsigned output() const;
	void run();

public slots:
	void setOutput(unsigned output);
	void timeout();

protected:
	void initPeripheral();
	void SDOSendReq(uint16_t NodeID, uint16_t idx, uint16_t subIdx, uint32_t value, int nByte);
	void NMTSendCommand(uint8_t NodeID, uint8_t Cmd);

private:
	unsigned m_output;
	uint8_t m_sendbuf[8];
	QTimer *m_timer;
	bool m_init_req;
};

#endif
