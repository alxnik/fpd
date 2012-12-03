/*
 * Copyright (c) 2012 Alexandros Nikolopoulos <alxnik@gmail.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.

 */

#ifndef CSCANNER_H_
#define CSCANNER_H_

#include "CFroniusProbe.h"
#include "CSunergyProbe.h"
#include "CSmaProbe.h"

#include "CModbusMTU.h"
#include "CBlueTooth.h"

#include <libudev.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include <iostream>
#include <string>
using namespace std;

class CScanner {
public:
	CScanner();
	virtual ~CScanner();

	CProbe* FindNetwork(void);

private:
	CProbe* FindFronius(list<string> &SerialPorts);
	CProbe* FindSunergy(list<string> &SerialPorts);
	CProbe*	FindSMABlueTooth(list<int> &BlueDevices);

	list<string> EnumSerialPorts(void);

	list<int> EnumBlueTooth(void);
};

#endif /* CSCANNER_H_ */
