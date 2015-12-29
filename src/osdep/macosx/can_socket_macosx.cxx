#include "can_socket.h"

extern "C" {

int
can_socket_create(const char *dev)
{
	return -1;
}

int
can_socket_destroy(int fd)
{
	return -1;
}

ssize_t
can_socket_send(int fd, unsigned id, uint8_t dlc, void *data)
{
	return 0;
}

ssize_t
can_socket_recv(int fd, unsigned *id, uint8_t *dlc, void *data)
{
	return 0;
}

int
can_bitrate_set(const char *device, unsigned bitrate)
{
	return -1;
}

int
can_start(const char *device)
{
	return -1;
}

int can_stop(const char *device)
{
	return -1;
}

int can_restart(const char *device)
{
	return -1;
}

int
can_state_get(const char *device, qcan_state_t *status)
{
	return -1;
}

}

