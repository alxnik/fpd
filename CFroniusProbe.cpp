/*
 * Copyright (c) 2012 Alexandros Nikolopoulos <alxnik@gmail.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.

 */

#include "CFroniusProbe.h"
#include "errno.h"


CFroniusProbe::CFroniusProbe(CInterface *SerialLine, string uuid)
{
	m_Interface = SerialLine;
	m_SendingThread = m_ReceivingThread = 0;

	pthread_mutex_init(&m_queueMutex, NULL);
}

CFroniusProbe::CFroniusProbe(CInterface *SerialLine, list<int> sensors, string uuid)
{
	m_Interface = SerialLine;
	m_SendingThread = m_ReceivingThread = 0;

	m_connected = sensors;

	pthread_mutex_init(&m_queueMutex, NULL);
}

CFroniusProbe::~CFroniusProbe()
{
	Stop();
	delete(m_Interface);
}

bool
CFroniusProbe::probeInverters(void)
{
	// Try to send probing message in order to find all the active inverters in the network
	for(int l=0; l < 5; l++)
	{
		if(!SendMessage(m_Interface, CMD_ACTIVE_INVERTERS, 0, TYPE_GENERAL, NULL, 0))
			return false; // Will never normally happen

		pthread_mutex_lock(&m_queueMutex);
		std::list<struct MsgStruct>::iterator i = m_MsgQueue.begin();
		while (i != m_MsgQueue.end())
		{
				if(i->Data[3] == 0x04 && i->DataLen>5)
				{
					int j = 0;
					for(j=0; j<i->DataLen - 5; j++)
						m_connected.push_back(i->Data[j+4]);

					i = m_MsgQueue.erase(i);
					pthread_mutex_unlock(&m_queueMutex);
					syslog(LOG_INFO, "Found %d inverters\n", j);
					return true;
				}
				else
					i++;
		}
		pthread_mutex_unlock(&m_queueMutex);
	}
	return false;
}

bool
CFroniusProbe::probeStaticValues(void)
{
	return true;
}

int
CFroniusProbe::Start(void)
{
	struct ThreadStruct RecArgs;

	RecArgs.interface = m_Interface;
	RecArgs.queueMutex = &m_queueMutex;
	RecArgs.MsgQueue = &m_MsgQueue;

	if(!m_ReceivingThread)
		pthread_create( &m_ReceivingThread, NULL, &CFroniusProbe::ReceivingFunction,  &RecArgs);

	// Check for inverters. If none is found, stop the process
	if(m_connected.empty() == true)
	{
		if(probeInverters() == false)
		{
			Stop();
			return false;
		}
	}

	// All is well.
	// Start the main sending thread
	struct ThreadStruct args;
	args.interface = m_Interface;
	args.connected = &m_connected;

	if(!m_SendingThread)
		pthread_create( &m_SendingThread, NULL, &CFroniusProbe::SendingFunction,  &args);
	sleep(1);
	return true;
}

int
CFroniusProbe::Stop(void)
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

list<int>
CFroniusProbe::GetConnectedInverters(void)
{
	// Save the old list. If the probe fails, restore the old one
	list<int> ConnectedProbes = m_connected;

	if(probeInverters() == false)
		m_connected = ConnectedProbes;

	return m_connected;
}

