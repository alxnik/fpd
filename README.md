fpd is a solar (photovoltaic) systems monitoring system, designed to work on linux.

It mainly supports fronius inverters and output to mysql, however its modular design allows the integration of multiple input methods, inverter protocols as well as multiple output modules (*sql, xml, json etc)


With minor adjustments it can be used as a general purpose data acquisition tool

In order to compile go to Debug/ and "make clean && make all"

Required libraries
	* libudev
	* libbluetooth
	* libmysqlclient
