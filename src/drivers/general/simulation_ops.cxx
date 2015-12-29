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
#include "drivers/simulation_ops.h"
#include "utils.h"
#include "os_utils.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static int m_fd;

static int simulation_create(const char *dev, unsigned bitrate);
static int simulation_destroy(int fd);
static int simulation_send(int fd, unsigned id, uint8_t dlc, void *data);
static int simulation_recv(int fd, unsigned *id, uint8_t *dlc, void *data,
                           int64_t *sec, int64_t *usec);
static int simulation_bitrate_set(const char *device, unsigned bitrate);
static int simulation_attribute_set(unsigned attribute, const void *value, unsigned value_len);
static int simulation_start(const char *device);
static int simulation_stop(const char *device);
static int simulation_state_get(const char *device, qcan_state_t *status);
static int simulation_restart(const char *device);
static void reverse(char s[]);
static int read_line(int fd, char *buf, int size);
static int parse_dump_line(int fd, uint32_t *id, uint8_t *dlc, uint8_t data[]);

can_ops_t simulation_ops = {
	/* .create =        */ simulation_create,
	/* .destroy =       */ simulation_destroy,
	/* .send =          */ simulation_send,
	/* .recv =          */ simulation_recv,
	/* .bitrate_set =   */ simulation_bitrate_set,
	/* .attribute_set = */ simulation_attribute_set,
	/* .start =         */ simulation_start,
	/* .stop =          */ simulation_stop,
	/* .state_get =     */ simulation_state_get,
	/* .restart =       */ simulation_restart
};

int
simulation_create(const char *dev, unsigned)
{
	m_fd = open(dev, O_RDONLY | _O_BINARY);

	return m_fd;
}

int
simulation_destroy(int fd)
{
	return  close(fd);
}

int
simulation_send(int, unsigned, uint8_t, void *)
{
	return 0U;
}

int
simulation_recv(int fd, unsigned *id, uint8_t *dlc, void *data,
                int64_t *sec, int64_t *usec)
{
	int result = 1;

	if (fd < 0) {
		result = -1;
		goto out;
	}
	if (parse_dump_line(fd, id, dlc, (uint8_t *)data) < 0) {
		result = -1;
		goto out;
	}

	if (*dlc > 8) {
		result = -1;
		goto out;
	}
	get_timestamp(sec, usec);

out:
	return result;
}

int
simulation_bitrate_set(const char *, unsigned)
{
	return 0;
}

int
simulation_attribute_set(unsigned, const void *, unsigned)
{
	return 0;
}

int
simulation_start(const char *)
{
	int ret = lseek(m_fd, -2, SEEK_END);
	if (ret < 0)
		return -1;

	return 0;
}

int
simulation_stop(const char *)
{
	return 0;
}

int
simulation_restart(const char *)
{
	return 0;
}

int
simulation_state_get(const char *, qcan_state_t *)
{
	return 0;
}

void reverse(char s[])
{
	int length = strlen(s);
	int c, i, j;

	for (i = 0, j = length - 2; i < j; i++, j--) {
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}

int
read_line(int fd, char *buf, int size)
{
	char c = 0;
	int i = 0;
	int64_t pos;

	while (i < size) {
		if (read(fd, &c, 1) != 1)
			break;
		pos = lseek(fd, -2, SEEK_CUR);
		buf[i++] = c;
		if (c == '\n')
			break;
		if (pos == 0) {
			read(fd, &c, 1);
			buf[i++] = c;
			buf[i++] = '\n';
			lseek(fd, -1, SEEK_CUR);
			break;
		}
	}
	if (i < 1024)
		buf[i] = 0;
	reverse(buf);
	return i;
}

int
parse_dump_line(int fd, uint32_t *id, uint8_t *dlc, uint8_t data[])
{
	char *ptr = NULL, *token = NULL, buffer[1024];
	int column = 0;
	int ret = 0;
	uint64_t delay = 100;

	if (lseek(fd, 0, SEEK_CUR) == 0) {
		ret = -1;
		goto out;
	}

	if (read_line(fd, buffer, 1024) <= 0) {
		ret = -2;
		goto out;
	}

	if ((*buffer == '\n') || !strcmp(buffer, "\r\n")) {
		ret = -1;
		goto out;
	}


	*id  = 0U;
	*dlc = 0U;
	buffer[1023] = '\0';

	ptr = buffer;
	do {
		if (!ptr) {
			break;
		}
		if ((*ptr == '\n') || !strcmp(ptr, "\r\n")) {
			break;
		}
		token = strsep(&ptr, " ");
		if (token && (strnlen(token, 1024U) > 1)) {
			column++;

			switch (column) {
			case 1:
				*id = strtoul(token, NULL, 16);
				break;

			case 2:
				*dlc = (uint8_t)atoi(token + 1);
				break;

			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
				if (column <= *dlc + 2)
					data[column-3] = strtoul(token, NULL, 16);
				break;

			default:
				if (column == *dlc + 5)
					delay = atoi(token);
				break;
			}
		}
	} while (token != NULL);

out:
	usleep(delay * 1000);
	return ret;
}



