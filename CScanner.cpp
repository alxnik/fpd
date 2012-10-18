/*
 * Copyright (c) 2012 Alexandros Nikolopoulos <alxnik@gmail.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.

 */

#include "CScanner.h"

extern bool SigTermFlag;

CScanner::CScanner() {
}

CScanner::~CScanner() {
}

CProbe*
CScanner::FindNetwork(void)
{
	CProbe *ActiveProbe = 0;

	list<int> BlueDevices = EnumBlueTooth();
	syslog(LOG_INFO, "Found %lu Bluetooth Adapters", BlueDevices.size());

//	ActiveProbe = FindSMABlueTooth(BlueDevices);

	if(ActiveProbe)
		return ActiveProbe;

	list<string> SerialPorts = EnumSerialPorts();
	syslog(LOG_INFO, "Found %lu Serial Adapters", SerialPorts.size());

	if(SerialPorts.size() == 0)
		return 0;

	syslog(LOG_INFO, "Scanning for Fronius\n");
	ActiveProbe = FindFronius(SerialPorts);

	if(ActiveProbe)
		return ActiveProbe;

	if(SigTermFlag == true)
		return 0;

	syslog(LOG_INFO, "Scanning for Sunergy\n");
	ActiveProbe = FindSunergy(SerialPorts);

	return ActiveProbe;
}

CProbe*
CScanner::FindSunergy(list<string> &SerialPorts)
{
	for (list<string>::iterator Send = SerialPorts.begin(); Send != SerialPorts.end(); Send++)
	{
		for(speed_t spd = B19200; spd<=B19200; spd++)
		{
			if(SigTermFlag == true)
				return 0;

			CModbusMTU *SerialLine = new CModbusMTU(*Send, spd);

			switch(spd)
			{
				case B2400:
					syslog(LOG_INFO, "Trying on [%s] @ 2400 bps\n", Send->c_str());
					break;
				case B4800:
					syslog(LOG_INFO, "Trying on [%s] @ 4800 bps\n", Send->c_str());
					break;
				case B9600:
					syslog(LOG_INFO, "Trying on [%s] @ 9600 bps\n", Send->c_str());
					break;
				case B19200:
					syslog(LOG_INFO, "Trying on [%s] @ 19200 bps\n", Send->c_str());
					break;
				default:
					syslog(LOG_INFO, "Trying on [%s] @ ???? bps\n", Send->c_str());
					break;
			}
			if(!SerialLine->Connect())
			{
				syslog(LOG_INFO, "cannot open serial\n");
				delete(SerialLine);
				continue;
			}
			CSunergyProbe *CurrProbe = new CSunergyProbe(SerialLine, UUID);
			if(!CurrProbe->Start())
			{
				delete(CurrProbe);
				usleep(10000);
				continue;
			}
			else
			{
				syslog(LOG_INFO, "Found Sunergy Inverters on [%s]", Send->c_str());
				return dynamic_cast<CProbe *>(CurrProbe);
				//return 0;
			}
		}
	}
	return 0;
}

CProbe*
CScanner::FindFronius(list<string> &SerialPorts)
{
	for (list<string>::iterator Send = SerialPorts.begin(); Send != SerialPorts.end(); Send++)
	{
		for (list<string>::iterator Receive = SerialPorts.begin(); Receive != SerialPorts.end(); Receive++)
		{
			if(SigTermFlag == true)
				return 0;

			for(speed_t spd = B2400; spd<=B19200; spd++)
			{
				CInterface *SerialLine = new CInterface(*Send, spd, *Receive, spd);

				if(debugFlag == true)
				{
					switch(spd)
					{
						case B2400:
							syslog(LOG_INFO, "Trying on [%s]-[%s] @ 2400 bps\n", Send->c_str(), Receive->c_str());
							break;
						case B4800:
							syslog(LOG_INFO, "Trying on [%s]-[%s] @ 4800 bps\n", Send->c_str(), Receive->c_str());
							break;
						case B9600:
							syslog(LOG_INFO, "Trying on [%s]-[%s] @ 9600 bps\n", Send->c_str(), Receive->c_str());
							break;
						case B19200:
							syslog(LOG_INFO, "Trying on [%s]-[%s] @ 19200 bps\n", Send->c_str(), Receive->c_str());
							break;
						default:
							syslog(LOG_INFO, "Trying on [%s]-[%s] @ ???? bps\n", Send->c_str(), Receive->c_str());
							break;
					}
				}

				if(!SerialLine->Connect())
				{
					syslog(LOG_INFO, "cannot open serial\n");
					delete(SerialLine);
					continue;
				}
				CFroniusProbe *CurrProbe = new CFroniusProbe(SerialLine, UUID);
				if(!CurrProbe->Start())
				{
					delete(CurrProbe);
					usleep(10000);
					continue;
				}
				else
				{
					syslog(LOG_INFO, "Found Fronius Inverters on [%s]-[%s]", Send->c_str(), Receive->c_str());
					return dynamic_cast<CProbe *>(CurrProbe);
				}
			}
		}
	}
	return 0;
}

