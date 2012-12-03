/*
 * Copyright (c) 2012 Alexandros Nikolopoulos <alxnik@gmail.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.

 */

#ifndef CMysqlDb_H_
#define CMysqlDb_H_

#include "fpd.h"
#include "tinyxml/tinyxml2.h"
#include <iostream>
#include <string>
#include <mysql/mysql.h>
#include <mysql/errmsg.h>

using namespace std;

#define INSERT_PER_QUERY 10

class CMysqlDb : public COutput
{
public:
	CMysqlDb(map<string, string> settings);
	virtual ~CMysqlDb();
	// Database connection functions
	int Connect(void);

	// Database insertion functions
	int Insert(DataContainer &Data);
	int Insert(DataContainerList &DataList);

	int Restore(DataContainer &Data) {return false;};
	// Restore multiple rows
	int Restore(DataContainerList &DataList, unsigned int maxRows) {return false;};

private:

	pthread_mutex_t queryMutex;


	MYSQL			m_conn;
	string 			m_host;
	uint16_t		m_port;
	string			m_user;
	string			m_pass;
	string			m_db;
};

#endif /* CMysqlDb_H_ */
