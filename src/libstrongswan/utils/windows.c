/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include "utils.h"

#include <errno.h>

/**
 * See header
 */
void windows_init()
{
	WSADATA wsad;

	/* initialize winsock2 */
	WSAStartup(MAKEWORD(2, 2), &wsad);
}

/**
 * See header
 */
void windows_deinit()
{
	WSACleanup();
}

/**
 * See header
 */
int socketpair(int domain, int type, int protocol, int sv[2])
{
	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_addr.s_addr = htonl(INADDR_LOOPBACK),
	};
	socklen_t len = sizeof(addr);
	int s, c, sc;
	BOOL on;

	/* We don't check domain for AF_INET, as we use it as replacement for
	 * AF_UNIX. */
	if (type != SOCK_STREAM)
	{
		errno = EINVAL;
		return -1;
	}
	if (protocol != 0 && protocol != IPPROTO_TCP)
	{
		errno = EINVAL;
		return -1;
	}
	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == -1)
	{
		return -1;
	}
	c = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (c == -1)
	{
		closesocket(c);
		return -1;
	}
	if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) == 0 &&
		getsockname(s,(struct sockaddr*)&addr, &len) == 0 &&
		listen(s, 0) == 0 &&
		connect(c, (struct sockaddr*)&addr, sizeof(addr)) == 0)
	{
		sc = accept(s, NULL, NULL);
		if (sc > 0)
		{
			closesocket(s);
			s = sc;
			if (setsockopt(s, IPPROTO_TCP, TCP_NODELAY,
						   (void*)&on, sizeof(on)) == 0 &&
				setsockopt(c, IPPROTO_TCP, TCP_NODELAY,
						   (void*)&on, sizeof(on)) == 0)
			{
				sv[0] = s;
				sv[1] = c;
				return 0;
			}
		}
	}
	closesocket(s);
	closesocket(c);
	return -1;
}
