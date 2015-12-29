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
#include "drivers/net_ops.h"
#include "drivers/simulation_ops.h"
#include "drivers/can_socket_ops.h"
#include "utils.h"
#include <sys/time.h>

int get_timestamp(int64_t *sec, int64_t *usec)
{
	struct timeval tv;

	gettimeofday(&tv, 0);
	if (sec) {
		*sec = tv.tv_sec;
	}
	if (usec) {
		*usec = tv.tv_usec;
	}

	return 0;
}

can_ops_t *get_can_ops(const char *name)
{
	can_ops_t *ret = NULL;

	if (!strcmp("USB2CAN 8Devices", name))
		ret = &can_socket_ops;
	if (!strcmp("PCAN-USB", name))
		ret = &can_socket_ops;
	if (!strcmp("CAN Over TCP", name))
		ret = &net_ops;
	if (!strcmp("Simulation", name))
		ret = &simulation_ops;
	return ret;
}

