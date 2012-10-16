/*
 * Copyright (c) 2012 Alexandros Nikolopoulos <alxnik@gmail.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.

 */

#include "CMysqlDb.h"

CMysqlDb::CMysqlDb()
{
	m_host = m_user = m_pass = m_db = 0;
	m_port =0;
   	mysql_init(&m_conn);
   	pthread_mutex_init(&queryMutex, NULL);
}

CMysqlDb::~CMysqlDb()
{
	if(m_host) free(m_host);
	if(m_user) free(m_user);
	if(m_pass) free(m_pass);
	if(m_db) free(m_db);

	mysql_close(&m_conn);
}

int
CMysqlDb::Connect(char *host, unsigned int port, char *user, char *pass, char *db)
{
	if(!host || !user || !pass || !db)
		return -1;
	if(!(port > 0 && port < 65536))
		return -1;

	pthread_mutex_lock(&queryMutex);

	mysql_close(&m_conn);
	mysql_init(&m_conn);
   	m_host = (char*) malloc(strlen(host)+1);
   	strcpy(m_host, host);
   	m_user = (char*) malloc(strlen(user)+1);
   	strcpy(m_user, user);
   	m_pass = (char*) malloc(strlen(pass)+1);
   	strcpy(m_pass, pass);
   	m_db = (char*) malloc(strlen(db)+1);
   	strcpy(m_db, db);

   	m_port = port;

   	mysql_options(&m_conn,MYSQL_OPT_COMPRESS,0);

   	unsigned int connectTimeout = 10;
   	mysql_options(&m_conn, MYSQL_OPT_CONNECT_TIMEOUT, &connectTimeout);

   	unsigned int timeout = 5;
   	mysql_options(&m_conn, MYSQL_OPT_READ_TIMEOUT, &timeout);
   	mysql_options(&m_conn, MYSQL_OPT_WRITE_TIMEOUT, &timeout);

   	my_bool MyTrue = 1;
   	mysql_options(&m_conn, MYSQL_OPT_RECONNECT, &MyTrue);

   	mysql_options(&m_conn, MYSQL_READ_DEFAULT_FILE, "./fpd.cnf");
   	mysql_options(&m_conn, MYSQL_READ_DEFAULT_GROUP, "fpd");

   	/* Connect to database */
   	if (!mysql_real_connect(&m_conn, host, user, pass, db, port, NULL, CLIENT_REMEMBER_OPTIONS))
   	{
   		syslog(LOG_ERR, "Error connecting to %s: %s", host, mysql_error(&m_conn));
   		pthread_mutex_unlock(&queryMutex);
   		return false;
   	}
   	else
   	{
   		pthread_mutex_unlock(&queryMutex);
   		return true;
   	}
}


int
CMysqlDb::Insert(DataContainer &Data)
{
	DataContainerList DataList;
	DataList.push_back(Data);
	return Insert(DataList);
}

int
CMysqlDb::Insert(DataContainerList &DataList)
{
	if(DataList.size() == 0)
		return true;

	while (!DataList.empty())
	{
		string ColumnsString = "(";
		string ValuesString = "(";
		DataContainer Data = DataList.front();
		DataContainer::iterator LastElement = Data.end();
		--LastElement;

		for( DataContainer::iterator ii=Data.begin(); ii!=Data.end(); ++ii)
		{
			ColumnsString += (*ii).first;
			ValuesString += (*ii).second;

			if(ii == LastElement)
			{
				ColumnsString += ")";
				ValuesString += ")";
			}
			else
			{
				ColumnsString += ",";
				ValuesString += ",";
			}
		}

		string query = "INSERT IGNORE INTO InverterData " + ColumnsString + " VALUES " + ValuesString + ";";
		pthread_mutex_lock(&queryMutex);

		/* send SQL query */
		if (mysql_query(&m_conn, query.c_str()))
		{
			int error = mysql_errno(&m_conn);
			pthread_mutex_unlock(&queryMutex);
			syslog(LOG_ERR, "MySQL Error (%d): %s", error, mysql_error(&m_conn));

			// Server error (fatal). Most probably because of a bug or something like that
			// Discard the entry so the program doesn't stop sending data because of it
			if(error >= 1000 && error < 2000)
			{
				DataList.pop_front();
				continue;
			}
			// Client error (transient). Do not delete the results and retry sending them
			else
				return false;
		}
		else
			pthread_mutex_unlock(&queryMutex);

		DataList.pop_front();
	}

	return true;
}

int
CMysqlDb::InsertStatic(uint8_t inverter, uint8_t *verString, uint8_t DeviceType, uint8_t Capabilities, uint32_t serial)
{
	pthread_mutex_lock(&queryMutex);

	MYSQL_ROW row;
	MYSQL_RES *result;

	char query[2048];
	sprintf(query, 	"SELECT entryID FROM inverters WHERE"\
			" inverterID=%d AND UniqueID=\"%X\" AND model=%d AND effectiveReduction=%d AND reactiveReduction=%d"\
			" ORDER BY entryID DESC LIMIT 1;",
			inverter, serial, DeviceType, (uint8_t) Capabilities & 0x01, (uint8_t) Capabilities & 0x02);


	/* send SQL query */
	if (mysql_query(&m_conn, query))
	{
		syslog(LOG_ERR, "%s", mysql_error(&m_conn));
		return false;
	}

	result = mysql_store_result(&m_conn);
	time_t TimeStamp = time(NULL);
	row = mysql_fetch_row(result);
	if(!row)
	{
	    sprintf(query,  "INSERT INTO inverters (TimeStamp, inverterID, UniqueID, model, effectiveReduction, reactiveReduction)"\
                        	" VALUES (%lu, %u, \"%X\", %u, %u, %u);",
			TimeStamp, inverter, serial, DeviceType, (uint8_t) Capabilities & 0x01, (uint8_t) Capabilities & 0x02);

		if (mysql_query(&m_conn, query))
		{
			syslog(LOG_ERR, "%s", mysql_error(&m_conn));
			return false;
		}

	}
	else
	{
		sprintf(query, "UPDATE inverters SET TimeStamp=%lu WHERE entryID=%s;", TimeStamp, row[0]);
		if (mysql_query(&m_conn, query))
		{
			syslog(LOG_ERR, "%s", mysql_error(&m_conn));
			return false;
		}

	}
	return true;
}

int
CMysqlDb::Test(void)
{
	if(mysql_query(&m_conn, "SELECT * FROM user;"))
	{
		printf("Error: %s\n", mysql_error(&m_conn));
		return mysql_errno(&m_conn);
	}

	mysql_free_result(mysql_use_result(&m_conn));
	return true;
}
