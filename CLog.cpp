/*
 * Copyright (c) 2012 Alexandros Nikolopoulos <alxnik@gmail.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.

 */

#include "CLog.h"
#include <stdarg.h>

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
CLog::hexDump(int priority, uint8_t *msg, int len)
{
    uint8_t AsciiBuff[2048];
    memset(AsciiBuff, 0x0, sizeof(AsciiBuff));
    int p = 0;
    for(int i=0; i<len; i++)
    {
            sprintf((char *) &AsciiBuff[p], "%02x ", *(msg+i));
            p = strlen((char *) AsciiBuff);
            if(i > 0 && i % 0x10 == 0)
                    sprintf((char *) &AsciiBuff[p], "\n");
            p = strlen((char *) AsciiBuff);
    }
    syslog(priority, "\n%s\n", AsciiBuff);
}

void
CLog::error(const char *format, ...)
{
	va_list ap;
	va_start ( ap, format );
	vsyslog(LOG_ERR, format, ap);
	va_end(ap);
}

void
CLog::warn(const char *format, ...)
{
	va_list ap;
	va_start ( ap, format );
	vsyslog(LOG_WARNING, format, ap);
	va_end(ap);
}

void
CLog::log(const char *format, ...)
{
	va_list ap;
	va_start ( ap, format );
	vsyslog(LOG_INFO, format, ap);
	va_end(ap);
}

void
CLog::debug(const char *format, ...)
{
	va_list ap;
	va_start ( ap, format );
	vsyslog(LOG_DEBUG, format, ap);
	va_end(ap);
}