int
CFroniusProbe::GetAverage(DataContainer &AverageData)
{
	int inverterId = atoi(AverageData["inverter"].c_str());

	bool rv = true;

	if(m_MsgQueue.size() == 0)
	{
		syslog(LOG_ERR, "No messages in Queue\n");
		return false;
	}

	pthread_mutex_lock(&m_queueMutex);
	std::list<struct MsgStruct>::iterator i = m_MsgQueue.begin();
	int answers = 0;

	while(i != m_MsgQueue.end())
	{
		if(i->Data[2] == inverterId && i->Data[3] == CMD_INVERTER_STATUS)
		{
			if(i->Data[0] > 0)
				AverageData["status"] = int2string(i->Data[4]);
			else
			{
				pthread_mutex_unlock(&m_queueMutex);
				if(debugFlag == true)
					syslog(LOG_INFO, "Inverter is offline (%ld secs ago)", time(NULL) - i->TimeStamp);
				return false;
			}
			i = m_MsgQueue.erase(i);
			answers++;
		}
		else
			i++;
	}
	pthread_mutex_unlock(&m_queueMutex);

	if(answers == 0)
	{
		syslog(LOG_WARNING, "No Status results");
		return false;
	}

	// If the inverter is not in operating state, don't probe for any other values as
	// the fronius inverters seem to send out faulty data on other states
	if(atoi(AverageData["status"].c_str()) != 2)
		return false;

	uint32_t u32;
	uint64_t u64;
	float	flt;

	if(RetreiveFromStack(inverterId, CMD_POWER_NOW, &u32) == false)
		rv = false;
	else
		AverageData["power"] = int2string(u32);

	if(RetreiveFromStack(inverterId, CMD_AC_CURRENT_NOW, &flt) == false)
		rv = false;
	else
		AverageData["currentAC"] = float2string(flt);

	if(RetreiveFromStack(inverterId, CMD_DC_CURRENT_NOW, &flt) == false)
		rv = false;
	else
		AverageData["currentDC"] = float2string(flt);

	if(RetreiveFromStack(inverterId, CMD_PHASE_CURRENT1, &flt) == false)
		rv = false;
	else
		AverageData["Phase1Current"] = float2string(flt);

	if(RetreiveFromStack(inverterId, CMD_PHASE_CURRENT2, &flt) == false)
		rv = false;
	else
		AverageData["Phase2Current"] = float2string(flt);

	if(RetreiveFromStack(inverterId, CMD_PHASE_CURRENT3, &flt) == false)
		rv = false;
	else
		AverageData["Phase3Current"] = float2string(flt);

	if(RetreiveFromStack(inverterId, CMD_AC_FREQUENCY_NOW, &flt) == false)
		rv = false;
	else
		AverageData["frequencyAC"] = float2string(flt);

	if(RetreiveFromStack(inverterId, CMD_AC_VOLTAGE_NOW, &u32) == false)
		rv = false;
	else
		AverageData["voltageAC"] = int2string(u32);

	if(RetreiveFromStack(inverterId, CMD_DC_VOLTAGE_NOW, &u32) == false)
		rv = false;
	else
		AverageData["voltageDC"] = int2string(u32);

	if(RetreiveFromStack(inverterId, CMD_PHASE_VOLTAGE1, &u32) == false)
		rv = false;
	else
		AverageData["Phase1Voltage"] = int2string(u32);

	if(RetreiveFromStack(inverterId, CMD_PHASE_VOLTAGE2, &u32) == false)
		rv = false;
	else
		AverageData["Phase2Voltage"] = int2string(u32);

	if(RetreiveFromStack(inverterId, CMD_PHASE_VOLTAGE3, &u32) == false)
		rv = false;
	else
		AverageData["Phase3Voltage"] = int2string(u32);

	if(RetreiveFromStack(inverterId, CMD_ENERGY_DAY, &u32) == false)
		rv = false;
	else
		AverageData["DailyEnergy"] = int2string(u32);

	if(RetreiveFromStack(inverterId, CMD_ENERGY_TOTAL_EX, &u64) == false)
		rv = false;
	else
		AverageData["TotalEnergy"] = int2string(u64);



	return rv;
}

// RetreiveFromStack with 64bit value is used only for TotalPower. For this, we don't want
// An averaging...
// TODO: Make a better way to differantiate between averaged/non averaged values
int
CFroniusProbe::RetreiveFromStack(int DeviceNumber, int Command, uint64_t *result)
{
	pthread_mutex_lock(&m_queueMutex);
	std::list<struct MsgStruct>::iterator i = m_MsgQueue.begin();
	uint64_t sum = 0;
	uint16_t answers = 0;
	while (i != m_MsgQueue.end())
	{
		if(i->Data[2] == DeviceNumber && i->Data[3] == Command)
		{
			if(Command == CMD_ENERGY_TOTAL_EX)
				sum = ProcessExponent64(&(i->Data[4]), i->Data[0]);
			else
				sum += ProcessExponent64(&(i->Data[4]), i->Data[0]);
			answers++;
			i = m_MsgQueue.erase(i);
		}
		else
			i++;
	}
	pthread_mutex_unlock(&m_queueMutex);

	if(answers > 0)
	{
		if(Command == CMD_ENERGY_TOTAL_EX)
			*result = sum;
		else
			*result = sum/answers;
		return true;
	}
	else
	{
		*result = 0;
		return false;
	}
}

