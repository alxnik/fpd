/*
 * Copyright (c) 2012 Alexandros Nikolopoulos <alxnik@gmail.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include "CSmaProbe.h"
#include "errno.h"
#include "in_smadata2plus_structs.h"


CSmaProbe::CSmaProbe(CInterface *Iface, string uuid)
{
	m_Connection = Iface;
	m_packetCount = 0;
}
// FIXME
CSmaProbe::CSmaProbe(CInterface *Iface, list<int> sensors, string uuid)
{
	m_Connection = Iface;
	m_packetCount = 0;
}
CSmaProbe::~CSmaProbe()
{
	Stop();
	delete(m_Connection);
}

int
CSmaProbe::Login(void)
{
	struct smadata2_l1_packet p;
	struct smadata2_l1_packet s;
	memset(&p, 0x0, sizeof(struct smadata2_l1_packet));
	memset(&s, 0x0, sizeof(struct smadata2_l1_packet));

	struct smadata2_l2_packet p2;
	struct smadata2_l2_packet s2;
	memset(&p2, 0x0, sizeof(struct smadata2_l2_packet));
	memset(&s2, 0x0, sizeof(struct smadata2_l2_packet));


	s.cmd_code = CMDCODE_LEVEL2;
	memset(s.dest, 0xff, 6);
	// FIXME
	//memcpy(s.src, m_Connection->GetLocalMac(), 6);

	/* Set Layer 2 */
	s2.ctrl1 = 0x0e;
	s2.ctrl2 = 0xa0;
	s2.zero = 0x01;
	s2.c = 0x01;
	/* Set L2 Content */
	uint8_t LoginPkt[21] = { 0x80, 0x0C, 0x04, 0xFD, 0xFF, 0x07, 0x00, 0x00, 0x00,
			0x84, 0x03, 0x00, 0x00, 0xaa, 0xaa, 0xbb, 0xbb, 0x00, 0x00, 0x00, 0x00 };

	memcpy(s2.content, LoginPkt, sizeof(LoginPkt));
	s2.content_length = sizeof(LoginPkt);

	/* Adding Password */
	int j = 0;
	uint8_t passwd[] = "0000";
	uint8_t passwd_char;
	for (int i = 0; i < 12; i++)
	{
		/* As soon as first null byte write only null bytes */
		if (passwd[j] == 0x00)
			s2.content[s2.content_length] = 0x00 + 0x88;
		else {
			passwd_char = passwd[j];
			s2.content[s2.content_length] = ((passwd_char + 0x88) % 0xff);
			j++;
		}
		s2.content_length++;
	}

	s.length = Level2Write(s.content, &s2) + L1_HEADER_LEN;

	Level1Write(&s);

	/* Wait for cmdcode 1 */
	WaitForCode(&p, CMDCODE_LEVEL2);
}

