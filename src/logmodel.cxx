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


#include "logmodel.h"
#include "canbus/can_drv.h"
#include <QObject>
#include <QString>
#include <QDateTime>
#include <QDebug>
#include <QModelIndex>
#include <QTime>
#include <QDebug>



logModel::logModel(QObject *parent)
	: QCanPkgAbstractModel(parent)
{
	m_enable = true;
	m_hexLayout = true;
}

logModel::~logModel()
{
}

void logModel::setEnable(bool enable)
{
	m_enable = enable;
}

void logModel::setHexLayout(bool enable)
{
	m_hexLayout = enable;
}

void logModel::messageEnqueued(can_packet_t canpack)
{
	can_str_packet_t str_canpck;
	static int firstTime = 1;
	static int64_t lsec = 0, lusec = 0;
	static int64_t msec;

	if (! m_enable) {
		return;
	}

	str_canpck.id = QString::number(canpack.id & EFF_MASK, 16).toUpper();

	if (canpack.id & EFF_FLAG)
		str_canpck.flags = "Ext ";
	else
		str_canpck.flags = "Std ";

	if (canpack.id & RTR_FLAG)
		str_canpck.flags += "| Rtr";

	if (canpack.id & ERR_FLAG)
		str_canpck.flags += "| Err";

	QTime curTime;
	curTime = ((QDateTime::fromTime_t(canpack.tv_sec)).time());
	curTime = curTime.addMSecs(canpack.tv_usec / 1000);
	str_canpck.timestamp = curTime.toString("hh:mm:ss.zzz");

	if (firstTime == 1) {
		m_startTime = curTime;
		firstTime = 0;
		msec = 0;
	} else
		msec = (((lsec - canpack.tv_sec ) * 1000000) + (lusec - canpack.tv_usec))/ 1000;

	str_canpck.elapsed = QString::number(abs(msec));
	lsec = canpack.tv_sec;
	lusec = canpack.tv_usec;

	str_canpck.length = QString::number(canpack.dlc);

	str_canpck.data = "";
	for (int i = 0; i < canpack.dlc; i++) {
		QString tok;
		if (m_hexLayout)
			str_canpck.data += tok.sprintf("%02X ", canpack.data[i]).toUpper();
		else
			str_canpck.data += tok.sprintf("%c ", canpack.data[i]).toUpper();
	}
	str_canpck.direction = (canpack.direction == DIRECTION_RX) ? "Rx" : "Tx";
	beginInsertRows(QModelIndex(), 0, 0);
	msgList.push_front(str_canpck);
	endInsertRows();
}
