/*
 * Copyright (c) 2012 Alexandros Nikolopoulos <alxnik@gmail.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.

 */

#ifndef CSOCKET_H_
#define CSOCKET_H_

#include "fpd.h"

#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>



class CSocket : public CInterface
{
public:
	CSocket(map<string, string> settings);
	virtual ~CSocket();

	int				Connect(void);
	int 			Disconnect(void);

	int 			Send(uint8_t *message, int length);
	int 			Receive(uint8_t *message, int length, time_t timeout);

	// FIXME Add a function to this
	string			GetMyAddress(void) { return "";};
private:

	int 			ReConnect(void);

	string 		m_host;
	uint16_t	m_port;
	bool 		m_IsConnected;
	int 		m_Socket;
};

#endif /* CSOCKET_H_ */
