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

#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <mysql/mysql.h>
#include <mysql/errmsg.h>
#include <syslog.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

#include <list>
#include <map>
#include <string>
#include <sstream>
using namespace std;

#define MAJOR_VERSION 2
#define MINOR_VERSION 5

#include "CInterface.h"
#include "settings.h"

extern bool debugFlag;

typedef map<string, string> DataContainer;
typedef list<DataContainer> DataContainerList;


class CProbe
{

public:
	virtual ~CProbe(void){};

	virtual int Start(void) = 0;
	virtual int Stop(void) = 0;
	virtual int GetAverage(DataContainer &AverageData) = 0;
	virtual list<int> GetConnectedInverters(void) = 0;
	virtual int ResetStack(void) = 0;

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
