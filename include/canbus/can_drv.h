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


#ifndef CAN_DRV_H
#define CAN_DRV_H

#include "canbus/can_state.h"
#include "os_utils.h"

#include <stdlib.h>
#include <stdint.h>

#include <QMap>

#define EFF_FLAG 0x80000000U /* EFF/SFF is set in the MSB */
#define RTR_FLAG 0x40000000U /* remote transmission request */
#define ERR_FLAG 0x20000000U /* error message frame */
#define EFF_MASK 0x1FFFFFFFU /* extended frame format (EFF) */

typedef struct {
	int (* create)(const char *, unsigned);
	int (* destroy)(int);
	int (* send)(int, unsigned, uint8_t, void *);
	int (* recv)(int, unsigned *, uint8_t *, void *,
	    int64_t *, int64_t *);
	int (* bitrate_set)(const char *, unsigned);
	int (* attribute_set)(unsigned, const void *, unsigned);
	int (* start)(const char *);
	int (* stop)(const char *);
	int (* state_get)(const char *, qcan_state_t *);
	int (* restart)(const char *);
} can_ops_t;

typedef struct {
	const char *name;
	can_ops_t *ops;
} can_drv_table_t;

extern can_ops_t * can_ops;
extern can_drv_table_t can_drv_table;

#endif // CAN_DRV_H