CProbe*
CScanner::FindSMABlueTooth(list<int> &BlueDevices)
{
	for (list<int>::iterator BTDev = BlueDevices.begin(); BTDev != BlueDevices.end(); BTDev++)
	{
		if(SigTermFlag == true)
			return 0;

		syslog(LOG_INFO, "Scanning for BlueTooth SMA inverters on device hci%d\n", *BTDev);
	    inquiry_info *ii = NULL;
	    int max_rsp, num_rsp;
	    int dev_id = 0, sock, len, flags;
	    int i;
	    char addr[19] = { 0 };
	    char name[248] = { 0 };

	    sock = hci_open_dev( *BTDev );
	    if (sock < 0)
	    {
	        perror("opening socket");
	        exit(1);
	    }
	    len  = 8;
	    max_rsp = 255;
	    flags = IREQ_CACHE_FLUSH;
	    ii = (inquiry_info*)malloc(max_rsp * sizeof(inquiry_info));

	    num_rsp = hci_inquiry(dev_id, len, max_rsp, NULL, &ii, flags);
	    if( num_rsp < 0 )
	    	perror("hci_inquiry");

	    for (i = 0; i < num_rsp; i++)
	    {
	        ba2str(&(ii+i)->bdaddr, addr);
	        memset(name, 0, sizeof(name));
	        if (hci_read_remote_name(sock, &(ii+i)->bdaddr, sizeof(name), name, 0) < 0)
	        	strcpy(name, "[unknown]");

	        if(strstr(name, "SMA"))
	        {
	        	printf("Found SMA at: [%s]  (%s)\n", addr, name);
	        }
	    }

	    free( ii );
	    close( sock );
	}

	return 0;
}

list<string>
CScanner::EnumSerialPorts(void)
{
	struct udev *udev;
	struct udev_enumerate *enumerate;
	struct udev_list_entry *devices, *dev_list_entry;
	struct udev_device *dev;

	list<string> PortList;

	udev = udev_new();
	if (!udev)
	{
		syslog(LOG_ERR, "Can't create udev object\n");
		return PortList;
	}


	enumerate = udev_enumerate_new(udev);
	udev_enumerate_add_match_subsystem(enumerate, "tty");
	udev_enumerate_scan_devices(enumerate);
	devices = udev_enumerate_get_list_entry(enumerate);

	udev_list_entry_foreach(dev_list_entry, devices)
	{
		const char *path;

		path = udev_list_entry_get_name(dev_list_entry);
		dev = udev_device_new_from_syspath(udev, path);

		if(!dev || !udev_device_get_devnode(dev))
			continue;

		string PortPath;
		PortPath = udev_device_get_devnode(dev);

		dev = udev_device_get_parent(dev);
		if (!dev)
			continue;

		if(udev_device_get_subsystem(dev) && !strcmp(udev_device_get_subsystem(dev), "usb-serial"))
		{
			syslog(LOG_INFO, "Found USB-Serial on %s\n", PortPath.c_str());
			PortList.push_back(PortPath);
		}

		else if(udev_device_get_driver(dev))
		{
			const char *driver = udev_device_get_driver(dev);
			if(!strcmp(driver, "uart-pl011") || !strcmp(driver, "uart-pl010"))
			{
				syslog(LOG_INFO, "Found ARM AMBA-type serial port on %s\n", PortPath.c_str());
				PortList.push_back(PortPath);
			}
			else if(!strcmp(driver, "atmel_usart"))
			{
				syslog(LOG_INFO, "Found Atmel AT91 / AT32 Serial on %s\n", PortPath.c_str());
				PortList.push_back(PortPath);
			}
		}


		udev_device_unref(dev);
	}
	/* Free the enumerator object */
	udev_enumerate_unref(enumerate);

	udev_unref(udev);
	return PortList;
}

static list<int> BlueToothDevices;
static int dev_info(int s, int dev_id, long arg)
{
	BlueToothDevices.push_back(dev_id);
	return 0;
}

list<int>
CScanner::EnumBlueTooth(void)
{
	BlueToothDevices.clear();
	hci_for_each_dev(HCI_UP, dev_info, 0);
	return BlueToothDevices;
}

