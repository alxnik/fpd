/*
 * Copyright (c) 2012 Alexandros Nikolopoulos <alxnik@gmail.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#ifndef CSMAPROBE_H_
#define CSMAPROBE_H_

#include "fpd.h"
#include "CBlueTooth.h"
#include "CLog.h"

class CSmaProbe : public CProbe
{
public:
	CSmaProbe(CBlueTooth *Connection, string uuid);
	virtual ~CSmaProbe();

	int Start(void);
	int Stop(void);
	int GetAverage(DataContainer &AverageData);
	list<int> GetConnectedInverters(void);
	int ResetStack(void);

private:
	int 		Login(void);
	int 		StripEscapes(uint8_t *buffer, uint32_t len);
	int 		AddEscapes(uint8_t *buffer, uint32_t len);

	u_int16_t 	CalcCRC(u_int16_t fcs, void *_cp, int len);
	void		GenerateCRC(uint8_t * buffer, int len, uint8_t * cs);

	void		WaitForCode(struct smadata2_l1_packet *p, int cmdcode);

	int			Level1Read(struct smadata2_l1_packet *p);
	int 		Level1Write(struct smadata2_l1_packet *p);

	int			Level2Read(uint8_t *buffer, int len, struct smadata2_l2_packet *p);
	int			Level2Write(uint8_t * buffer, struct smadata2_l2_packet *p);


	CBlueTooth *m_Connection;
	uint8_t		m_packetCount;

};

#endif /* CSMAPROBE_H_ */
