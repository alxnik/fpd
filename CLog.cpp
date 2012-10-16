/*
 * Copyright (c) 2012 Alexandros Nikolopoulos <alxnik@gmail.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.

 */

#include "CLog.h"
#include "fpd.h"
#include <syslog.h>

CLog::CLog()
{
}

void
CLog::Init()
{
	if(debugFlag == true)
		openlog("fpd", LOG_PERROR, LOG_LOCAL7);
	else
		openlog("fpd", 0, LOG_LOCAL7);
}

CLog::~CLog()
{
	closelog();
}

void
CLog::hexDump(uint8_t *msg, int len)
{
	for(int i=0; i<len; i++)
		fprintf(stderr, "[%x]", *(msg+i));
	fprintf(stderr, "\n");
}
