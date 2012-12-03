/*
 * Copyright (c) 2012 Alexandros Nikolopoulos <alxnik@gmail.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.

 */

#ifndef CLOG_H_
#define CLOG_H_

#include <sys/types.h>
#include <syslog.h>
//#include "fpd.h"

#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

#include <list>
#include <map>
#include <string>
#include <sstream>
using namespace std;

extern bool debugFlag;

class CLog {
public:
	CLog();
	~CLog();

	void Init(void);
	void hexDump(int priority, uint8_t *msg, int len);

	void error(const char *format, ...);
	void warn(const char *format, ...);
	void log(const char *format, ...);
	void debug(const char *format, ...);
};

#endif /* CLOG_H_ */

