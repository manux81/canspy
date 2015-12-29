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

#include "canbus/can_drv.h"
#include "canbus/can_state.h"

#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <string.h>
#include <libsocketcan.h>
#include <unistd.h>

#ifndef PF_CAN
#define PF_CAN 29
#endif

#ifndef AF_CAN
#define AF_CAN PF_CAN
#endif


static int can_socket_create(const char *dev, unsigned bitrate);
static int can_socket_destroy(int fd);
static int can_socket_send(int fd, unsigned id, uint8_t dlc, void *data);
static int can_socket_recv(int fd, unsigned *id, uint8_t *dlc, void *data,
    int64_t *sec, int64_t *usec);
static int can_socket_bitrate_set(const char *device, unsigned bitrate);
static int can_socket_attribute_set(unsigned attribute, const void *value, unsigned value_len);
static int can_socket_start(const char *device);
static int can_socket_stop(const char *device);
static int can_socket_state_get(const char *device, qcan_state_t *status);
static int can_socket_restart(const char *device);


can_ops_t can_socket_ops = {
	.create = can_socket_create,
	.destroy = can_socket_destroy,
	.send = can_socket_send,
	.recv = can_socket_recv,
	.bitrate_set = can_socket_bitrate_set,
	.attribute_set = can_socket_attribute_set,
	.start = can_socket_start,
	.stop = can_socket_stop,
	.state_get = can_socket_state_get,
	.restart = can_socket_restart
};


static struct sockaddr_can canaddr;

int
can_socket_create(const char *dev, unsigned)
{
	int skt;
	int r;
	can_err_mask_t err_mask = 0U;
	struct can_filter filter;
	struct ifreq ifr;
	const int timestamp_on = 1;


	skt = socket(AF_CAN, SOCK_RAW, CAN_RAW);
	if (skt < 0)
		return skt;

	strcpy(ifr.ifr_name, dev);
	r = ioctl(skt, SIOCGIFINDEX, &ifr);
	if (r < 0)
		goto exit_error;

	canaddr.can_family = AF_CAN;
	canaddr.can_ifindex = ifr.ifr_ifindex;

	setsockopt(skt, SOL_CAN_RAW, CAN_RAW_ERR_FILTER,
	    &err_mask, sizeof(err_mask));

	filter.can_id = 0;
	filter.can_mask = 0 & (~CAN_ERR_FLAG);
	setsockopt(skt, SOL_CAN_RAW, CAN_RAW_FILTER,
	    &filter, sizeof(struct can_filter));

	setsockopt(skt, SOL_SOCKET, SO_TIMESTAMP,
	    &timestamp_on, sizeof(timestamp_on));

	r = bind(skt, (struct sockaddr *) &canaddr, sizeof(canaddr));
	if (r < 0)
		goto exit_error;

	return skt;

exit_error:
	close(skt);
	return r;
}

int
can_socket_destroy(int fd)
{
	return close(fd);
}

int
can_socket_send(int fd, unsigned id, uint8_t dlc, void *data)
{
	struct can_frame frame;

	if (dlc > 8)
		return -1;

	frame.can_id = id;
	if (dlc != 0)
		memcpy(frame.data, data, dlc);
	frame.can_dlc = dlc;

	return write(fd, &frame, sizeof(frame));
}

int
can_socket_recv(int fd, unsigned *id, uint8_t *dlc, void *data,
    int64_t *sec, int64_t *usec)
{
	struct can_frame frame;
	struct iovec iov;
	struct msghdr msg;
	struct cmsghdr *cmsg;
	char ctrlmsg[CMSG_SPACE(sizeof(struct timeval)) + CMSG_SPACE(sizeof(__u32))];
	struct timeval tv = { 0, 0};

	iov.iov_base = &frame;
	iov.iov_len = sizeof(frame);
	msg.msg_name = &canaddr;
	msg.msg_namelen = sizeof(canaddr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = &ctrlmsg;
	msg.msg_controllen = sizeof(ctrlmsg);
	msg.msg_flags = 0;
	int r = recvmsg(fd, &msg, 0);
	if (r <= 0)
		return r;
	for (cmsg = CMSG_FIRSTHDR(&msg);
	    cmsg && (cmsg->cmsg_level == SOL_SOCKET);
	    cmsg = CMSG_NXTHDR(&msg,cmsg)) {
		if (cmsg->cmsg_type == SO_TIMESTAMP)
			tv = *(struct timeval *)CMSG_DATA(cmsg);

	}

	*id = frame.can_id;
	if (frame.can_dlc > 8)
		return -1;

	if (frame.can_dlc != 0)
		memcpy(data, frame.data, frame.can_dlc);
	*dlc = frame.can_dlc;

	if (sec != NULL)
		*sec = tv.tv_sec;

	if (usec != NULL)
		*usec = tv.tv_usec;

	return r;
}

int
can_socket_bitrate_set(const char *device, unsigned bitrate)
{
	return can_set_bitrate(device, bitrate);
}

int
can_socket_attribute_set(unsigned, const void *, unsigned)
{
	return 0;
}

int
can_socket_start(const char *device)
{
	return can_do_start(device);
}

int
can_socket_stop(const char *device)
{
	return can_do_stop(device);
}

int
can_socket_restart(const char *device)
{
	return can_do_restart(device);
}

int
can_socket_state_get(const char *device, qcan_state_t *status)
{
	int st;
	int const r = can_get_state(device, &st);

	if (r < 0)
		return r;

	switch (st) {
	case CAN_STATE_ERROR_ACTIVE:
		*status = QCAN_STATE_ACTIVE;
		break;
	case CAN_STATE_ERROR_WARNING:
		*status = QCAN_STATE_WARNING;
		break;
	case CAN_STATE_ERROR_PASSIVE:
		*status = QCAN_STATE_PASSIVE;
		break;
	case CAN_STATE_BUS_OFF:
		*status = QCAN_STATE_BUS_OFF;
		break;
	case CAN_STATE_STOPPED:
		*status = QCAN_STATE_STOPPED;
		break;
	case CAN_STATE_SLEEPING:
		*status = QCAN_STATE_SLEEPING;
		break;
	default:
		*status = QCAN_STATE_UNKNOWN;
		break;
	}

	return r;
}

