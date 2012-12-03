/*
 * Copyright (c) 2012 Alexandros Nikolopoulos <alxnik@gmail.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.

 */

#ifndef CXMLDB_H_
#define CXMLDB_H_

#include "fpd.h"
#include "tinyxml/tinyxml2.h"

using namespace tinyxml2;
using namespace std;

class CXMLDb : public COutput
{
public:
	CXMLDb(map<string, string> settings);
	virtual ~CXMLDb();

	int Connect(void);

	// Insert one row
	int Insert(DataContainer &Data);
	// Insert multiple rows
	int Insert(DataContainerList &DataList);

	// Restore one row
	int Restore(DataContainer &Data);
	// Restore multiple rows
	int Restore(DataContainerList &DataList, unsigned int maxRows);

private:
	string m_FileName;
	XMLDocument *m_BackupFile;
	pthread_mutex_t FileMutex;

	int LoadBackupFile(void);
	int SaveBackupFile(void);
	int CreateBackupFile(void);

};

#endif /* CXMLDB_H_ */
