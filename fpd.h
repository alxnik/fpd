/*
 * Copyright (c) 2012 Alexandros Nikolopoulos <alxnik@gmail.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#ifndef FPD_H_
#define FPD_H_


#define MAJOR_VERSION 2
#define MINOR_VERSION 6

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// #include <time.h>
// #include <pthread.h>

#include <list>
#include <map>
#include <string>
#include <sstream>
using namespace std;

#include "CLog.h"

extern CLog Log;
extern bool debugFlag;
extern bool SigTermFlag;

typedef map<string, string> DataContainer;
typedef list<DataContainer> DataContainerList;

typedef DataContainer SettingsContainer;
typedef DataContainerList SettingsContainerList;

// Generic class of the Probes
class CProbe
{

public:
	virtual ~CProbe(void){};

	virtual int Start(void) = 0;
	virtual int Stop(void) = 0;
	virtual int GetAverage(DataContainer &AverageData) = 0;
	virtual list<int> GetConnectedInverters(void) = 0;
	virtual int ResetStack(void) = 0;

	map<int, string> outputs;
};

// Generic class of the hardware interfaces
class CInterface
{
public:
	virtual ~CInterface(){};

	virtual int	Connect(void) = 0;
	virtual int Disconnect(void) = 0;

	virtual int Send(uint8_t *message, int length) = 0;
	virtual int Receive(uint8_t *message, int length, time_t timeout) = 0;

	virtual string	GetMyAddress(void) = 0;
};

// Generic class of the outputs
class COutput
{
public:
	virtual ~COutput(){};

	virtual int Connect(void) = 0;
	virtual int Insert(DataContainer &Data) = 0;
	virtual int Insert(DataContainerList &DataList) = 0;

	virtual int Restore(DataContainer &Data) = 0;
	virtual int Restore(DataContainerList &DataList, unsigned int maxRows) = 0;
};

// Structure that contains the data to push to the queue
struct MsgStruct
{
	uint8_t 	Data[256];		// Message data
	uint16_t 	DataLen;		// Message length
	uint32_t 	TimeStamp;		// Timestamp of message arrival
	CInterface 	*Iface;			// The serial interface that received the message
};

struct ThreadStruct
{
	CInterface *interface;
	list<struct MsgStruct> *MsgQueue;
	pthread_mutex_t *queueMutex;

	list<int> *connected;
};



string int2string(uint8_t num);
string int2string(uint16_t num);
string int2string(uint32_t num);
string int2string(uint64_t num);
string int2string(int8_t num);
string int2string(int16_t num);
string int2string(int32_t num);
string int2string(int64_t num);
string int2string(time_t num);
string float2string(float num);

#endif /* FPD_H_ */
