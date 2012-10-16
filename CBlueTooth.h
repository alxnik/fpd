/*
 * Copyright (c) 2012 Alexandros Nikolopoulos <alxnik@gmail.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.

 */

#ifndef CBLUETOOTH_H_
#define CBLUETOOTH_H_

#include "fpd.h"
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/socket.h>


class CBlueTooth {
public:
	CBlueTooth(string device, string host);
	virtual ~CBlueTooth();

	int	Connect(void);
	int Disconnect(void);

	int Send(uint8_t *message, int length);
	int Receive(uint8_t *message, int length);
private:

	string m_device;
	string m_host;
	bool m_IsConnected;

	int m_Socket;
};

#endif /* CBLUETOOTH_H_ */
