/*
 * Copyright (c) 2012 Alexandros Nikolopoulos <alxnik@gmail.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#ifndef CSETTINGS_H_
#define CSETTINGS_H_


#include <list>
#include <map>
#include <string>
#include <sstream>
using namespace std;


#include "tinyxml/tinyxml2.h"
using namespace tinyxml2;

#include "fpd.h"

typedef struct
{
	string		uuid;
	string		type;
	string		iface;

	map<string, string>		ifaceSettings;
	list<int> 				sensors;
	map<int, string>		outputs;
} InputContainer;

typedef struct
{
	string 	name;
	string	type;
	map<string,string>	settings;
} OutputContainer;


class CSettings {
public:
	CSettings();
	virtual ~CSettings();

	int LoadFile(string filename);
	list<InputContainer> 	InputsDB;
	list<OutputContainer> 	OutputsDB;

private:
	string m_FileName;
	XMLDocument *m_SettingsFile;
	pthread_mutex_t FileMutex;


	int OpenFile(void);

	int ParseInputs(XMLElement *Inputs);
	int ParseOutputs(XMLElement *Outputs);

};

#endif /* CSETTINGS_H_ */
