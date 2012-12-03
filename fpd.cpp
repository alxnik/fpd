/*
 * Copyright (c) 2012 Alexandros Nikolopoulos <alxnik@gmail.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.

 */

#include "fpd.h"
#include "CMysqlDb.h"
#include "CXMLDb.h"
#include "CLog.h"
#include "CSerial.h"
#include "CSocket.h"
#include "CScanner.h"
#include "CSolutronicProbe.h"
#include "CSettings.h"
#include "CSmaProbe.h"

#include <signal.h>

CLog Log;
bool debugFlag = false;
bool SigTermFlag = false;

pthread_t RestoringThread;
pthread_t DbConnectThread;

bool RestoringFinished;

string stripPathFromArgv(char *argv);
void handler(int value);

struct RestoreThreadStruct
{
	CMysqlDb *MysqlDb;
	CXMLDb *XMLDb;
};

void handler(int value)
{
	syslog(LOG_INFO, "caught SIGTERM, shutting down operations");
	SigTermFlag = true;
}



COutput* StartOutput(OutputContainer *outp)
{

	COutput *out = 0;
	if(outp->type.compare("mysql"))
	{
		CMysqlDb *Mysql = new CMysqlDb(outp->settings);
		Mysql->Connect();
		out = dynamic_cast<COutput *> (Mysql);
	}
	else if(outp->type.compare("xml"))
	{
		CXMLDb *xml = new CXMLDb(outp->settings);
		xml->Connect();
		out = dynamic_cast<COutput *> (xml);
	}
	else
	{
		syslog(LOG_ERR, "Unsupported output requested\n");
	}

	return out;
}


CInterface* StartInterface(InputContainer *inp)
{
	CInterface *iface = 0;

	if(!inp->iface.compare("serial"))
	{
		CSerial *Serial = new CSerial(inp->ifaceSettings);
		iface = dynamic_cast<CInterface *> (Serial);
	}
	else if(!inp->iface.compare("socket"))
	{
		CSocket *sock = new CSocket(inp->ifaceSettings);
		iface = dynamic_cast<CInterface *> (sock);
	}
	else if(!inp->iface.compare("bluetooth"))
	{
		CBlueTooth *bt = new CBlueTooth(inp->ifaceSettings);
		iface = dynamic_cast<CInterface *> (bt);
	}
	else
	{
		syslog(LOG_ERR, "Unsupported Interface requested\n");
	}

	return iface;
}

CProbe* StartProbe(InputContainer *inp, CInterface* iface)
{
	CProbe *Probe = 0;
	if(!inp->type.compare("fronius"))
	{
		CFroniusProbe *FProbe = new CFroniusProbe(iface, inp->uuid);
		Probe = dynamic_cast<CProbe *> (FProbe);
	}
	else if(!inp->type.compare("sunergy"))
	{
		//CSunergyProbe *SProbe = new CSunergyProbe(iface, CurInput->uuid);
		//Probe = dynamic_cast<CProbe *> (SProbe);
	}
	else if(!inp->type.compare("sma"))
	{
		//CSmaProbe *SProbe = new CSmaProbe(iface, CurInput->uuid);
		//Probe = dynamic_cast<CProbe *> (SProbe);
	}
	else if(!inp->type.compare("solutronic"))
	{
		CSolutronicProbe *SProbe = new CSolutronicProbe(iface, inp->sensors, inp->uuid);
		Probe = dynamic_cast<CProbe *> (SProbe);
	}
	else
	{
		syslog(LOG_ERR, "Unsupported Probe requested\n");
	}

	return Probe;
}


