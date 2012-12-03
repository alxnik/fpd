/*
 * Copyright (c) 2012 Alexandros Nikolopoulos <alxnik@gmail.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include "CSocket.h"
#include "CLog.h"


CSocket::CSocket(map<string, string> settings)
{
	m_IsConnected = false;
	m_host = settings["host"];
	m_port = atoi(settings["port"].c_str());

	m_Socket = 0;
}

CSocket::~CSocket()
{
	if(m_IsConnected == true)
		Disconnect();
}

int
CSocket::Connect(void)
{
	if(m_host.empty())
		return false;

	syslog(LOG_INFO, "Connecting to %s:%d\n", m_host.c_str(), m_port);

	m_Socket = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0x00, sizeof(struct sockaddr_in));

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(m_port);

    if ((serveraddr.sin_addr.s_addr = inet_addr(m_host.c_str())) == (unsigned long) INADDR_NONE)
    {
    	struct hostent *hostp;
        hostp = gethostbyname(m_host.c_str());
        if (hostp == (struct hostent *) NULL)
        {
            close(m_Socket);
            return false;
        }

        memcpy(&serveraddr.sin_addr, hostp->h_addr, sizeof(serveraddr.sin_addr));
    }



	int status = connect (m_Socket, (struct sockaddr *) &serveraddr, sizeof (serveraddr));
	if (status < 0)
	{
		syslog(LOG_ERR, "Socket Connect error: %s\n", strerror(errno));
		return false;
	}

	m_IsConnected = true;

	return true;

}

int
CSocket::Disconnect(void)
{
	close (m_Socket);
	sleep(1);
	m_IsConnected = false;
	return true;
}

int
CSocket::Send(uint8_t *message, int length)
{
	int rv = write(m_Socket, message, length);

	if(rv == -1)
	{
		syslog(LOG_ERR, "Socket write error: %s\n", strerror(errno));
		return false;
	}
	else
		return true;
}

int
CSocket::Receive(uint8_t *message, int length, time_t timeout)
{
	int rv = 0;
	int p = 0;

	time_t StartTime = time(NULL);

	while(true)
	{
		rv = read(m_Socket, message+p, length-p);

		if(rv == -1)
		{
			syslog(LOG_ERR, "Socket read error: %s\n", strerror(errno));
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
