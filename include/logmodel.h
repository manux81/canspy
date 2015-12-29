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


#ifndef LOGMODEL_H
#define LOGMODEL_H

#include "canbus/can_packet.h"
#include "qcanpkgabstractmodel.h"
#include <QTime>

class logModel : public QCanPkgAbstractModel
{
	Q_OBJECT

	public slots:
		void messageEnqueued(can_packet_t);
	public:
		logModel(QObject *parent = 0);
		void setEnable(bool enable);
		~logModel();
		void setHexLayout(bool enable);

	QVector <can_str_packet_t> buffer;
	QTime m_lastTimer;
	bool m_enable;
	bool m_hexLayout;

};

#endif
