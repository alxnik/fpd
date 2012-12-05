/*
 * Copyright (c) 2012 Alexandros Nikolopoulos <alxnik@gmail.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.

 */

#ifndef CFRONIUSPROBE_H_
#define CFRONIUSPROBE_H_
#include "fpd.h"
#include "CLog.h"
#include "CSerial.h"
using namespace std;

class CFroniusProbe : public CProbe
{
public:
	CFroniusProbe(CInterface *SerialLine, string uuid);
	CFroniusProbe(CInterface *SerialLine, list<int> sensors, string uuid);
	virtual ~CFroniusProbe();

	int Start(void);
	int Stop(void);
	int GetAverage(DataContainer &AverageData);
	list<int> GetConnectedInverters(void);
	string GetUUID(void);
	int ResetStack(void);

private:
	bool probeInverters(void);
	bool probeStaticValues(void);

	// Constructs and sends a message to the inverter network
	static int SendMessage(CInterface *interface, uint8_t Command, uint8_t networkNumber, uint8_t DeviceType, uint8_t *data, uint8_t datalen);

	int RetreiveFromStack(int DeviceNumber, int Command, float *result);
	int RetreiveFromStack(int DeviceNumber, int Command, uint8_t *result);
	int RetreiveFromStack(int DeviceNumber, int Command, uint32_t *result);
	int RetreiveFromStack(int DeviceNumber, int Command, uint64_t *result);

	// The thread function that run independently
	static void * SendingFunction(void *ptr);
	static void * ReceivingFunction(void *ptr);

	// Convert byteArray to uint32 or unit64 according to fronius specs
	float ProcessExponent32(uint8_t *exp, uint8_t len);
	uint64_t ProcessExponent64(uint8_t *exp, uint8_t len);

	// The Thread holders
	pthread_t m_SendingThread;
	pthread_t m_ReceivingThread;


	CInterface *m_Interface;

	// Int array containing the network numbers of all the connected devices
	list<int> m_connected;

	// The structure that holds the received messages, ready to be processed,
	// Along with its mutex
	std::list<struct MsgStruct> m_MsgQueue;
	pthread_mutex_t m_queueMutex;
};


// Types of commands
enum {
	TYPE_GENERAL=0x00,
	TYPE_INVERTER,
	TYPE_SENSOR,
	TYPE_DATALOGGER,
	TYPE_RESERVED,
	TYPE_STRING_CTRL
};

// The commands!
enum {
	// General Commands
	CMD_VERSION=0x01,			// 01
	CMD_DEVICE_TYPE,			// 02
	CMD_DATE_TIME,				// 03
	CMD_ACTIVE_INVERTERS,		// 04
	CMD_ACTIVE_SENSORS,			// 05
	CMD_SOLAR_NET_STATUS,		// 06
	CMD_DEVICE_VERSION=0xbe,	// be
	CMD_DEVICE_ID,				// bf

	// Error Messages
	CMD_SET_ERROR_SENDING=0x07,		// 07
	CMD_SET_ERROR_FORWARDING=0x0d,	// 0d
	CMD_IFC_PROTOCOL_ERROR,			// 0e
	CMD_STATES,						// 0f

	// Measured value queries
	CMD_POWER_NOW=0x10,				// 10
	CMD_ENERGY_TOTAL,				// 11
	CMD_ENERGY_DAY,					// 12
	CMD_ENERGY_YEAR,				// 13
	CMD_AC_CURRENT_NOW,				// 14
	CMD_AC_VOLTAGE_NOW,				// 15
	CMD_AC_FREQUENCY_NOW,			// 16
	CMD_DC_CURRENT_NOW,				// 17
	CMD_DC_VOLTAGE_NOW,				// 18
	CMD_YIELD_DAY,					// 19
	CMD_MAX_POWER_DAY,				// 1A
	CMD_MAX_AC_VOLTAGE_DAY,			// 1B
	CMD_MIN_AC_VOLTAGE_DAY,			// 1C
	CMD_MAX_DC_VOLTAGE_DAY,			// 1D
	CMD_OPERATING_HOURS_DAY,		// 1E
	CMD_YIELD_YEAR,					// 1F
	CMD_MAX_POWER_YEAR,				// 20
	CMD_MAX_AC_VOLTAGE_YEAR,		// 21
	CMD_MIN_AC_VOLTAGE_YEAR,		// 22
	CMD_MAX_DC_VOLTAGE_YEAR,		// 23
	CMD_OPERATING_HOURS_YEAR,		// 24
	CMD_YIELD_TOTAL,				// 25
	CMD_MAX_POWER_TOTAL,			// 26
	CMD_MAX_AC_VOLTAGE_TOTAL,		// 27
	CMD_MIN_AC_VOLTAGE_TOTAL,		// 28
	CMD_MAX_DC_VOLTAGE_TOTAL,		// 29
	CMD_OPERATING_HOURS_TOTAL,		// 2A
	CMD_PHASE_CURRENT1,				// 2B
	CMD_PHASE_CURRENT2,				// 2C
	CMD_PHASE_CURRENT3,				// 2D
	CMD_PHASE_VOLTAGE1,				// 2E
	CMD_PHASE_VOLTAGE2,				// 2F
	CMD_PHASE_VOLTAGE3,				// 30
	CMD_AMBIENT_TEMP,				// 31
	CMD_FAN_ROTATION1,				// 32
	CMD_FAN_ROTATION2,				// 33
	CMD_FAN_ROTATION3,				// 34
	CMD_FAN_ROTATION4,				// 35
	CMD_ENERGY_TOTAL_EX,			// 36
	CMD_INVERTER_STATUS,			// 37

};


#endif /* CPROBE_H_ */
