/*
 * Copyright (c) 2012 Alexandros Nikolopoulos <alxnik@gmail.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include "CSolutronicProbe.h"
#include "errno.h"
#include "fpd.h"


CSolutronicProbe::CSolutronicProbe(CInterface *Iface, string uuid)
{
	m_Interface = Iface;
	m_SendingThread = m_ReceivingThread = 0;
	m_Uuid = uuid;

	pthread_mutex_init(&m_queueMutex, NULL);
}

CSolutronicProbe::CSolutronicProbe(CInterface *Iface, list<int> sensors, string uuid)
{
	m_Interface = Iface;
	m_SendingThread = m_ReceivingThread = 0;
	m_Uuid = uuid;

	m_connected = sensors;

	pthread_mutex_init(&m_queueMutex, NULL);
}

CSolutronicProbe::~CSolutronicProbe()
{
	Stop();
	delete(m_Interface);
}

bool
CSolutronicProbe::probeInverters(void)
{
	// Send all the messages
	//for(uint16_t i = 0; i< 0xffff; i++)
	for(uint16_t i =  36820; i<  36840; i++)
	{
		SendMessage(m_Interface, 0x0000, i, 148, READ, (uint32_t) 0x0);
		if(SigTermFlag == true)
			return 0;
		usleep(200000);
	}

	// Check for any replies
	int j = 0;
	pthread_mutex_lock(&m_queueMutex);
	std::list<struct MsgStruct>::iterator i = m_MsgQueue.begin();
	while (i != m_MsgQueue.end())
	{
			uint16_t InvNum = (uint16_t)(i->Data[3] << 8 | i->Data[4]);
			m_connected.push_back(InvNum);

			i = m_MsgQueue.erase(i);
			j++;
	}
	pthread_mutex_unlock(&m_queueMutex);

	if(j > 0)
	{
		if(j == 1) // :)
			syslog(LOG_INFO, "Found %d inverter\n", j);
		else
			syslog(LOG_INFO, "Found %d inverters\n", j);

		return true;
	}
	return false;
}

list<int>
CSolutronicProbe::GetConnectedInverters(void)
{
	return m_connected;
}

int
CSolutronicProbe::ResetStack(void)
{
	std::list<struct MsgStruct>::iterator i = m_MsgQueue.begin();
	pthread_mutex_lock(&m_queueMutex);

	while (i != m_MsgQueue.end())
	{
		i = m_MsgQueue.erase(i);
		i++;
	}
	pthread_mutex_unlock(&m_queueMutex);

	return true;
}

int
CSolutronicProbe::Start(void)
{
	struct ThreadStruct RecArgs;

	RecArgs.interface = m_Interface;
	RecArgs.queueMutex = &m_queueMutex;
	RecArgs.MsgQueue = &m_MsgQueue;

	if(!m_ReceivingThread)
		pthread_create( &m_ReceivingThread, NULL, &CSolutronicProbe::ReceivingFunction,  &RecArgs);

	usleep(100000);
	// Check for inverters. If none is found, stop the process
//	if(probeInverters() == false)
//	{
//		Stop();
//		return false;
//	}


	// All is well.
	// Start the main sending thread
	struct ThreadStruct args;
	args.interface = m_Interface;
	args.connected = &m_connected;

	if(!m_SendingThread)
		pthread_create( &m_SendingThread, NULL, &CSolutronicProbe::SendingFunction,  &args);
	sleep(1);
	return true;
}

int
CSolutronicProbe::Stop(void)
{
	if(m_SendingThread)
	{
		pthread_cancel(m_SendingThread);
		m_SendingThread = 0;
	}
	if(m_ReceivingThread)
	{
		pthread_cancel(m_ReceivingThread);
		m_ReceivingThread = 0;
	}
	return true;
}

int
CSolutronicProbe::GetAverage(DataContainer &AverageData)
{
	int inverterId = atoi(AverageData["inverter"].c_str());

	bool rv = true;

	if(m_MsgQueue.size() == 0)
	{
		syslog(LOG_ERR, "No messages in Queue\n");
		return false;
	}

	uint32_t u32;

	if(RetreiveFromStack(inverterId, SOL_CMD_VOLTAGE_AC, &u32) == false)
		rv = false;
	else
		AverageData["voltageAC"] = int2string(u32);

	if(RetreiveFromStack(inverterId, SOL_CMD_VOLTAGE_DC, &u32) == false)
		rv = false;
	else
		AverageData["voltageDC"] = int2string(u32);

	if(RetreiveFromStack(inverterId, SOL_CMD_CURRENT_AC, &u32) == false)
		rv = false;
	else
		AverageData["currentAC"] = int2string(u32);

	if(RetreiveFromStack(inverterId, SOL_CMD_CURRENT_DC, &u32) == false)
		rv = false;
	else
		AverageData["currentDC"] = int2string(u32);

//	if(RetreiveFromStack(inverterId, SOL_CMD_POWER_AC, &u32) == false)
//		rv = false;
//	else
//		AverageData["power"] = int2string(u32);

	if(RetreiveFromStack(inverterId, SOL_CMD_POWER_DC, &u32) == false)
		rv = false;
	else
		AverageData["power"] = int2string(u32);

	if(RetreiveFromStack(inverterId, SOL_CMD_ENERGY_TODAY, &u32) == false)
		rv = false;
	else
		AverageData["DailyEnergy"] = int2string(u32);

	if(RetreiveFromStack(inverterId, SOL_CMD_ENERGY_TOTAL, &u32) == false)
		rv = false;
	else
		AverageData["TotalEnergy"] = int2string(u32);

	if(RetreiveFromStack(inverterId, SOL_CMD_FREQ_AC, &u32) == false)
		rv = false;
	else
		AverageData["frequencyAC"] = int2string(u32 /100);

	AverageData["status"] = "2";
	AverageData["clientID"] = "\"" + m_Uuid + "\"";

	return rv;
}

int
CSolutronicProbe::RetreiveFromStack(int DeviceNumber, int Command, uint32_t *result)
{
	pthread_mutex_lock(&m_queueMutex);
	std::list<struct MsgStruct>::iterator i = m_MsgQueue.begin();
	uint64_t sum = 0;
	uint16_t answers = 0;

	while (i != m_MsgQueue.end())
	{
		uint16_t CurInv = ((uint16_t) i->Data[3] << 8) | ((uint16_t) i->Data[4]);
		uint16_t CurCommand = (((uint16_t) i->Data[5] << 8) | (uint16_t) i->Data[6]) & 0x03FF;

		if(CurInv == DeviceNumber && CurCommand == Command)
		{
			sum += (uint32_t) i->Data[7] << 24 | i->Data[8] << 16 | i->Data[9] << 8 | i->Data[10];
			answers++;
			i = m_MsgQueue.erase(i);
		}
		else
			i++;
	}
	pthread_mutex_unlock(&m_queueMutex);

	if(answers > 0)
	{
		*result = sum/answers;
		return true;
	}
	else
	{
		*result = 0;
		return false;
	}
}


void *
CSolutronicProbe::SendingFunction(void *ptr)
{
	struct ThreadStruct args;
	memcpy(&args, ptr, sizeof(struct ThreadStruct));
	CInterface *Interface = args.interface;

	while(true)
	{
		for(list<int>::iterator curInv=args.connected->begin();curInv!=args.connected->end(); ++curInv)
		{
			if(SendMessage(Interface, 0x0, *curInv, SOL_CMD_VOLTAGE_AC, READ, (uint32_t)0x0000) == false)
				Log.error("Error sending VoltageAC Command");
			usleep(500000);
			if(SendMessage(Interface, 0x0, *curInv, SOL_CMD_VOLTAGE_DC, READ, (uint32_t)0x0000) == false)
				Log.error("Error sending VoltageDC Command");
			usleep(500000);
			if(SendMessage(Interface, 0x0, *curInv, SOL_CMD_CURRENT_AC, READ, (uint32_t)0x0000) == false)
				Log.error("Error sending CurrentAC Command");
			usleep(500000);
			if(SendMessage(Interface, 0x0, *curInv, SOL_CMD_CURRENT_DC, READ, (uint32_t)0x0000) == false)
				Log.error("Error sending CurrentDC Command");
			usleep(500000);
			if(SendMessage(Interface, 0x0, *curInv, SOL_CMD_POWER_AC, READ, (uint32_t)0x0000) == false)
				Log.error("Error sending PowerAC Command");
			usleep(500000);
			if(SendMessage(Interface, 0x0, *curInv, SOL_CMD_POWER_DC, READ, (uint32_t)0x0000) == false)
				Log.error("Error sending PowerDC Command");
			usleep(500000);
			if(SendMessage(Interface, 0x0, *curInv, SOL_CMD_ENERGY_TODAY, READ, (uint32_t)0x0000) == false)
				Log.error("Error sending EnergyDay Command");
			usleep(500000);
			if(SendMessage(Interface, 0x0, *curInv, SOL_CMD_ENERGY_TOTAL, READ, (uint32_t)0x0000) == false)
				Log.error("Error sending EnergyTotal Command");
			usleep(500000);
			if(SendMessage(Interface, 0x0, *curInv, SOL_CMD_FREQ_AC, READ, (uint32_t)0x0000) == false)
				Log.error("Error sending FreqAC Command");
			usleep(200000);
		}
	}

	return NULL;
}

void *
CSolutronicProbe::ReceivingFunction(void *ptr)
{
	struct ThreadStruct args;
	memcpy(&args, ptr, sizeof(struct ThreadStruct));

	CInterface *Interface = args.interface;

	uint8_t CurrentMessage[1024];
	uint16_t MsgLen = 13; // By default we assume that we have 4 byte data
	uint8_t in = 0;
	uint16_t MsgPtr = 0;
	bool STXDetected = false;

	while(true)
	{
		// Solutronic made a major mistake in the design of the protocol as the STX/ETX which
		// are the start and end of the protocol work only on ASCII protocols. In binary protocols
		// you cannot distinguish the start/end of the message from the actual data

		if(!Interface->Receive(&in, 1, 1))
		{
			syslog(LOG_ERR, "Error Reading from Interface\n");
			sleep(1);
			continue;
		}

		if(STXDetected == false && in == 0x02)
		{
			STXDetected = true;
			MsgPtr = 0;
		}
		else if(STXDetected == false)
			continue;

		CurrentMessage[MsgPtr]  = in;

		if(MsgPtr == 5)
		{
			if((CurrentMessage[MsgPtr] & 0x08) != 0)
				MsgLen += 124;	// Change the overall data length to 128 bytes
		}

		if(MsgPtr == MsgLen - 1)
		{
			if(CurrentMessage[MsgPtr] != 0x03)
			{
				STXDetected = false;	// Last byte is not ETX. Discard the message
				MsgPtr = 0;
				continue;
			}

			uint8_t LRC = 0;
			for(int i=1; i< MsgLen - 2; i++)
				LRC ^= CurrentMessage[i];

			if(LRC != CurrentMessage[MsgPtr-1])
			{
				STXDetected = false;	// LRC calculation failed. Discard the message
				MsgPtr = 0;
				continue;
			}

			// If we are here, we have a message. Push it up the queue
			if(debugFlag == true)
			{
				//syslog(LOG_INFO, "Message Received:\n");
				//Log.hexDump(LOG_DEBUG, CurrentMessage, MsgLen);
			}

			// Push the message back to the message queue
			struct MsgStruct msg;
			msg.DataLen = MsgLen;
			memcpy((void *)msg.Data, CurrentMessage, MsgLen);
			msg.TimeStamp = time(NULL);
			msg.Iface = Interface;

			pthread_mutex_lock(args.queueMutex);
			args.MsgQueue->push_back(msg);
			pthread_mutex_unlock(args.queueMutex);

			// Reset and wait for new message
			STXDetected = false;
			MsgPtr = 0;
			continue;
		}


		MsgPtr++;
	}

	return NULL;
}

int
CSolutronicProbe::SendMessage(CInterface *Interface, uint16_t src, uint16_t dest, uint16_t Command, bool rw, uint32_t data)
{
	return SendMessage(Interface, src, dest, Command, rw, BYTE4, (uint8_t *)&data, 4);
}

int
CSolutronicProbe::SendMessage(CInterface *Interface, uint16_t src, uint16_t dest, uint16_t Command, bool rw, bool proto, uint8_t *data, uint16_t length)
{
        uint8_t message[256];
        memset(message, 0x0, sizeof(message));

        // Destination
        message[0] = (uint8_t) (dest >> 8);
        message[1] = (uint8_t) (dest);
        // Source
        message[2] = (uint8_t) (src >> 8);
        message[3] = (uint8_t) (src);

        // Command
        if(rw == READ)
                Command |= 0x4000;
        else // It's a write
                Command |= 0x8000;

        if(proto == BYTE128)
        	Command |= 0x0800;

        message[4] = (uint8_t) (Command >> 8);
        message[5] = (uint8_t) (Command);

        // Add empty 32bit data
        memcpy(&message[6], data, length);


        // Add LRC
        for(int i=0; i<length + 6; i++)
                message[length + 6] ^= message[i];

        // Encapsulate STX/ETX around message
        memmove(&message[1], message, length + 7);
        message[0] = 0x02;
        message[length + 8] = 0x03;

    	if(debugFlag == true)
    	{
    		//syslog(LOG_INFO, "Sending msg:\n");
    		//Log.hexDump(LOG_DEBUG, message, length + 9);
    	}

    	if(!Interface->Send(message, length + 9))
    		return false;
    	else
    		return true;
}
