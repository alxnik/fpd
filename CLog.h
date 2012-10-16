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

#include "fpd.h"

class CLog {
public:
	CLog();
	~CLog();

	void Init(void);
	void hexDump(uint8_t *msg, int len);
};

#endif /* CLOG_H_ */