int main(int argc, char *argv[])
{
	pid_t sid;
	struct sigaction new_action;


    int c;
    opterr = 0;
    while ((c = getopt (argc, argv, "d")) != -1)
    {
    	if(c == 'd')
    		debugFlag = true;
    }

    Log.Init();

    // VER is set by the makefile and contains the svn revision number
#ifdef VER
    string Revision = VER;
#else
    string Revision = "Unknown";
#endif

    syslog(LOG_INFO, "SolarSpy Probe Daemon %d.%d (rev %s) Starting...", MAJOR_VERSION, MINOR_VERSION, Revision.c_str());

    // Daemonize!
    if(debugFlag == false)
    {
    	// fork the process into the background
    	int rv = fork();
    	if(rv < 0)
    		return -1;
    	else if(rv > 0)
    		return 0;

    	umask(0);

    	// Get a new group ID
    	sid = setsid();
    	if (sid < 0)
    	{
    		syslog(LOG_ERR, "Cannot set SID");
    		return -1;
    	}

    	// Close all contact with the world
    	close(STDIN_FILENO);
    	close(STDOUT_FILENO);
    	close(STDERR_FILENO);
    }

    // Register signal handlers for proper termination of the daemon - sigterm & ctrl-C
    new_action.sa_handler = handler;
    sigemptyset (&new_action.sa_mask);
    new_action.sa_flags = 0;

    if( sigaction (SIGINT, &new_action, NULL) == -1 || sigaction (SIGTERM, &new_action, NULL) == -1)
    {
    	syslog(LOG_ERR, "Failed to register Signal Handlers, Exiting");
    	return(EXIT_FAILURE);
    }


    // Now start the actual application
    CSettings Settings;

    if(!Settings.LoadFile("settings.xml"))
    {
        syslog(LOG_ERR, "Error reading settings\n");
		return(EXIT_FAILURE);
    }

    syslog(LOG_INFO, "Loaded Settings file\n");
    list<CProbe*> Inputs;
    list<COutput*> Outputs;


    // Parse and start the outputs
	for(list<OutputContainer>::iterator CurOutput=Settings.OutputsDB.begin();CurOutput!=Settings.OutputsDB.end(); ++CurOutput)
	{
		COutput *out = StartOutput(&(*CurOutput));
		if(out)
			Outputs.push_back(out);
		else
		{
			syslog(LOG_ERR, "Failed to start output [%s]\n", CurOutput->name.c_str());
			continue;
		}
	}

    // Parse and start the inputs
	for(list<InputContainer>::iterator CurInput=Settings.InputsDB.begin();CurInput!=Settings.InputsDB.end(); ++CurInput)
	{
		// Start The interface first
		CInterface *iface = StartInterface(&(*CurInput));

		if(!iface || !iface->Connect())
		{
			syslog(LOG_ERR, "Failed to start interface [%s]\n", CurInput->uuid.c_str());
			continue;
		}

		// And pass it to the probe
		CProbe *Probe = StartProbe(&(*CurInput),iface);
		if(!Probe || !Probe->Start())
		{
			syslog(LOG_ERR, "Failed to start probe [%s]\n", CurInput->uuid.c_str());
			delete iface;
			continue;
		}
	}
	sleep(5);

	return 0;
}


string int2string(uint8_t num)
{
	stringstream strstm;
	unsigned int NumCast = num;
	strstm <<  NumCast;
	return strstm.str();
}
string int2string(uint16_t num)
{
	stringstream strstm;
	unsigned int NumCast = num;
	strstm <<  NumCast;
	return strstm.str();
}
string int2string(uint32_t num)
{
	stringstream strstm;
	unsigned int NumCast = num;
	strstm <<  NumCast;
	return strstm.str();
}
string int2string(uint64_t num)
{
	stringstream strstm;
	strstm << num;
	return strstm.str();
}
string int2string(int8_t num)
{
	stringstream strstm;
	int NumCast = num;
	strstm <<  NumCast;
	return strstm.str();
}
string int2string(int16_t num)
{
	stringstream strstm;
	int NumCast = num;
	strstm <<  NumCast;
	return strstm.str();
}
string int2string(int32_t num)
{
	stringstream strstm;
	strstm << num;
	return strstm.str();
}
string int2string(int64_t num)
{
	stringstream strstm;
	strstm << num;
	return strstm.str();
}

#if __WORDSIZE != 64
string int2string(time_t num)
{
	stringstream strstm;
	strstm << num;
	return strstm.str();
}
#endif

string float2string(float num)
{
	stringstream strstm;
	strstm << num;
	return strstm.str();
}

string stripPathFromArgv(char *argv)
{
	char *lastSlash, *Slash;
	char path[256];

	Slash = argv;
	do
	{
		lastSlash = Slash;
		Slash = strstr(lastSlash+1, "/");
	} while(Slash);

	strncpy(path, argv, lastSlash-argv);
	path[lastSlash-argv+1] = 0x0;
	return string(path);
}
