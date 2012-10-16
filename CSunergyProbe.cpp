/*
 * Copyright (c) 2012 Alexandros Nikolopoulos <alxnik@gmail.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.

 */

#include "CSunergyProbe.h"
extern bool SigTermFlag;

CSunergyProbe::CSunergyProbe(CModbusMTU *Modbus, string uuid)
{
	m_Interface = Modbus;
	m_ReceivingThread = 0;

	pthread_mutex_init(&m_queueMutex, NULL);
}

CSunergyProbe::~CSunergyProbe()
{
	Stop();
	delete(m_Interface);
}

int
CSunergyProbe::ResetStack(void)
{
	std::list<struct SynMsgStruct>::iterator i = m_MsgQueue.begin();
	pthread_mutex_lock(&m_queueMutex);

	while (i != m_MsgQueue.end())
	{
		i = m_MsgQueue.erase(i);
		i++;
	}
	pthread_mutex_unlock(&m_queueMutex);

	return true;
}

bool
CSunergyProbe::probeInverters(void)
{
	uint16_t p = 0;
	int lastConnected = 0;
	m_Interface->SetTimeout(600000);

	for(int slave = 1; slave <= 247; slave++)
	{
		map<uint16_t,uint16_t> ResultSet;

		if(m_Interface->ReadInputReg(slave, 1000, 2, ResultSet))
		{
			syslog(LOG_INFO, "Found inverter at %d\n", slave);
			m_connected.push_back(slave);
			lastConnected = slave;
		}

		if(slave - lastConnected > 5)
			break;

		if(SigTermFlag)
			break;
	}

	m_Interface->SetTimeout(3000000);

	if(p == 0)
		return false;
	else
		return true;
}

int
CSunergyProbe::Start(void)
{
	if(probeInverters() == false)
	{
		Stop();
		return false;
	}

	// All is well.
	// Start the main sending thread
	struct SunergyThreadStruct args;
	args.interface = m_Interface;
	args.connected = &m_connected;
	args.queueMutex = &m_queueMutex;
	args.MsgQueue = &m_MsgQueue;

	if(!m_ReceivingThread)
		pthread_create( &m_ReceivingThread, NULL, &CSunergyProbe::ReceivingFunction,  &args);
	sleep(1);

	return true;
}

int
CSunergyProbe::Stop(void)
{
	if(m_ReceivingThread)
	{
		pthread_cancel(m_ReceivingThread);
		m_ReceivingThread = 0;
	}
	return true;
}

list<int>
CSunergyProbe::GetConnectedInverters(void)
{
	return m_connected;
}

int
CSunergyProbe::GetAverage(DataContainer &AverageData)
{
	int inverterId = atoi(AverageData["inverter"].c_str());

	bool rv = true;
	float flt = 0;
	uint32_t u32 = 0;
	if(RetreiveFromStack(inverterId, INVERTER_STATE, &u32) == false)
			rv = false;
	else
	{
		// Change Sunergy status values to Solarspy.net status values (stolen from fronius)
		switch(u32)
		{
			case 0:
				AverageData["status"] = "1";
				break;
			case 1:
				AverageData["status"] = "1";
				break;
			case 2:
				AverageData["status"] = "4";
				break;
			case 3:
				AverageData["status"] = "3";
				break;
			case 4:
				AverageData["status"] = "2";
				break;
			default:
				rv = false;
				break;
		}
	}
	if(RetreiveFromStack(inverterId, INPUT_VDC, &flt) == false)
		rv = false;
	else
		AverageData["voltageDC"] = flt;

	if(RetreiveFromStack(inverterId, INPUT_IDC, &flt) == false)
		rv = false;
	else
		AverageData["currentDC"] = flt;

	if(RetreiveFromStack(inverterId, OUTPUT_VAC, &flt) == false)
		rv = false;
	else
		AverageData["voltageAC"] = flt;

	if(RetreiveFromStack(inverterId, OUTPUT_IAC, &flt) == false)
		rv = false;
	else
		AverageData["currentAC"] = flt;

	if(RetreiveFromStack(inverterId, GRID_FREQUENCY, &flt) == false)
		rv = false;
	else
		AverageData["frequencyAC"] = flt;

	if(RetreiveFromStack(inverterId, LIFETIME_KWH, &flt) == false)
		rv = false;
	else
		AverageData["TotalEnergy"] = float2string(flt * 1000);

	if(RetreiveFromStack(inverterId, DAILY_KWH, &flt) == false)
		rv = false;
	else
		AverageData["DayEnergy"] = float2string(flt * 1000);

	if(rv == true)
		AverageData["power"] = int2string(atoi(AverageData["currentDC"].c_str()) * atoi(AverageData["voltageDC"].c_str()));

	return rv;
}

int
CSunergyProbe::RetreiveFromStack(int DeviceNumber, int Command, uint32_t *result)
{
	if(m_MsgQueue.size() == 0)
			return false;

	pthread_mutex_lock(&m_queueMutex);
	std::list<struct SynMsgStruct>::iterator i = m_MsgQueue.begin();
	int answers = 0;
	float sum = 0;

	while(i != m_MsgQueue.end())
	{
		if(DeviceNumber == i->slaveId)
		{
			uint16_t Answer[2];
			Answer[0] = i->ResultSet[Command];
			Answer[1] = i->ResultSet[Command+1];
			sum += MODBUS_GET_INT32_FROM_INT16(Answer, 0);

			answers++;
		}
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
CSunergyProbe::RetreiveFromStack(int DeviceNumber, int Command, float *result)
{
	if(m_MsgQueue.size() == 0)
		return false;

	pthread_mutex_lock(&m_queueMutex);
	std::list<struct SynMsgStruct>::iterator i = m_MsgQueue.begin();
	int answers = 0;
	float sum = 0;

	while(i != m_MsgQueue.end())
	{
		if(DeviceNumber == i->slaveId)
		{
			uint16_t Answer[2];
			Answer[0] = i->ResultSet[Command+1];
			Answer[1] = i->ResultSet[Command];
			sum += modbus_get_float(Answer);

			answers++;
		}
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
CSunergyProbe::ReceivingFunction(void *ptr)
{
	struct SunergyThreadStruct args;

	memcpy(&args, ptr, sizeof(struct SunergyThreadStruct));


	while(true)
	{
		// For modbus interfaces we don't need to read all the values one by one. Read'em
		// all at once and then break them down
		int i = 0;

		for(list<int>::iterator curInv=args.connected->begin();curInv!=args.connected->end(); ++curInv)
		{
			map<uint16_t, uint16_t> ResultSet;
			if(args.interface->ReadInputReg(*curInv, INVERTER_STATE, GFDI_TYPE - INVERTER_STATE, ResultSet))
			{
				struct SynMsgStruct msg;

				msg.slaveId = *curInv;
				msg.ResultSet = ResultSet;
				msg.TimeStamp = time(NULL);
				msg.Iface = args.interface;

				pthread_mutex_lock(args.queueMutex);
				args.MsgQueue->push_back(msg);
				pthread_mutex_unlock(args.queueMutex);
			}
			else
				syslog(LOG_ERR, "Unresponsive Inverter %d", *curInv);
			i++;
		}

		sleep(5);
	}

	return 0;
}
