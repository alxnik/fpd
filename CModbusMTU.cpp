/*
 * Copyright (c) 2012 Alexandros Nikolopoulos <alxnik@gmail.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.

 */

#include "CModbusMTU.h"

CModbusMTU::CModbusMTU(string SendingDev, speed_t SendSpeed)
{
	m_ModFace = 0;
	m_IsConnected = false;

	m_DevName = SendingDev;
	m_Speed = SendSpeed;

}

int
CModbusMTU::Connect(void)
{
	if(!m_ModFace)
	{
		// TODO: Make it not be 8N1 by default, but who the fuck cares...
		m_ModFace = modbus_new_rtu(m_DevName.c_str(), speed_t2int(m_Speed), 'N', 8, 1);

		if(!m_ModFace)
			return false;

        if (modbus_connect(m_ModFace) == -1)
        {
                syslog(LOG_ERR, "RTU Connection failed: %s\n", modbus_strerror(errno));
                modbus_free(m_ModFace);
                m_ModFace = 0;
                return false;
        }
        m_IsConnected = true;

        // Set the response timeout by default to 3 secs
        struct timeval response_timeout;
        response_timeout.tv_sec = 3;
        response_timeout.tv_usec = 0;
        modbus_set_response_timeout(m_ModFace, &response_timeout);
        response_timeout.tv_sec = 0;
        response_timeout.tv_usec = 10000;
        modbus_set_byte_timeout(m_ModFace, &response_timeout);
        //modbus_set_debug(m_ModFace, true);
	}
	return true;
}

int
CModbusMTU::SetTimeout(int usec)
{

    struct timeval response_timeout;
    response_timeout.tv_sec = usec/1000000;
    response_timeout.tv_usec = usec%1000000;
    modbus_set_response_timeout(m_ModFace, &response_timeout);
    return true;
}
int
CModbusMTU::Disconnect(void)
{
	if(m_ModFace)
	{
		if(m_IsConnected)
			modbus_close(m_ModFace);

        modbus_free(m_ModFace);
	}
	return true;
}

CModbusMTU::~CModbusMTU()
{
	Disconnect();
}

int
CModbusMTU::ReadInputReg(int InverterId, int Address, int Length, map<uint16_t,uint16_t> &ResultSet)
{
	if(!m_ModFace || !m_IsConnected)
		return false;

	//modbus_set_debug(m_ModFace, true);

	modbus_set_slave(m_ModFace, InverterId);

	uint16_t *out;
	out = (uint16_t *) malloc(Length * sizeof(uint16_t));
	memset(out, 0x0, Length * sizeof(uint16_t));

    int rv = modbus_read_input_registers(m_ModFace, Address-1, Length, out);

    if(rv == -1)
    {
    	free(out);
    	syslog(LOG_ERR, "RTU Read Input Registers Failed: %s\n", modbus_strerror(errno));
    	return false;
    }
    else
    {
    	for(int i = Address; i<=Address + Length; i++)
    	{
    		ResultSet[i] = *(out + i - Address);
    	}
    	free(out);
    	return true;
    }
}

int
CModbusMTU::speed_t2int(speed_t inSpeed)
{
	switch(inSpeed)
	{
		case B1200:
			return 1200;
		case B2400:
			return 2400;
		case B4800:
			return 4800;
		case B9600:
			return 9600;
		case B19200:
			return 19200;
		case B38400:
			return 38400;
		case B57600:
			return 57600;
		case B115200:
			return 115200;
	}
	return -1;
}
