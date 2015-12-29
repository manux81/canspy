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


#ifndef CAN_PACKET_H
#define CAN_PACKET_H

#include <stdint.h>

#define DIRECTION_RX 0
#define DIRECTION_TX 1

#pragma pack(1)
typedef struct can_packet {
	uint32_t id;
	uint8_t  dlc;
	uint8_t  data[8];
	int64_t tv_sec;
	int64_t tv_usec;
	uint8_t direction;
} can_packet_t;
#pragma pack()

#endif // CAN_PACKET_H
