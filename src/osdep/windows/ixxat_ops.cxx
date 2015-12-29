#include "canbus/can_drv.h"
#include "utils.h"

#include <string.h>
#include <stdio.h>
#include <vcinpl.h>
#include <Windows.h>


#define FLAG_LOOPBACK    1
#define FLAG_SILENT      2
#define FLAG_DAR         4
#define FLAG_ENMSGSTATUS 8

static int m_fd;
HANDLE hDevice;
HANDLE hCanChn;
HANDLE hCanCtl;
static int64_t ofsSec = 0;
static int64_t ofsUsec = 0;
struct BaudRate {
	uint64_t baud_rate;
	uint8_t bt0;
	uint8_t bt1;
};

static struct BaudRate listBaudRates[] = {
{10000, 0x31, 0x1C},
{20000, 0x18, 0x1C},
{50000, 0x09, 0x1C},
{10000, 0x04, 0x1C},
{125000, 0x03, 0x1C},
{250000, 0x01, 0x1C},
{500000, 0x00, 0x1C},
{800000, 0x00, 0x16},
{1000000, 0x00, 0x14} };

static int ixxat_create(const char *dev, unsigned bitrate);
static int ixxat_destroy(int fd);
static int ixxat_send(int fd, unsigned id, uint8_t dlc, void *data);
static int ixxat_recv(int fd, unsigned *id, uint8_t *dlc, void *data,
	int64_t *sec, int64_t *usec);
static int ixxat_bitrate_set(const char *device, unsigned bitrate);
static int ixxat_attribute_set(unsigned attribute, const void *value, unsigned value_len);
static int ixxat_start(const char *device);
static int ixxat_stop(const char *device);
static int ixxat_state_get(const char *device, qcan_state_t *status);
static int ixxat_restart(const char *device);
static BaudRate ixxat_getbaudrate(uint64_t baund);

can_ops_t ixxat_ops = {
	ixxat_create,
	ixxat_destroy,
	ixxat_send,
	ixxat_recv,
	ixxat_bitrate_set,
	ixxat_attribute_set,
	ixxat_start,
	ixxat_stop,
	ixxat_state_get,
	ixxat_restart
};

int ixxat_create(const char *, unsigned bitrate)
{
	HANDLE      hEnum;   // enumerator handle
	VCIDEVICEINFO sInfo;   // device info
	BaudRate br;

	vciEnumDeviceOpen(&hEnum);
	vciEnumDeviceNext(hEnum, &sInfo);
	vciEnumDeviceClose(hEnum);
	vciDeviceOpen(sInfo.VciObjectId, &hDevice);

	m_fd = (int)hDevice;
	canChannelOpen(hDevice, 0, FALSE, &hCanChn);
	canChannelInitialize(hCanChn, 1024, 1, 128, 1);
	canChannelActivate(hCanChn, TRUE);
	br = ixxat_getbaudrate(bitrate);
	canControlOpen(hDevice, 0, &hCanCtl);
	canControlInitialize(hCanCtl, CAN_OPMODE_STANDARD | CAN_OPMODE_ERRFRAME,
		br.bt0, br.bt1);
	canControlSetAccFilter(hCanCtl, FALSE, CAN_ACC_CODE_ALL, CAN_ACC_MASK_ALL);

	return m_fd;
}

/*TODO: close ixxat*/
int ixxat_destroy(int)
{
	return 0;
}

int ixxat_send(int fd, unsigned id, uint8_t dlc, void *data)
{
	CANMSG msg;

	if (fd < 0)
		return -1;

	msg.dwMsgId = id;
	msg.dwTime = 0;
	msg.uMsgInfo.Bits.dlc = dlc;
	if (id & ERR_FLAG)
		msg.uMsgInfo.Bytes.bType = CAN_MSGTYPE_ERROR;
	else
		msg.uMsgInfo.Bytes.bType = CAN_MSGTYPE_DATA;
	msg.uMsgInfo.Bytes.bFlags = CAN_MAKE_MSGFLAGS(8, 0, 0, 0, 0);
	msg.uMsgInfo.Bits.srr = 0;
	memcpy(msg.abData, (char *)data, dlc);
	msg.uMsgInfo.Bits.rtr = (id & RTR_FLAG);
	msg.uMsgInfo.Bits.ext = (id & EFF_FLAG);

	canChannelPostMessage(hCanChn, &msg);

	return 0;
}

int ixxat_recv(int fd, unsigned *id, uint8_t *dlc, void *data,
	int64_t *sec, int64_t *usec)
{
	CANMSG msg;
	HRESULT ret;

	if (fd < 0)
		return -1;

	ret = canChannelReadMessage(hCanChn, INFINITE, &msg);
	if (ret != S_OK ||
	    !(msg.uMsgInfo.Bytes.bType == CAN_MSGTYPE_DATA || msg.uMsgInfo.Bytes.bType == CAN_MSGTYPE_ERROR))
		return -1;

	*id = msg.dwMsgId;
	*dlc = msg.uMsgInfo.Bits.dlc;
	memcpy((char *)data, msg.abData, *dlc);

	/*
	if (sec)
		*sec = (long)(msg.dwTime / 1000000LL);
	if (usec)
		*usec = (long)(msg.dwTime % 1000000LL);
		*/
	get_timestamp(sec, usec);

	return 8;
}

int ixxat_bitrate_set(const char *, unsigned)
{
	return 0;
}

int ixxat_attribute_set(unsigned, const void *, unsigned)
{
	return 0;
}

int ixxat_start(const char *)
{
	canControlStart(hCanCtl, TRUE);
	return 0;
}

int ixxat_stop(const char *)
{

	return 0;
}

int ixxat_state_get(const char *, qcan_state_t *status)
{
	HRESULT r;
	CANCHANSTATUS stat;

	*status = QCAN_STATE_UNKNOWN;

	r = canChannelGetStatus(hCanChn, &stat);
	if (r != S_OK)
		return -1;

	if (stat.fActivated) {
		*status = QCAN_STATE_ACTIVE;
		goto out;
	}
	if (stat.sLineStatus.dwStatus & CAN_STATUS_OVRRUN) {
		*status = QCAN_STATE_WARNING;
		goto out;
	}
	if (stat.sLineStatus.dwStatus & CAN_STATUS_ERRLIM) {
		*status = QCAN_STATE_PASSIVE;
		goto out;
	}
	if (stat.sLineStatus.dwStatus & CAN_STATUS_BUSOFF) {
		*status = QCAN_STATE_BUS_OFF;
		goto out;
	}
	if (stat.sLineStatus.dwStatus & CAN_STATUS_ININIT) {
		*status = QCAN_STATE_STOPPED;
		goto out;
	}
	if (0) {
		*status = QCAN_STATE_SLEEPING;
		goto out;
	}
	*status = QCAN_STATE_UNKNOWN;

out:
	return 0;
}

int ixxat_restart(const char *)
{
	return 0;
}

BaudRate ixxat_getbaudrate(uint64_t baund)
{
	unsigned i;
	BaudRate ret = { 0, 0, 0 };
	for (i = 0u; i < 9; i++) {
		if (baund == listBaudRates[i].baud_rate)
			return listBaudRates[i];
	}
	return ret;
}
