/*
 * Copyright (c) 2012 Alexandros Nikolopoulos <alxnik@gmail.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.

 */

#ifndef CMODBUSMTU_H_
#define CMODBUSMTU_H_
#include "fpd.h"
#include "CInterface.h"

#include "libmodbus/modbus.h"
#include <errno.h>

#include <iostream>
#include <string>
#include <map>
using namespace std;


class CModbusMTU
{
public:
	CModbusMTU(string SendingDev, speed_t SendSpeed);
	virtual ~CModbusMTU();

	int Connect(void);
	int Disconnect(void);

	int ReadInputReg(int InverterId, int Address, int Length, map<uint16_t,uint16_t> &ResultSet);
	int SetTimeout(int usec);

private:
	int speed_t2int(speed_t inSpeed);

	modbus_t *m_ModFace;
	string m_DevName;
	speed_t m_Speed;
	bool m_IsConnected;
};

#endif /* CMODBUSMTU_H_ */
