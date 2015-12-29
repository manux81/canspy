#include "canbus/can_drv.h"
#include "osdep/canal.h"
#include "utils.h"

#include <string.h>
#include <stdio.h>
#include <Windows.h>


#define FLAG_LOOPBACK    1
#define FLAG_SILENT      2
#define FLAG_DAR         4
#define FLAG_ENMSGSTATUS 8

static int m_fd;
static int64_t ofsSec = 0;
static int64_t ofsUsec = 0;

static int usb2can_create(const char *dev, unsigned bitrate);
static int usb2can_destroy(int fd);
static int usb2can_send(int fd, unsigned id, uint8_t dlc, void *data);
static int usb2can_recv(int fd, unsigned *id, uint8_t *dlc, void *data,
	int64_t *sec, int64_t *usec);
static int usb2can_bitrate_set(const char *device, unsigned bitrate);
static int usb2can_attribute_set(unsigned attribute, const void *value, unsigned value_len);
static int usb2can_start(const char *device);
static int usb2can_stop(const char *device);
static int usb2can_state_get(const char *device, qcan_state_t *status);
static int usb2can_restart(const char *device);


can_ops_t usb2can_ops = {
	usb2can_create,
	usb2can_destroy,
	usb2can_send,
	usb2can_recv,
	usb2can_bitrate_set,
	usb2can_attribute_set,
	usb2can_start,
	usb2can_stop,
	usb2can_state_get,
	usb2can_restart
};


int usb2can_create(const char *dev, unsigned bitrate)
{
	char cntstr[128];
	int handle;

	sprintf(cntstr, "%s ; %d", dev, bitrate / 1000);
	handle = CanalOpen(cntstr, FLAG_ENMSGSTATUS);
	if (handle <= 0) {
		m_fd = -1;
		return -1;
	}
	m_fd = handle;

	return handle;
}

int usb2can_destroy(int fd)
{
	return CanalClose(fd);
}

int usb2can_send(int fd, unsigned id, uint8_t dlc, void *data)
{
	canalMsg msg;
	int ret;

	if (fd < 0)
		return -1;

	msg.id = id & EFF_MASK;
	msg.flags = ((id & RTR_FLAG) >> 29) |
	    ((id & EFF_FLAG) >> 31) | ((id  & ERR_FLAG) >> 28);
	msg.sizeData = dlc;
	memcpy(msg.data, (char *)data, dlc);
	ret = CanalSend(fd, &msg);

	return ret;
}

int usb2can_recv(int fd, unsigned *id, uint8_t *dlc, void *data,
    int64_t *sec, int64_t *usec)
{
	canalMsg msg;
	int ret;

	if (fd < 0)
		return -1;

	while (CanalDataAvailable(fd) == 0) {
		Sleep(1);
	}

	ret = CanalReceive(fd, &msg);

	if (ret < 0)
		return ret;

	if (msg.sizeData > 8)
		return -1;

	*id = msg.id | ((msg.flags & 0x2) << 29) |
		((msg.flags & 0x1) << 31) | ((msg.flags & 0x4) << 27);

	*dlc = msg.sizeData;
	memcpy((char *)data, msg.data, msg.sizeData);

	get_timestamp(sec, usec);

	return msg.sizeData;
}

int usb2can_bitrate_set(const char *, unsigned bitrate)
{
	int ret;
	int baudrate;

	if (m_fd < 0)
		return -1;

	switch (bitrate) {
	case 10000:
		baudrate = CANAL_BAUD_10;
		break;

	case 20000:
		baudrate = CANAL_BAUD_20;
		break;

	case 50000:
		baudrate = CANAL_BAUD_50;
		break;

	case 100000:
		baudrate = CANAL_BAUD_100;
		break;

	case 125000:
		baudrate = CANAL_BAUD_125;
		break;

	case 250000:
		baudrate = CANAL_BAUD_250;
		break;

	case 500000:
		baudrate = CANAL_BAUD_500;
		break;

	case 800000:
		baudrate = CANAL_BAUD_800;
		break;

	case 1000000:
		baudrate = CANAL_BAUD_1000;
		break;

	default:
		baudrate = CANAL_BAUD_USER;
		break;
	}

	ret = CanalSetBaudrate(m_fd, baudrate);

	return ret;
}

int usb2can_attribute_set(unsigned, const void *, unsigned)
{
	return 0;
}

int usb2can_start(const char *)
{
	return 0;
}

int usb2can_stop(const char *)
{

	return 0;
}

int usb2can_state_get(const char *, qcan_state_t *status)
{
	int r;
	canalStatus stat;

	return 0;
	r = 0;
	*status = QCAN_STATE_UNKNOWN;

	r = CanalGetStatus(m_fd, &stat);
	if (r != CANAL_ERROR_SUCCESS)
		return r;

	if (! stat.channel_status|| stat.channel_status & CANAL_STATUS_ACTIVE) {
		*status = QCAN_STATE_ACTIVE;
		goto out;
	}
	if (stat.channel_status & CANAL_STATUS_BUS_WARN) {
		*status = QCAN_STATE_WARNING;
		goto out;
	}
	if (stat.channel_status & CANAL_STATUS_PASSIVE) {
		*status = QCAN_STATE_PASSIVE;
		goto out;
	}
	if (stat.channel_status & CANAL_STATUS_BUS_OFF) {
		*status = QCAN_STATE_BUS_OFF;
		goto out;
	}
	if (stat.channel_status & CANAL_STATUS_STOPPED) {
		*status = QCAN_STATE_STOPPED;
		goto out;
	}
	if (stat.channel_status & CANAL_STATUS_SLEEPING) {
		*status = QCAN_STATE_SLEEPING;
		goto out;
	}
	*status = QCAN_STATE_UNKNOWN;

out:
	return r;
}

int usb2can_restart(const char *)
{
	return 0;
}
