/*
 * Copyright (c) 2012 Alexandros Nikolopoulos <alxnik@gmail.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.

 */

#include "CBlueTooth.h"
#include "CLog.h"


CBlueTooth::CBlueTooth(map<string, string> settings)
{
	m_IsConnected = false;
	m_host = settings["host"];

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
	if(m_host.empty())
		return false;

	struct sockaddr_rc addr = { 0 };
	m_Socket = socket (AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

	addr.rc_family = AF_BLUETOOTH;
	addr.rc_channel = (uint8_t) 1;
	str2ba (m_host.c_str(), &addr.rc_bdaddr);

	sleep(3);
	int status = connect (m_Socket, (struct sockaddr *) &addr, sizeof (addr));
	if (status < 0)
	{
		syslog(LOG_ERR, "BlueTooth Connect error: %s\n", strerror(errno));
		return false;
	}

	m_IsConnected = true;

	/* Get my Mac */
	struct sockaddr_rc mymac = { 0 };
	unsigned int mymac_size = sizeof(mymac);
	getsockname(m_Socket,(struct sockaddr *) &mymac, &mymac_size);
	baswap(&m_LocalAddr, &mymac.rc_bdaddr);

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
	Log.hexDump(LOG_DEBUG, message, length);
	int rv = write(m_Socket, message, length);

	if(rv == -1)
	{
		syslog(LOG_ERR, "BlueTooth write error: %s\n", strerror(errno));
		return false;
	}
	else
		return true;
}

int
CBlueTooth::Receive(uint8_t *message, int length, time_t timeout)
{
	int rv = 0;
	int p = 0;

	time_t StartTime = time(NULL);

	while(true)
	{
		rv = read(m_Socket, message+p, length-p);

		if(rv == -1)
		{
			syslog(LOG_ERR, "BlueTooth read error: %s\n", strerror(errno));
			return rv;
		}
		else if(rv == 0 && p == 0)
		{
			usleep(1000);
			continue;
		}
		else
		{
			p += rv;
			if(time(NULL) >= StartTime || p == length)
				return rv;
		}
	}

	return rv;
}

string
CBlueTooth::GetMyAddress(void)
{
	//return &m_LocalAddr;
}