int
CSmaProbe::Start(void)
{
	struct smadata2_l1_packet p;
	struct smadata2_l1_packet s;
	memset(&p, 0x0, sizeof(struct smadata2_l1_packet));
	memset(&s, 0x0, sizeof(struct smadata2_l1_packet));

	struct smadata2_l2_packet p2;
	struct smadata2_l2_packet s2;
	memset(&p2, 0x0, sizeof(struct smadata2_l2_packet));
	memset(&s2, 0x0, sizeof(struct smadata2_l2_packet));


	WaitForCode(&p, CMDCODE_BROADCAST);


	/* fetch netid from package */
	uint8_t netid = p.content[4];

	/* Answer broadcast */
	s.cmd_code = CMDCODE_BROADCAST;
	/* Set destination */
	memcpy(s.dest, p.src, 6);
	/* Set my address */
	// FIXME
	//memcpy(s.src, m_Connection->GetLocalMac(), 6);

	uint16_t len = 0;
	/* Copy content for Broadcast */
	memcpy(s.content, L1_CONTENT_BROADCAST, sizeof(L1_CONTENT_BROADCAST));
	len += sizeof(L1_CONTENT_BROADCAST);
	/* Set netid */
	s.content[4] = netid;

	/* Setting length of packet */
	s.length = L1_HEADER_LEN + len;

	/* Send Packet out */
	Level1Write(&s);

	WaitForCode(&p, CMDCODE_10);
	WaitForCode(&p, CMDCODE_5);

	// Reset contents and go for phase 2
	memset(&s, 0x0, sizeof(struct smadata2_l1_packet));
	memset(&s2, 0x0, sizeof(struct smadata2_l2_packet));


	// ----Sending L2 Packet 1-----
	// Get Level 1 packet ready
	s.cmd_code = CMDCODE_LEVEL2;
	memset(s.dest, 0xff, 6);
	// FIXME
	//memcpy(s.src, m_Connection->GetLocalMac(), 6);

	// Set Level 2 packet. I have NO idea what these are
	s2.ctrl1 = 0x09;
	s2.ctrl2 = 0xa0;

	uint8_t PktLoad[] = { 0x80, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	memcpy(s2.content, PktLoad, sizeof(PktLoad));
	s2.content_length = sizeof(PktLoad);

	s.length = Level2Write(s.content, &s2) + L1_HEADER_LEN;


	Level1Write(&s);

	// Wait for a level2 reply to our magic packet 1
	WaitForCode(&p, CMDCODE_LEVEL2);

	Level2Read(p.content, p.length-L1_HEADER_LEN, &p2);

	uint32_t Serial = p2.src[3] | p2.src[2] << 8 | p2.src[1] << 16 | p2.src[0] << 24;


	syslog(LOG_INFO, "Inverter found with serial = [%u]\n", Serial);

	// Reset contents and go for phase 3
	memset(&s, 0x0, sizeof(struct smadata2_l1_packet));
	memset(&s2, 0x0, sizeof(struct smadata2_l2_packet));

	s.cmd_code = CMDCODE_LEVEL2;
	memset(s.dest, 0xff, 6);
	// FIXME
	//memcpy(s.src, m_Connection->GetLocalMac(), 6);
	/* Set Layer 2 */
	s2.ctrl1 = 0x08;
	s2.ctrl2 = 0xa0;
	s2.zero = 0x03;
	s2.c = 0x03;
	/* Set L2 Content */
	uint8_t PktLoad2[] = { 0x80, 0x0E, 0x01, 0xFD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

	memcpy(s2.content, PktLoad2, sizeof(PktLoad2));
	s2.content_length = sizeof(PktLoad2);
	/* Generate L2 Paket */
	s.length = Level2Write(s.content, &s2) + L1_HEADER_LEN;

	/* Send Packet out */
	Level1Write(&s);


	if(Login() == false)
		return false;


	return true;
}


int
CSmaProbe::Stop(void)
{
	m_Connection->Disconnect();
	return true;
}

int
CSmaProbe::GetAverage(DataContainer &AverageData)
{
	return true;
}

list<int>
CSmaProbe::GetConnectedInverters(void)
{
	list<int> Inverters;
	return Inverters;
}

int
CSmaProbe::ResetStack(void)
{
	return true;
}

int
CSmaProbe::StripEscapes(uint8_t *buffer, uint32_t len)
{
	int inLen = len;

	for (int i = 1; i < inLen; i++)
	{
		if (buffer[i] == 0x7d)
		{
			/* Found escape character. Need to convert*/
			buffer[i] = buffer[i + 1] ^ 0x20;

			/* Short buffer */
			for (int j = i + 1; j < inLen - 1; j++)
				buffer[j] = buffer[j + 1];
			inLen--;
		}
	}
	return inLen;
}

int
CSmaProbe::AddEscapes(uint8_t *buffer, uint32_t len)
{
	int inLen = len;

	/* Loop through buffer from second byte*/
	for (int i = 1; i < inLen; i++)
	{
		switch (buffer[i])
		{
			/* chars to escape */
			case 0x7d:
			case 0x7e:
			case 0x11:
			case 0x12:
			case 0x13:
				/* move following chars */
				for (int j = inLen; j > i; j--)
					buffer[j] = buffer[j - 1];

				/* add escape */
				buffer[i + 1] = buffer[i] ^ 0x20;
				buffer[i] = 0x7d;
				inLen++;
				break;
		}
	}
	return inLen;
}

/* Calculate a new fcs given the current fcs and the new data. */
u_int16_t
CSmaProbe::CalcCRC(u_int16_t fcs, void *_cp, int len)
{
	register uint8_t *cp = (uint8_t *) _cp;

	while (len--)
		fcs = (fcs >> 8) ^ SMADATA2PLUS_L2_FCSTAB[(fcs ^ *cp++) & 0xff];
	return (fcs);
}

void
CSmaProbe::GenerateCRC(uint8_t * buffer, int len, uint8_t * cs)
{
	u_int16_t trialfcs;
	uint8_t stripped[BUFSIZ] = { 0 };

	memcpy(stripped, buffer, len);

	trialfcs = CalcCRC(INIT_FCS16, stripped, len);
	trialfcs ^= 0xffff; /* complement */

	cs[0] = (trialfcs & 0x00ff); /* least significant byte first */
	cs[1] = ((trialfcs >> 8) & 0x00ff);
}

int
CSmaProbe::Level2Read(uint8_t *buffer, int len, struct smadata2_l2_packet *p)
{

	int pos = 0;

	/* Strip escapes */
	int InitLen = len;
	len = StripEscapes(buffer, len);
	int diff = InitLen - len;

	/* Remove checksum */
	len -= 1;
	uint8_t checksum[2], checksum_recv[2];
	checksum_recv[1] = buffer[(len--) - 1];
	checksum_recv[0] = buffer[(len--) - 1];

	/* Generate checksum */
	GenerateCRC(buffer + 1, len - 1, checksum);

	/* Log */
	syslog(LOG_INFO, "Unescaped %d chars, checksum %02x:%02x==%02x:%02x  ", diff, checksum[0], checksum[1], checksum_recv[0], checksum_recv[1]);

	/* Compare checksums */
	if (memcmp(checksum, checksum_recv, 2) != 0)
	{
		syslog(LOG_INFO, "Received packet with wrong Checksum");
		return false;
	}

	/*** Start Reading of packet ***/

	/* Start byte */
	pos++;

	/* header bytes */
	pos += sizeof(L2_HEADER);

	/* Ctrl codes */
	p->ctrl1 = buffer[pos++];
	p->ctrl2 = buffer[pos++];

	/* Destination */
	baswap((bdaddr_t*) p->dest, (const bdaddr_t*) (buffer + pos));
	pos += 6;

	/* ArchCd and zero */
	p->archcd = buffer[pos++];
	p->zero = buffer[pos++];

	/* Source */
	baswap((bdaddr_t*) p->src, (const bdaddr_t*) (buffer + pos));
	pos += 6;

	/* zero and  c */
	pos++;
	p->c = buffer[pos++];

	/* four zeros */
	pos += 4;

	/* packetcount */
	pos += 2;

	/* content */
	p->content_length = len - pos;
	memcpy(p->content, buffer + pos, p->content_length);

	syslog(LOG_INFO, "[L2] Received packet [contentlength: %d]", p->content_length);
	return true;
}

int
CSmaProbe::Level2Write(uint8_t * buffer, struct smadata2_l2_packet *p)
{

	// Set some defaults
	uint8_t null_addr[] = {0x00,0x00,0x00,0x00,0x00,0x00};
	if (!memcmp(p->dest,null_addr,6))
		memset(p->dest,0xff,6);

	// FIXME
	//if (!memcmp(p->src,null_addr,6))
	//	memcpy(p->src, m_Connection->GetLocalMac(), 6);
	/** Packet Header **/


	/* Length of buffer used */
	int len = 0;

	/* Startbyte */
	buffer[len++] = L2_STARTBYTE;

	/* Headerbytes */
	memcpy(buffer + len, L2_HEADER, sizeof(L2_HEADER));
	len += sizeof(L2_HEADER);

	/* Ctrl codes */
	buffer[len++] = p->ctrl1;
	buffer[len++] = p->ctrl2;

	/* Destination */
	baswap((bdaddr_t*) (buffer + len), (const bdaddr_t*) p->dest);
	len += 6;

	/* ArchCd and zero */
	buffer[len++] = p->archcd;
	buffer[len++] = p->zero;

	/* Source */
	baswap((bdaddr_t*) (buffer + len), (const bdaddr_t*) p->src);
	len += 6;

	/* zero and  c */
	buffer[len++] = 0x00;
	buffer[len++] = p->c;

	/* four zeros */
	memset(buffer + len, 0x0, 4);
	len += 4;

	/* packetcount */
	buffer[len++] = m_packetCount++;

	/* adding content */
	memcpy(buffer + len, p->content, p->content_length);
	len += p->content_length;

	/* build checksum of content */
	uint8_t checksum[2];
	GenerateCRC(buffer + 1, len - 1, checksum);

	/* Escape special chars */
	len = AddEscapes(buffer, len);

	/* Adding checksum */
	int checksum_len = 2;
	memcpy(buffer + len, checksum, 2);
	/* Escaping checksum if needed */
	AddEscapes(buffer + len, checksum_len);
	len += checksum_len;

	/* Trailing Byte */
	buffer[len++] = L2_STARTBYTE;

	return len;

}


void
CSmaProbe::WaitForCode(struct smadata2_l1_packet *p, int cmdcode)
{
	syslog(LOG_INFO, "Waiting for packet code [%d]\n", cmdcode);
	int act_cmdcode;
	do
	{
		act_cmdcode = Level1Read(p);

		if(SigTermFlag == true)
			break;
	} while(act_cmdcode != cmdcode);

	syslog(LOG_INFO, "Got packet code == [%d]\n", cmdcode);
}


int
CSmaProbe::Level1Read(struct smadata2_l1_packet *p)
{

	/* Offset for fragments */
	int offset = 0;

	/* wait for start package */
	int rv = 0;
	uint8_t ch = 0;
	do
	{
		rv = m_Connection->Receive(&ch, 1, 3);
		usleep(500);
	} while(rv != 1 && ch != L1_STARTBYTE);

	/* Fetching Packet length and computing checksum */
	uint8_t len[2];
	if(m_Connection->Receive(len, 2, 3) != 2)
		return -1;

	if(m_Connection->Receive(&p->checksum, 1, 3) != 1)
			return -1;

	if (p->checksum != (L1_STARTBYTE ^ len[0] ^ len[1]))
	{
		syslog(LOG_WARNING, "Received packet with wrong Checksum");
		return -1;
	}

	/* packet_len */
	int ContentLen = (len[0] | (len[1] << 8)) - L1_HEADER_LEN;

	if (p->cmd_code == CMDCODE_FRAGMENT)
	{
		/* Fragment */
		offset = p->length- L1_HEADER_LEN;
		p->length += ContentLen;
	}
	else {
		/* No Fragment */
		offset = 0;
		p->length = L1_HEADER_LEN + ContentLen;
	}

	/* Fetching source + dest addresses */
	bdaddr_t addr;
	m_Connection->Receive((uint8_t*)&addr, 6, 3);
	baswap((bdaddr_t*) p->src, &addr);
	m_Connection->Receive((uint8_t*)&addr, 6, 3);
	baswap((bdaddr_t*) p->dest, &addr);

	/* cmdcode */
	uint8_t cmd[2];
	if(m_Connection->Receive(cmd, 2, 3) != 2)
			return -1;
	p->cmd_code = (uint16_t) cmd[0] | cmd[1] << 8;

	/* getcontent */
	m_Connection->Receive(p->content+offset, ContentLen, 3);

	/* Check if L1 packet is fragmented */
	if (p->cmd_code == CMDCODE_FRAGMENT)
	{
		syslog(LOG_INFO, "Going for another segment\n");
		return Level1Read(p);
	}
	else
	{
		/* Packet complete */

		syslog(LOG_INFO, "Received L1 Packet [code:%x][len:%d]\n", p->cmd_code, p->length);

		/* Check if contains L2 packet */
		if (p->length - L1_HEADER_LEN > 4
				&& p->content[0] == L1_STARTBYTE
				&& memcmp(p->content + 1, L2_HEADER, 4) == 0)
		{
			syslog(LOG_INFO, "seems to contain l2 packet");
		}

		return p->cmd_code;
	}
}

int
CSmaProbe::Level1Write(struct smadata2_l1_packet *p)
{

	uint8_t buffer[BUFSIZ];
	int i = 0;

	/* Generate lengths and checksum */
	uint8_t len2 = p->length >> 8;
	uint8_t len1 = p->length & 0xff;

	p->checksum = L1_STARTBYTE ^ len1 ^ len2;

	/* Command */
	uint8_t cmd2 = (uint8_t) p->cmd_code >> 8;
	uint8_t cmd1 = (uint8_t) p->cmd_code & 0xff;

	/* Build Packet */
	buffer[i++] = L1_STARTBYTE;
	buffer[i++] = len1;
	buffer[i++] = len2;
	buffer[i++] = p->checksum;
	baswap((bdaddr_t*) (buffer + i), (const bdaddr_t*) p->src);
	i += 6;
	baswap((bdaddr_t*) (buffer + i), (const bdaddr_t*) p->dest);
	i += 6;
	buffer[i++] = cmd1;
	buffer[i++] = cmd2;
	memcpy(buffer + i, p->content, p->length - L1_HEADER_LEN);
	i += p->length - L1_HEADER_LEN;

	syslog(LOG_INFO, "Writing L1 Packet\n");
	return m_Connection->Send(buffer, i);
}
