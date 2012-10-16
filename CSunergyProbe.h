/*
 * Copyright (c) 2012 Alexandros Nikolopoulos <alxnik@gmail.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.

 */

#ifndef CSYNERGYPROBE_H_
#define CSYNERGYPROBE_H_

#include "fpd.h"
#include "CModbusMTU.h"

struct SynMsgStruct
{
	uint16_t				slaveId;
	map<uint16_t, uint16_t> ResultSet;
	uint32_t 				TimeStamp;
	CModbusMTU				*Iface;
};
struct SunergyThreadStruct
{
	CModbusMTU *interface;

	std::list<struct SynMsgStruct> *MsgQueue;
	pthread_mutex_t *queueMutex;

	list<int> *connected;
};

class CSunergyProbe : public CProbe
{
public:
	CSunergyProbe(CModbusMTU *Modbus, string uuid);
	virtual ~CSunergyProbe();

	int Start(void);
	int Stop(void);
	int GetAverage(DataContainer &AverageData);
	list<int> GetConnectedInverters(void);
	int ResetStack(void);
private:
	static void *ReceivingFunction(void *ptr);
	bool probeInverters(void);

	int RetreiveFromStack(int DeviceNumber, int Command, uint32_t *result);
	int RetreiveFromStack(int DeviceNumber, int Command, float *result);

	CModbusMTU *m_Interface;

	list<int> m_connected;

	pthread_t m_ReceivingThread;

	// The structure that holds the received messages, ready to be processed,
	// Along with its mutex
	std::list<struct SynMsgStruct> m_MsgQueue;
	pthread_mutex_t m_queueMutex;
};

// The commands!
enum
{
	INVERTER_STATE = 1000,
	INVERTER_PERMISSIVE = 1002,
	INVERTER_PERMISSIVE_DATA = 1004,
	TIME_CONNECTED = 1006,
	INPUT_VDC = 1008,
	INPUT_IDC = 1010,
	OUTPUT_VAC = 1012,
	OUTPUT_IAC = 1014,
	GRID_FREQUENCY = 1016,
	GRID_VAC_L1 = 1018,
	GRID_VAC_L2 = 1020,
	INTERNAL_GFDI = 1022,
	INTERNAL_TEMP = 1030,
	LIFETIME_KWH = 1032,
	DAILY_KWH = 1034,
	GFDI_TYPE = 1036
};
#endif /* CSYNERGYPROBE_H_ */
