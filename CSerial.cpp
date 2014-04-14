/*
 * Copyright (c) 2012 Alexandros Nikolopoulos <alxnik@gmail.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.

 */

#include "CSerial.h"

CSerial::CSerial(map<string,string>	settings)
{
	m_SendingDev = m_ReceivingDev = 0;
	m_IsConnected = false;


	m_SendingDevName = settings["port"];
	m_SendingSpeed = int2speed(atoi(settings["speed"].c_str()));

	if(settings["port2"].length() > 0 && settings["speed2"].length() > 0)
	{
		m_ReceivingDevName = settings["port2"];
		m_ReceivingSpeed = int2speed(atoi(settings["speed2"].c_str()));
	}
	else
	{
		m_ReceivingDevName = settings["port"];
		m_ReceivingSpeed = int2speed(atoi(settings["speed"].c_str()));
	}
}

CSerial::CSerial(string SendingDev, speed_t SendSpeed, string ReceivingDev, speed_t ReceiveSpeed)
{
	m_SendingDev = m_ReceivingDev = 0;
	m_IsConnected = false;

	m_SendingDevName = SendingDev;
	m_SendingSpeed = SendSpeed;

	m_ReceivingDevName = ReceivingDev;
	m_ReceivingSpeed = ReceiveSpeed;
}

CSerial::CSerial(string SendingDev, speed_t SendSpeed)
{
	m_SendingDev = m_ReceivingDev = 0;
	m_IsConnected = false;

	m_SendingDevName = SendingDev;
	m_ReceivingDevName = SendingDev;

	m_SendingSpeed = SendSpeed;
	m_ReceivingSpeed = SendSpeed;
}

speed_t
CSerial::int2speed(int speed)
{
	switch(speed)
	{
		case 50:
			return B50;
		case 75:
			return B75;
		case 110:
			return B110;
		case 134:
			return B134;
		case 150:
			return B150;
		case 200:
			return B200;
		case 300:
			return B300;
		case 600:
			return B600;
		case 1200:
			return B1200;
		case 1800:
			return B1800;
		case 2400:
			return B2400;
		case 4800:
			return B4800;
		case 9600:
			return B9600;
		case 19200:
			return B19200;
		case 38400:
			return B38400;
	}
	return 0;
}

CSerial::~CSerial()
{
	Disconnect();
}

int
CSerial::Connect(void)
{

	if(m_SendingDevName.empty() || m_ReceivingDevName.empty())
		return false;

	if(!Lock(m_SendingDevName))
		return false;

	m_SendingDev = open(m_SendingDevName.c_str(), O_RDWR | O_NOCTTY);

	if(m_SendingDev < 0 || SetAttributes(m_SendingDev, m_SendingSpeed))
	{
		UnLock(m_SendingDevName);
		return false;
	}


	if(m_SendingDevName != m_ReceivingDevName)
	{
		if(!Lock(m_ReceivingDevName))
		{
			UnLock(m_SendingDevName);
			return false;
		}

		m_ReceivingDev = open(m_ReceivingDevName.c_str(), O_RDWR | O_NOCTTY);

		if(m_ReceivingDev < 0 || SetAttributes(m_ReceivingDev, m_ReceivingSpeed))
		{
			UnLock(m_SendingDevName);
			UnLock(m_ReceivingDevName);
			return false;
		}

	}
	else
		m_ReceivingDev = m_SendingDev;

	m_IsConnected = true;
	return true;
}

int
CSerial::Disconnect(void)
{
	if(m_IsConnected)
	{
		close(m_SendingDev);
		UnLock(m_SendingDevName);

		if(m_SendingDevName != m_ReceivingDevName)
		{
			close(m_ReceivingDev);
			UnLock(m_ReceivingDevName);
		}
		m_IsConnected = false;
	}
	return true;
}

int
CSerial::Lock(string device)
{
	string filename;

	filename = "fpd" + device + ".lck";

	for(unsigned int i=0; i<filename.length(); i++)
		if(filename[i] == '/')
			filename[i] = '.';

	string fullpath = "/tmp/" + filename;

	if(!access(fullpath.c_str(), R_OK))
		return false;

	FILE *lockFile;
	if ( (lockFile = fopen(fullpath.c_str(), "w" )) )
	{
		fclose(lockFile);
		return true;
	}

	return false;
}

int
CSerial::UnLock(string device)
{
	string filename;

	filename = "fpd" + device + ".lck";

	for(unsigned int i=0; i<filename.length(); i++)
		if(filename[i] == '/')
			filename[i] = '.';

	string fullpath = "/tmp/" + filename;

	unlink(fullpath.c_str());
	return true;
}

int
CSerial::SetAttributes(int FileDescriptor, speed_t speed)
{
    struct termios serialOptions;
    tcgetattr(FileDescriptor,&serialOptions); /* save current port settings */

    serialOptions.c_cflag = speed | CS8 | CLOCAL | CREAD;
    serialOptions.c_oflag = 0;
    serialOptions.c_iflag = 0;
    serialOptions.c_lflag = 0;
    serialOptions.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    serialOptions.c_cc[VMIN]     = 1;   /* blocking read until 1 char received */
    tcflush(FileDescriptor, TCIFLUSH);
    return(tcsetattr(FileDescriptor,TCSANOW,&serialOptions));
}

int
CSerial::Send(uint8_t *message, int length)
{
	int rv = write(m_SendingDev, message, length);

	if(rv == -1)
		return false;
	else
		return true;
}

int
CSerial::Receive(uint8_t *message, int length, time_t timeout)
{
	int rv = 0;
	while(true)
	{
		rv = read(m_ReceivingDev, message, length);

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
	return false;
}
