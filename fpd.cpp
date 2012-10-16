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
#include "CInterface.h"
#include "CScanner.h"

bool debugFlag = false;
CLog Logger;


bool SigTermFlag = false;

pthread_t RestoringThread;
pthread_t DbConnectThread;

void* RestoringFunction(void *ptr);
void* DbConnectFunction(void *ptr);

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

    Logger.Init();

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

    // Mysql Connection object
	CMysqlDb SolarSpyDb;
	// XML Connection object. Used for local caching if mysql fails
	CXMLDb BackupDb;

	string path = stripPathFromArgv(argv[0]);
	path = path + "/" + "backup.xml";
	BackupDb.Connect(path);

	RestoringThread = 0;
	DbConnectThread = 0;
	RestoringFinished = true;

	// If we connect, then everything is ok, libmysql handles reconnections automatically.
	// If we don't connect, we have to spawn a thread to keep trying reconnecting

	char host[] = HOST;
	int	 port = PORT;
	char user[] = USER;
	char pass[] = PASS;
	char db[] = DB;


	RestoreThreadStruct ThreadStruct;
	ThreadStruct.MysqlDb = &SolarSpyDb;
	ThreadStruct.XMLDb = &BackupDb;

	if(!SolarSpyDb.Connect(host, port, user, pass, db))
	{
		syslog(LOG_ERR, "Error Connecting to database %s on %s:%d",db, host, port);
		pthread_create( &DbConnectThread, NULL, &DbConnectFunction,  &SolarSpyDb);
	}
	else
	{
		syslog(LOG_INFO, "Connected to database %s on %s:%d",db, host, port);



		// Start restoring any backup data
		syslog(LOG_INFO, "Uploading any pending data...");
		pthread_create( &RestoringThread, NULL, &RestoringFunction,  &ThreadStruct);
		if(debugFlag == true)
			syslog(LOG_INFO, "Restoring Thread started\n");
	}

	CScanner Scanner;
	CProbe *probe;

	// Send the scanner to find a network of probes and return a live probe
	while((probe = Scanner.FindNetwork()) == 0)
	{
		syslog(LOG_ERR, "Cannot Find Any Inverter Network. Sleeping for 10 secs");

		for(int w=0; w < 10; w++)
		{
			if(SigTermFlag == true)
						return 0;
			sleep(1);
		}
	}

	syslog(LOG_INFO, "Initialized Successfully");

	map<int, int> ConsecutiveErrors;

	while(true)
	{
		bool DBState = true;
		time_t TimeStamp = time(NULL);
		struct tm *ptr;
		ptr = localtime((const time_t *) &TimeStamp);

		if(ptr->tm_sec == 0)
		{

			list<int> ConnectedProbes;
			ConnectedProbes = probe->GetConnectedInverters();

			list<int>::iterator currentProbe;
			for(currentProbe=ConnectedProbes.begin();currentProbe!=ConnectedProbes.end(); ++currentProbe)
			{
				DataContainer Results;

				Results["clientID"] = UUID;
				Results["TimeStamp"] = int2string(TimeStamp);
				Results["inverter"] = int2string(*currentProbe);

				// By default, start with non-responsive status and work the way from there
				Results["status"] = "5";

				if(!ConsecutiveErrors[*currentProbe])
					ConsecutiveErrors[*currentProbe] = 0;

				// Try to get the probe results.
				// If it fails, flag it as non responsive (and discard it...)
				// On 5 errors send it to db anyway

				if(probe->GetAverage(Results) == false)
				{
					syslog(LOG_ERR, "Non responsive Inverter\n");
					ConsecutiveErrors[*currentProbe]++;
				}
				else
					ConsecutiveErrors[*currentProbe] = 0;

				if(ConsecutiveErrors[*currentProbe] == 0 || ConsecutiveErrors[*currentProbe] > 5)
				{
					// If the last db access returned an error, send it straight to
					// backup and don't lose time trying again. Will try again on next
					// probe
					if(DBState == true && SolarSpyDb.Insert(Results))
					{

							// If we had a successful database insertion, the reconnect thread did its job.
							// Stop it...
							if(DbConnectThread)
							{
								pthread_cancel(DbConnectThread);
								DbConnectThread = 0;
							}

							if(RestoringFinished == true)
							{
								if(RestoringThread)
								{
									pthread_join(RestoringThread, NULL);
									RestoringThread = 0;
								}
								pthread_create( &RestoringThread, NULL, &RestoringFunction,  &ThreadStruct);
								if(debugFlag == true)
									syslog(LOG_INFO, "Restoring Thread started\n");
							}

							DBState = true;
					}
					else
					{
						syslog(LOG_ERR, "Error Inserting data to solarspy\n");

						DBState = false;

						if(!BackupDb.Insert(Results))
							syslog(LOG_ERR, "Error inserting data to XML\n");
					}
				}
			}
			probe->ResetStack();
			sleep(1);
		}
		usleep(30000);
		if(SigTermFlag == true)
			break;
	}

	pthread_join(RestoringThread, NULL);
	delete probe;
	return 0;
}

void* RestoringFunction(void *ptr)
{
	RestoringFinished = false;
	RestoreThreadStruct *ThreadStruct = (RestoreThreadStruct *) ptr;

	while(true)
	{
		DataContainerList Data2Restore;

		if(ThreadStruct->XMLDb->Restore(Data2Restore, 100) == false)
		{
			syslog(LOG_ERR, "Error Reading from XML file\n");
			break;
		}

		if(Data2Restore.size() == 0)
			break;

		if(ThreadStruct->MysqlDb->Insert(Data2Restore) == false)
		{
			syslog(LOG_ERR, "Error Inserting data to database\n");
			break;
		}
	}

	RestoringFinished = true;
	pthread_exit(NULL);

	return NULL;
}

void* DbConnectFunction(void *ptr)
{
	char host[] = HOST;
	char user[] = USER;
	char pass[] = PASS;
	char db[] = DB;

	CMysqlDb *RestoreDb = (CMysqlDb *) ptr;
	if(!RestoreDb->Connect(host, 65020, user, pass, db))
	{
		sleep(30);
		syslog(LOG_INFO, "Trying to connect to db\n");
	}
	syslog(LOG_INFO, "Connected to db\n");
	pthread_exit(NULL);

	return NULL;
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
