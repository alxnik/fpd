/*
 * Copyright (c) 2012 Alexandros Nikolopoulos <alxnik@gmail.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.

 */

#ifndef CSOLUTRONICPROBE_H_
#define CSOLUTRONICPROBE_H_

#include "fpd.h"
#include "CLog.h"
#include "CSerial.h"
#include "CFroniusProbe.h"
using namespace std;

class CSolutronicProbe : public CProbe
{
public:
	CSolutronicProbe(CInterface *Iface, string uuid);
	CSolutronicProbe(CInterface *Iface, list<int> sensors, string uuid);
	virtual ~CSolutronicProbe();

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
	static int SendMessage(CInterface *Interface, uint16_t src, uint16_t dest, uint16_t Command, bool rw, bool proto, uint8_t *data, uint16_t length);
	static int SendMessage(CInterface *Interface, uint16_t src, uint16_t dest, uint16_t Command, bool rw, uint32_t data);

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

enum ReadBit
{
	READ = 0,
	WRITE
};
enum PacketProto
{
	BYTE4 = 0,
	BYTE128
};
// The commands!
enum {
	SOL_CMD_VOLTAGE_AC= 1,
	SOL_CMD_VOLTAGE_DC,
	SOL_CMD_CURRENT_AC,
	SOL_CMD_CURRENT_DC,
	SOL_CMD_POWER_AC,
	SOL_CMD_POWER_DC,
	SOL_CMD_ENERGY_TODAY = 8,
	SOL_CMD_ENERGY_TOTAL = 12,
	SOL_CMD_FREQ_AC = 15,
};

#endif /* CSOLUTRONICPROBE_H_ */
