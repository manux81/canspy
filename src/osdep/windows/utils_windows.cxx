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

#include "drivers/usb2can_ops.h"
#include "drivers/ixxat_ops.h"
#include "drivers/net_ops.h"
#include "canbus/simulation_ops.h"
#include "utils.h"

#include <winsock2.h>
#include <stdint.h>

inline int gettimeofday(struct timeval* tp, void*)
{
	union {
		long long ns100; /*time since 1 Jan 1601 in 100ns units */
		FILETIME ft;
	} now;

	GetSystemTimeAsFileTime(&(now.ft));
	tp->tv_usec = (long)((now.ns100 / 10LL) % 1000000LL);
	tp->tv_sec = (long)((now.ns100 - (116444736000000000LL)) / 10000000LL);
	return 0;
}

int get_timestamp(int64_t *sec, int64_t *usec)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	if (sec)
		*sec = tv.tv_sec;

	if (usec)
		*usec = tv.tv_usec;

	return 0;
}

can_ops_t *get_can_ops(const char *name)
{
	can_ops_t *ret = NULL;

	if (!strcmp("USB2CAN 8Devices", name))
		ret = &usb2can_ops;
	if (!strcmp("IXXAT USB", name))
		ret = &ixxat_ops;
	if (!strcmp("CAN Over TCP", name))
		ret = &net_ops;
	if (!strcmp("Simulation", name))
		ret = &simulation_ops;


	return ret;
}

void usleep(__int64 usec)
{
	HANDLE timer;
	LARGE_INTEGER ft;

	ft.QuadPart = -(10 * usec); // Convert to 100 nanosecond interval, negative value indicates relative time

	timer = CreateWaitableTimer(NULL, TRUE, NULL);
	SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
	WaitForSingleObject(timer, INFINITE);
	CloseHandle(timer);
}

char *
strsep(char **stringp, const char *delim)
{
	char *begin = NULL, *end = NULL;

	begin = *stringp;
	if (begin == NULL)
		return NULL;

	/* A frequent case is when the delimiter string contains only one
	character.  Here we don't need to call the expensive `strpbrk'
	function and instead work using `strchr'.  */
	if (delim[0] == '\0' || delim[1] == '\0') {
		char ch = delim[0];

		if (ch == '\0')
			end = NULL;
		else {
			if (*begin == ch)
				end = begin;
			else if (*begin == '\0')
				end = NULL;
			else
				end = strchr(begin + 1, ch);
		}
	}
	else
		/* Find the end of the token.  */
		end = strpbrk(begin, delim);

	if (end) {
		/* Terminate the token and set *STRINGP past NUL character.  */
		*end++ = '\0';
		*stringp = end;
	}
	else
		/* No more delimiters; this is the last token.  */
		* stringp = NULL;

	return begin;

}

