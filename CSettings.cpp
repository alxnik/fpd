/*
 * Copyright (c) 2012 Alexandros Nikolopoulos <alxnik@gmail.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include "CSettings.h"
#include "CLog.h"

CSettings::CSettings()
{
	m_SettingsFile = 0;
	pthread_mutex_init(&FileMutex, NULL);
}

CSettings::~CSettings()
{
}

int
CSettings::LoadFile(string filename)
{
	m_FileName = filename;

	if(!OpenFile())
		return false;

	XMLElement *Main = m_SettingsFile->FirstChildElement("fpd");
	if(!Main)
		return false;

	XMLElement *Inputs = Main->FirstChildElement("inputs");
	XMLElement *Outputs = Main->FirstChildElement("outputs");
	if(!Inputs || !Outputs)
		return false;

	if(!ParseInputs(Inputs))
		return false;
	if(!ParseOutputs(Outputs))
		return false;

	return true;
}

int
CSettings::ParseInputs(XMLElement *Inputs)
{
	// Parse Inputs
	XMLElement *Input = Inputs->FirstChildElement("input");

	int NumberOfInputs = 0;
	while(Input)
	{
		InputContainer InputParse;
		NumberOfInputs++;

		if(!Input->Attribute("type") || !Input->Attribute("UUID"))
			return false;

		InputParse.uuid = Input->Attribute("UUID");
		InputParse.type = Input->Attribute("type");

		XMLElement *Interface = Input->FirstChildElement("iface");
		if(!Interface || !Interface->Attribute("type"))
			return false;

		InputParse.iface = Interface->Attribute("type");

		XMLNode *iSets = Interface->FirstChild();
		while(iSets)
		{
			if(iSets->Value() || iSets->ToElement()->GetText())
			{
				InputParse.ifaceSettings[iSets->Value()] = iSets->ToElement()->GetText();
				Log.debug("parsed [%s]=[%s]", iSets->Value(), InputParse.ifaceSettings[iSets->Value()].c_str());
			}

			iSets = iSets->NextSibling();
		}



		XMLElement *Sensor = Input->FirstChildElement("sensors")->FirstChildElement("sensor");
		int NumOfSensors = 0;
		while(Sensor)
		{
			if(!Sensor->Attribute("address"))
				return false;
			NumOfSensors++;
			InputParse.sensors.push_back(atol(Sensor->Attribute("address")));
			Sensor = Sensor->NextSiblingElement("sensor");
		}

		XMLElement *Output = Input->FirstChildElement("output");
		if(!Output || !Output->Attribute("name"))
			return false;

		InputParse.output = Output->Attribute("name");
		if(Output->Attribute("cache"))
			InputParse.cache = Output->Attribute("cache");
		else
			Log.warn("No cache db set. This is not recommended\n");

		InputsDB.push_back(InputParse);

		Log.debug("Parsed %s (%s - %s) with %d sensors\n", InputParse.uuid.c_str(), InputParse.type.c_str(),
				InputParse.iface.c_str(), NumOfSensors);

		Input = Input->NextSiblingElement("input");
	}
	Log.debug("Parsed %d inputs\n", NumberOfInputs);

	return true;
}

int
CSettings::ParseOutputs(XMLElement *Outputs)
{
	// Parse Outputs
	XMLElement *Output = Outputs->FirstChildElement("output");

	int NumberOfOutputs = 0;
	while(Output)
	{
		OutputContainer OutputParse;
		NumberOfOutputs++;

		if(!Output->Attribute("name"))
		{
			Log.error("Output entry has no name\n");
			return false;
		}

		// Check if there is a duplicate name
		OutputParse.name = Output->Attribute("name");
		for(list<OutputContainer>::iterator i=OutputsDB.begin(); i != OutputsDB.end(); ++i)
		{
			if(OutputParse.name.compare((*i).name) == 0)
			{
				Log.error("Duplicate output name [%s]\n", OutputParse.name.c_str());
				return false;
			}
		}


		OutputParse.type = Output->Attribute("type");

		XMLNode *OutSettings = Output->FirstChild();

		while(OutSettings)
		{
			if(OutSettings->Value() && OutSettings->ToElement()->GetText())
			{
				OutputParse.settings[OutSettings->Value()] = OutSettings->ToElement()->GetText();
				Log.debug("parsed [%s]=[%s]", OutSettings->Value(), OutputParse.settings[OutSettings->Value()].c_str());
			}

			OutSettings = OutSettings->NextSibling();
		}

		OutputsDB.push_back(OutputParse);

		Output = Output->NextSiblingElement("output");
	}


	Log.log("Parsed %d outputs\n", NumberOfOutputs);

	return true;
}

int
CSettings::OpenFile(void)
{
	if(m_SettingsFile)
		return true;

	m_SettingsFile = new XMLDocument();

	if(m_SettingsFile->LoadFile(m_FileName.c_str()) != XML_NO_ERROR)
	{
		Log.error("Error Opening %s: %d\n", m_FileName.c_str(), m_SettingsFile->ErrorID());

		delete m_SettingsFile;
		m_SettingsFile = 0;
		return false;
	}

	return true;
}
