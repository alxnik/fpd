/*
 * Copyright (c) 2012 Alexandros Nikolopoulos <alxnik@gmail.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.

 */

#include "CMysqlDb.h"

CMysqlDb::CMysqlDb(map<string, string> settings)
{
	m_host 	= settings["host"];
	m_user 	= settings["user"];
	m_pass 	= settings["pass"];
	m_db 	= settings["db"];
	m_table = settings["table"];
	m_port 	= atoi(settings["port"].c_str());

   	mysql_init(&m_conn);
   	pthread_mutex_init(&queryMutex, NULL);
}

CMysqlDb::~CMysqlDb()
{
	mysql_close(&m_conn);
}

int
CMysqlDb::Connect(void)
{
	if(m_host.length() == 0 || m_user.length() == 0 || m_pass.length() == 0 || m_db.length() == 0)
		return false;

	if(!(m_port > 0 && m_port < 65536))
		return false;

	pthread_mutex_lock(&queryMutex);

	mysql_close(&m_conn);
	mysql_init(&m_conn);

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
   	if (!mysql_real_connect(&m_conn, m_host.c_str(), m_user.c_str(), m_pass.c_str(), m_db.c_str(), m_port, NULL, CLIENT_REMEMBER_OPTIONS))
   	{
   		syslog(LOG_ERR, "Error connecting to %s: %s", m_host.c_str(), mysql_error(&m_conn));
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

		string query = "INSERT IGNORE INTO " + m_table + " " + ColumnsString + " VALUES " + ValuesString + ";";
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
