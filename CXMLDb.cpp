/*
 * Copyright (c) 2012 Alexandros Nikolopoulos <alxnik@gmail.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.

 */

#include "CXMLDb.h"

CXMLDb::CXMLDb() {
	m_BackupFile = 0;
	pthread_mutex_init(&FileMutex, NULL);
}

CXMLDb::~CXMLDb() {
}

int
CXMLDb::Connect(string FileName) {

	m_FileName = FileName;
	return LoadBackupFile();
}

int
CXMLDb::Insert(DataContainer &Data)
{

	list<map<string, string> > DataList;
	DataList.push_back(Data);
	return Insert(DataList);
}

int
CXMLDb::Insert(DataContainerList &DataList)
{
	pthread_mutex_lock(&FileMutex);

	if(!LoadBackupFile())
	{
		pthread_mutex_unlock(&FileMutex);
		return false;
	}

	while (!DataList.empty())
	{
		DataContainer Data = DataList.front();

		syslog(LOG_INFO, "Pushing element\n");
		XMLNode *ProbeData = m_BackupFile->NewElement("pd");

		for( DataContainer::iterator ii=Data.begin(); ii!=Data.end(); ++ii)
		{
			string name = (*ii).first;
			string value = (*ii).second;
			ProbeData->ToElement()->SetAttribute(name.c_str(),value.c_str());
		}

		m_BackupFile->FirstChildElement()->InsertEndChild(ProbeData);
		DataList.pop_front();
	}

	if(!SaveBackupFile())
	{
		pthread_mutex_unlock(&FileMutex);
		return false;
	}

	pthread_mutex_unlock(&FileMutex);
	return true;
}

// Restores the cached entries to the database. It's spawned off as a seperate
int
CXMLDb::Restore(DataContainerList &DataList, unsigned int maxRows)
{
	pthread_mutex_lock(&FileMutex);

	if(!LoadBackupFile())
	{
		syslog(LOG_ERR, "Error opening Backup file\n");
		pthread_mutex_unlock(&FileMutex);
		return false;
	}

	XMLElement *CurrentElement = m_BackupFile->FirstChildElement()->FirstChildElement("pd");

	// file is empty. Return true
	if(!CurrentElement)
	{
		pthread_mutex_unlock(&FileMutex);
		return true;
	}

	std::list<struct XMLElement*> XmlList;

	while(CurrentElement && DataList.size() < maxRows)
	{
		DataContainer CurrentData;

		const XMLAttribute *CurrentAtt = CurrentElement->FirstAttribute();

		while(CurrentAtt)
		{
			string Name = CurrentAtt->Name();
			string Value = CurrentAtt->Value();

			CurrentData[Name] = Value;
			CurrentAtt = CurrentAtt->Next();
		}

		DataList.push_back(CurrentData);
		XmlList.push_back(CurrentElement);
		CurrentElement = CurrentElement->NextSiblingElement();
	}

	while(!XmlList.empty())
	{
		XMLElement *Element2Delete = XmlList.front();
		m_BackupFile->FirstChildElement()->DeleteChild(Element2Delete);
		XmlList.pop_front();
	}
	SaveBackupFile();

	pthread_mutex_unlock(&FileMutex);

	return true;
}

int
CXMLDb::LoadBackupFile(void)
{
	if(m_BackupFile)
		return true;

	m_BackupFile = new XMLDocument();

	if(m_BackupFile->LoadFile(m_FileName.c_str()) != XML_NO_ERROR)
	{
		if(m_BackupFile->ErrorID() == XML_ERROR_FILE_NOT_FOUND)
			return CreateBackupFile();
		else
		{
			syslog(LOG_ERR, "Error Opening backup.xml: %d\n",
					m_BackupFile->ErrorID());

			delete m_BackupFile;
			m_BackupFile = 0;
			return false;
		}
	}

	return true;
}

int
CXMLDb::SaveBackupFile(void)
{
	if(!m_BackupFile)
		return false;

	if(!m_BackupFile->SaveFile(m_FileName.c_str()))
		return false;

	return true;
}

int
CXMLDb::CreateBackupFile(void)
{
	if(!m_BackupFile)
		return false;

	m_BackupFile->InsertEndChild(m_BackupFile->NewDeclaration(NULL));
	m_BackupFile->InsertEndChild(m_BackupFile->NewElement("Backup"));

	if(!m_BackupFile->SaveFile(m_FileName.c_str()))
		return false;

	return true;
}
