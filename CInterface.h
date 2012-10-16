/*
 * Copyright (c) 2012 Alexandros Nikolopoulos <alxnik@gmail.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.

 */

#ifndef CINTERFACE_H_
#define CINTERFACE_H_

#include "fpd.h"

#include <iostream>
#include <string>
using namespace std;

class CInterface {
public:
	CInterface(string SendingDev, speed_t SendSpeed);
	CInterface(string SendingDev, speed_t SendSpeed, string ReceivingDev, speed_t ReceiveSpeed);

	virtual ~CInterface();

	int	Connect(void);
	int Disconnect(void);

	int Send(uint8_t *message, int length);
	int Receive(uint8_t *message, int length);

private:
	int Lock(string device);
	int UnLock(string device);
	int SetAttributes(int FileDescriptor, speed_t speed);

	bool m_IsConnected;
	string m_SendingDevName;
	int m_SendingDev;
	speed_t m_SendingSpeed;

	string m_ReceivingDevName;
	int m_ReceivingDev;
	speed_t m_ReceivingSpeed;
};

#endif /* CINTERFACE_H_ */
