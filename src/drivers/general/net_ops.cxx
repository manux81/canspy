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
#include "canbus/can_packet.h"
#include "drivers/net_ops.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>

#if __linux
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define closesocket close
#define INIT_SOCKET
#define CLEANUP_SOCKET
#elif _WIN32
#include <winsock2.h>
#include <io.h>
#include <stdio.h>
#define socklen_t int
#define INIT_SOCKET do { \
	WSADATA wsaData = {0};\
	WSAStartup(MAKEWORD(2, 2), &wsaData); \
	} while(0)

#define CLEANUP_SOCKET do { \
	WSACleanup();\
	} while(0)

#endif

static struct sockaddr_in server_addr;
static char server_ipstr[256];
static unsigned server_port;

static int m_fd;

static int net_create(const char *dev, unsigned bitrate);
static int net_destroy(int fd);
static int net_send(int fd, unsigned id, uint8_t dlc, void *data);
static int net_recv(int fd, unsigned *id, uint8_t *dlc, void *data,
    int64_t *sec, int64_t *usec);
static int net_bitrate_set(const char *device, unsigned bitrate);
static int net_attribute_set(unsigned attribute, const void *value, unsigned value_len);
static int net_start(const char *device);
static int net_stop(const char *device);
static int net_state_get(const char *device, qcan_state_t *status);
static int net_restart(const char *device);

can_ops_t net_ops = {
	/* .create =        */ net_create,
	/* .destroy =       */ net_destroy,
	/* .send =          */ net_send,
	/* .recv =          */ net_recv,
	/* .bitrate_set =   */ net_bitrate_set,
	/* .attribute_set = */ net_attribute_set,
	/* .start =         */ net_start,
	/* .stop =          */ net_stop,
	/* .state_get =     */ net_state_get,
	/* .restart =       */ net_restart
};

uint64_t htonll(uint64_t n)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
         return ((n & 0xffU) << 56) |
             ((n & 0xff00U) << 40) |
             ((n & 0xff0000U) << 24) |
             ((n & 0xff000000ULL) << 8) |
             ((n & 0xff00000000ULL) >> 8) |
             ((n & 0xff0000000000ULL) >> 24) |
             ((n & 0xff000000000000ULL) >> 40) |
             ((n & 0xff00000000000000ULL) >> 56);
#else
	return n;
#endif
}

int
net_create(const char *, unsigned)
{
	int skt;

	INIT_SOCKET;
	skt = socket(AF_INET, SOCK_DGRAM, 0);
	m_fd = skt;

	return skt;
}

int
net_destroy(int fd)
{
	return  closesocket(fd);
}

int
net_send(int fd, unsigned id, uint8_t dlc, void *data)
{
	can_packet_t pkt;

	if (dlc > 8)
		return -1;

	pkt.id = htonl(id);
	if (dlc != 0)
		memcpy(pkt.data, data, dlc);
	pkt.dlc = dlc;
	return sendto(fd, (char *) &pkt, sizeof(can_packet_t),0,
	    (struct sockaddr *) &server_addr, sizeof(server_addr));
}

int
net_recv(int fd, unsigned *id, uint8_t *dlc, void *data,
    int64_t *sec, int64_t *usec)
{
	can_packet_t pkt;
	socklen_t slen;
	int r;

	slen =  sizeof(server_addr);

	r = recvfrom(fd, (char *)&pkt, sizeof(can_packet_t), 0,
	    (struct sockaddr *)&server_addr, &slen);

	if (r <= 0) {
		return r;
	}
	*id = htonl(pkt.id);
	*dlc = pkt.dlc;
	memcpy(data, pkt.data, sizeof(uint8_t) * 8);
	*sec = htonll(pkt.tv_sec);
	*usec = htonll(pkt.tv_usec);

	return 1;
}

int
net_bitrate_set(const char *, unsigned)
{
	return 0;
}

int
net_attribute_set(unsigned attribute, const void *value, unsigned value_len)
{
	switch (attribute) {
	case NET_SOCKET_ADDR:
		strncpy(server_ipstr, (const char *)value, value_len);
		break;

	case NET_SOCKET_PORT:
		if (value_len != sizeof(uint16_t))
			return -1;

		server_port = *((uint16_t *)value);
		break;

	default:
		break;
	}

	return 0;
}

int
net_start(const char *)
{
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(server_ipstr);
	server_addr.sin_port = htons(server_port);
	return 0;// inet_aton(server_ipstr, &server_addr.sin_addr);
}

int
net_stop(const char *)
{
	CLEANUP_SOCKET;
	return 0;
}

int
net_restart(const char *)
{
	return 0;
}

int
net_state_get(const char *, qcan_state_t *)
{
	return 0;
}

