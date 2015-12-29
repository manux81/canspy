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


#include "qcanmonitor.h"
#include "canbus/can_drv.h"

#include <QRegExp>


QCanMonitor::QCanMonitor(QObject *parent) :
	QCanPacketConsumer(parent),
	m_reg_pattern("[0-9a-fA-F]+$")
{

}

void QCanMonitor::canPacketRecv(can_packet_t packet)
{
	emit packetReceived(packet);
}

bool QCanMonitor::filterCallback(can_packet_t *packet)
{
	bool match = false;
	QString strPktId = QString::number(packet->id & EFF_MASK, 16);
	if (! m_reg_pattern.isEmpty()) {
		QRegExp regexp = QRegExp(m_reg_pattern);
		match = (regexp.isValid()) ?
		        regexp.exactMatch(strPktId) : true;
	} else
		match = true;

	return match;
}

void QCanMonitor::setFilterId(const QString &string)
{
	m_reg_pattern = string;
}
