/*
 * Copyright (c) 2012 Alexandros Nikolopoulos <alxnik@gmail.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.

 */

#include "CBlueTooth.h"

CBlueTooth::CBlueTooth(string device, string host)
{
	m_IsConnected = false;
	m_host = host;
	m_device = device;

	m_Socket = 0;
}

CBlueTooth::~CBlueTooth()
{
	if(m_IsConnected == false)
		Disconnect();
}

int
CBlueTooth::Connect(void)
{
	if(m_host.empty() || m_device.empty())
		return false;

	struct sockaddr_rc addr = { 0 };
	m_Socket = socket (AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

	addr.rc_family = AF_BLUETOOTH;
	addr.rc_channel = (uint8_t) 1;
	str2ba (m_host.c_str(), &addr.rc_bdaddr);

	int status = connect (m_Socket, (struct sockaddr *) &addr, sizeof (addr));
	if (status < 0)
		return false;

	m_IsConnected = true;
	return true;

}

int
CBlueTooth::Disconnect(void)
{
	close (m_Socket);
	m_IsConnected = false;
	return true;
}

int
CBlueTooth::Send(uint8_t *message, int length)
{
	int rv = write(m_Socket, message, length);

	if(rv == -1)
		return false;
	else
		return true;
}

int
CBlueTooth::Receive(uint8_t *message, int length)
{
	int rv = 0;
	while(true)
	{
		rv = read(m_Socket, message, length);

		if(rv == -1)
			return false;

		else if(rv == 0)
		{
			usleep(1000);
			continue;
		}
		else
			return true;
	}
}

