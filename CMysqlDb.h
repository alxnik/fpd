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
using namespace std;
using namespace tinyxml2;

#define INSERT_PER_QUERY 10

class CMysqlDb {
public:
	CMysqlDb();
	virtual ~CMysqlDb();
	// Database connection functions
	int Connect(char *host, unsigned int port, char *user, char *pass, char *db);

	// Database insertion functions
	int Insert(DataContainer &Data);
	int Insert(DataContainerList &DataList);

	int InsertStatic(uint8_t inverter, uint8_t *verString, uint8_t DeviceType, uint8_t Capabilities, uint32_t serial);

	// In case of error, backup to XML
	int BackupDynamic(struct PollStruct &polldata);
	// In case of resolve of the error, restore from XML
	int RestoreDynamic(void);

	int Test(void);

private:

	pthread_mutex_t queryMutex;


	MYSQL			m_conn;
	char 			*m_host;
	unsigned int	m_port;
	char			*m_user;
	char			*m_pass;
	char			*m_db;
};

#endif /* CMysqlDb_H_ */