int
CFroniusProbe::RetreiveFromStack(int DeviceNumber, int Command, uint32_t *result)
{
	pthread_mutex_lock(&m_queueMutex);
	std::list<struct MsgStruct>::iterator i = m_MsgQueue.begin();
	uint64_t sum = 0;
	uint16_t answers = 0;

	while (i != m_MsgQueue.end())
	{
		if(i->Data[2] == DeviceNumber && i->Data[3] == Command)
		{
			if(Command == CMD_ENERGY_DAY)
				sum = ProcessExponent32(&(i->Data[4]), i->Data[0]);
			else
				sum += ProcessExponent32(&(i->Data[4]), i->Data[0]);
			answers++;
			i = m_MsgQueue.erase(i);
		}
		else
			i++;
	}
	pthread_mutex_unlock(&m_queueMutex);
	if(answers > 0)
	{
		if(Command == CMD_ENERGY_DAY)
			*result = sum;
		else
			*result = sum/answers;
		return true;
	}
	else
	{
		*result = 0;
		return false;
	}
}

int
CFroniusProbe::RetreiveFromStack(int DeviceNumber, int Command, float *result)
{
	pthread_mutex_lock(&m_queueMutex);
	std::list<struct MsgStruct>::iterator i = m_MsgQueue.begin();
	float sum = 0;
	uint16_t answers = 0;

	while (i != m_MsgQueue.end())
	{
		if(i->Data[2] == DeviceNumber && i->Data[3] == Command)
		{
			sum += ProcessExponent32(&(i->Data[4]), i->Data[0]);
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

int
CFroniusProbe::ResetStack(void)
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

float
CFroniusProbe::ProcessExponent32(uint8_t *exp, uint8_t len)
{
	if(len > 5)
		return 0;

	uint8_t mainLen = len-1;
	uint32_t number = 0;

	for(int i=0; i<mainLen; i++)
		number |= ((uint32_t) *(exp+i)) << ((56-(64-mainLen*8))- (8 * i));

	return number * pow(10, (int8_t) *(exp+len-1));
}

uint64_t
CFroniusProbe::ProcessExponent64(uint8_t *exp, uint8_t len)
{
	if(len > 11)
		return 0;

	uint8_t mainLen = len-1;
	uint32_t number = 0;

	for(int i=0; i<mainLen; i++)
		number |= ((uint32_t) *(exp+i)) << ((56-(64-mainLen*8))- (8 * i));

	return number * pow(10, (int8_t) *(exp+len-1));
}

void *
CFroniusProbe::SendingFunction(void *ptr)
{
	struct ThreadStruct args;

	memcpy(&args, ptr, sizeof(struct ThreadStruct));



	while(true)
	{
		for(list<int>::iterator curInv=args.connected->begin();curInv!=args.connected->end(); ++curInv)
		{
			SendMessage(args.interface, CMD_INVERTER_STATUS, *curInv, TYPE_INVERTER, NULL, 0);

			SendMessage(args.interface, CMD_POWER_NOW, *curInv, TYPE_INVERTER, NULL, 0);

			SendMessage(args.interface, CMD_AC_CURRENT_NOW, *curInv, TYPE_INVERTER, NULL, 0);
			SendMessage(args.interface, CMD_DC_CURRENT_NOW, *curInv, TYPE_INVERTER, NULL, 0);
			SendMessage(args.interface, CMD_PHASE_CURRENT1, *curInv, TYPE_INVERTER, NULL, 0);
			SendMessage(args.interface, CMD_PHASE_CURRENT2, *curInv, TYPE_INVERTER, NULL, 0);
			SendMessage(args.interface, CMD_PHASE_CURRENT3, *curInv, TYPE_INVERTER, NULL, 0);

			SendMessage(args.interface, CMD_AC_VOLTAGE_NOW, *curInv, TYPE_INVERTER, NULL, 0);
			SendMessage(args.interface, CMD_DC_VOLTAGE_NOW, *curInv, TYPE_INVERTER, NULL, 0);
			SendMessage(args.interface, CMD_PHASE_VOLTAGE1, *curInv, TYPE_INVERTER, NULL, 0);
			SendMessage(args.interface, CMD_PHASE_VOLTAGE2, *curInv, TYPE_INVERTER, NULL, 0);
			SendMessage(args.interface, CMD_PHASE_VOLTAGE3, *curInv, TYPE_INVERTER, NULL, 0);

			SendMessage(args.interface, CMD_AC_FREQUENCY_NOW, *curInv, TYPE_INVERTER, NULL, 0);
			SendMessage(args.interface, CMD_ENERGY_DAY, *curInv, TYPE_INVERTER, NULL, 0);


			SendMessage(args.interface, CMD_ENERGY_TOTAL_EX, *curInv, TYPE_INVERTER, (uint8_t *) "\x01", 1);
		}
	}

	return NULL;
}

void *
CFroniusProbe::ReceivingFunction(void *ptr)
{
	struct ThreadStruct args;
	memcpy(&args, ptr, sizeof(struct ThreadStruct));

	CInterface *Interface = args.interface;


	uint8_t CurrentMessage[512];
	uint16_t MsgLen = 0;
	uint8_t in = 0;
	uint16_t MsgPtr = 0;
	uint8_t SeqDetect = 0;
	bool SeqDetected = false;

	while(true)
	{
		// Read from serial
		if(!Interface->Receive(&in, 1, 1))
		{
			syslog(LOG_ERR, "Error Reading from Interface\n");
			sleep(1);
			continue;
		}
		// 0x808080 Detector
		// Don't continue to the parsing if we don't detect the start sequence
		// Also, reset the parsing if we detect a start sequence
		if(in == 0x80)
			SeqDetect++;
		else
		{
			if(SeqDetect >= 3)
			{
				SeqDetected = true;
				MsgPtr = 0;
			}
			SeqDetect = 0;
		}

		// Don't parse if there was no sequence detected
		if(SeqDetected == false)
			continue;


		CurrentMessage[MsgPtr] = in;

		// If this is the first byte, grab the length
		if(MsgPtr == 0)
		{
			// If the length is too big, then it's probably an error. Reset
			if(in > 0x85)
			{
				SeqDetected = false;
				continue;
			}
			MsgLen = in;
		}

		// If the current pointer matches the length, go for processing
		else if(MsgPtr == MsgLen + 5)
		{
			int i = 0;
			uint8_t sum = 0;

			while(i < MsgPtr-1)
			{
				sum += CurrentMessage[i];
				i++;
			}

			if(sum == CurrentMessage[i])
			{
				if(debugFlag == true)
				{
					//syslog(LOG_INFO, "Message Received:\n");
					//Log.hexDump(LOG_DEBUG, CurrentMessage, MsgPtr);
				}

				// Push the message back to the message queue
				struct MsgStruct msg;
				msg.DataLen = MsgPtr;
				memcpy((void *)msg.Data, CurrentMessage, MsgPtr);
				msg.TimeStamp = time(NULL);
				msg.Iface = Interface;

				pthread_mutex_lock(args.queueMutex);
				args.MsgQueue->push_back(msg);
				pthread_mutex_unlock(args.queueMutex);
			}

			// Reset and wait for new message
			SeqDetected = false;
			MsgPtr = 0;
			continue;
		}

		// Continue with the next byte of the message
		MsgPtr++;
	}

	return NULL;
}

int
CFroniusProbe::SendMessage(CInterface *interface, uint8_t Command, uint8_t networkNumber, uint8_t DeviceType, uint8_t *data, uint8_t datalen)
{
	uint8_t message[256];
	uint16_t ptr = 0;

	// Message Sync. 0x808080
	message[0] = message[1] = message[2] = 0x80;
	ptr += 3;

	// Length
	message[ptr++] = datalen;

	// Device/option - Hardcoded to Inverter
	message[ptr++] = DeviceType;

	// Network Number
	message[ptr++] = networkNumber;

	// Command
	message[ptr++] = Command;

	// data
	if(data && datalen>0)
	{
		memcpy(&message[ptr], (void*) data, datalen);
		ptr+= datalen;
	}

	// Checksum
	message[ptr] = 0x0;
	for(int i=3; i<ptr; i++)
	{
		message[ptr] += message[i];
	}
	ptr++;

	if(debugFlag == true)
	{
		//syslog(LOG_INFO, "Sending msg:\n");
		//Log.hexDump(LOG_DEBUG, message, ptr);
	}

	if(!interface->Send(message, ptr))
	{
		usleep(250000);
		return false;
	}
	else
	{
		usleep(50000);
		return true;
	}
}
