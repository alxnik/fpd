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

	pthread_mutex_init(&m_SocketMutex, NULL);
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

	pthread_mutex_lock(&m_SocketMutex);
	if(m_IsConnected == true)
	{
		pthread_mutex_unlock(&m_SocketMutex);
		return true;
	}

	Log.log("Connecting to %s:%d\n", m_host.c_str(), m_port);

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
            pthread_mutex_unlock(&m_SocketMutex);
            return false;
        }

        memcpy(&serveraddr.sin_addr, hostp->h_addr, sizeof(serveraddr.sin_addr));
    }



	int status = connect (m_Socket, (struct sockaddr *) &serveraddr, sizeof (serveraddr));
	if (status < 0)
	{
		Log.error("Socket Connect error: %s\n", strerror(errno));
		close(m_Socket);
		m_Socket = 0;
		pthread_mutex_unlock(&m_SocketMutex);
		return false;
	}

	Log.log("Connected\n");

	m_IsConnected = true;

	pthread_mutex_unlock(&m_SocketMutex);
	return true;

}

int
CSocket::ReConnect(void)
{
	if(m_IsConnected)
		Disconnect();

	int rv = Connect();

	// In order to not spam the server with connection requests,
	// if a reconnection fails, wait for a few seconds
	if(rv == false)
		sleep(30);

	return rv;
}

int
CSocket::Disconnect(void)
{
	pthread_mutex_lock(&m_SocketMutex);

	close (m_Socket);
	m_Socket = 0;
	sleep(1);
	m_IsConnected = false;

	pthread_mutex_unlock(&m_SocketMutex);
	return true;
}

int
CSocket::Send(uint8_t *message, int length)
{

	if(m_IsConnected == false && ReConnect() == false)
		return false;

	int rv = write(m_Socket, message, length);

	if(rv == -1)
	{

		Log.error("Socket write error: %s [%d]\n", strerror(errno), errno);
		Log.log("Reconnecting\n");
		return ReConnect();
	}
	else
		return true;
}

int
CSocket::Receive(uint8_t *message, int length, time_t timeout)
{
	if(m_IsConnected == false && ReConnect() == false)
		return false;

	int rv = 0;
	int p = 0;

	time_t StartTime = time(NULL);

	while(true)
	{
		rv = read(m_Socket, message+p, length-p);

		if(rv == -1)
		{
			Log.error("Socket read error: %s [%d]\n", strerror(errno), errno);
			Log.log("Reconnecting\n");
			ReConnect();
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
