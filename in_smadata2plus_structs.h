/*
 *  OpenSunny -- OpenSource communication with SMA Readers
 *
 *  Copyright (C) 2012 Christian Simon <simon@swine.de>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program (see the file COPYING included with this
 *  distribution); if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef OPENSUNNY_IN_SMADATA2PLUS_STRUCTS_H_
#define OPENSUNNY_IN_SMADATA2PLUS_STRUCTS_H_

#include <stdio.h>

/* level1 packet */
struct smadata2_l1_packet {
	uint16_t length;
	uint8_t checksum;
	uint8_t src[6];
	uint8_t dest[6];
	uint16_t cmd_code;
	uint8_t content[BUFSIZ];
};

/* level2 packet */
struct smadata2_l2_packet {
	unsigned char src[6];
	unsigned char dest[6];
	unsigned char ctrl1;
	unsigned char ctrl2;
	unsigned char archcd;
	unsigned char zero;
	unsigned char c;
	unsigned char content[BUFSIZ];
	int content_length;
};

/* smadata2 value */
struct smadata2_value {
	char name[64];
	char unit[5];
	float factor;
	unsigned long value;
	int timestamp;
	int r_value_pos;
	int r_value_len;
	int r_timestamp_pos;

};

/* smadata2 query */
struct smadata2_query {
	unsigned char q_ctrl1;
	unsigned char q_ctrl2;
	unsigned char q_archcd;
	unsigned char q_zero;
	unsigned char q_c;
	unsigned char q_content[128];
	int q_content_length;
	unsigned char r_ctrl1;
	unsigned char r_ctrl2;
	struct smadata2_value values[12];
	int value_count;
};

/* smadata2 model */
struct smadata2_model {
	char name[64];
	unsigned char code[2];
	int max_power[4];
	int model_count;
};

#define L1_STARTBYTE 	0x7e
#define L1_HEADER_LEN 	18

#define L2_STARTBYTE 	0x7e

#define CMDCODE_LEVEL2 			0x0001
#define CMDCODE_BROADCAST 		0x0002
#define CMDCODE_SIGNAL			0x0003
#define CMDCODE_5 				0x0005
#define CMDCODE_FRAGMENT 		0x0008
#define CMDCODE_10 				0x000a
#define CMDCODE_12 				0x000c



#define SMADATA2PLUS_MAX_VALUES 64

/* L1 Stuff */
uint8_t L1_CONTENT_BROADCAST[13] = { 0x00, 0x04, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00 };

/* L2 Stuff */
uint8_t L2_HEADER[4] = { 0xFF, 0x03, 0x60, 0x65 };
uint8_t L2_CONTENT_2[9] = { 0x80, 0x0E, 0x01, 0xFD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };


#define INIT_FCS16 0xffff		// Initial FCS value

static u_int16_t SMADATA2PLUS_L2_FCSTAB[256] = { 0x0000, 0x1189, 0x2312, 0x329b,
		0x4624, 0x57ad, 0x6536, 0x74bf, 0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c,
		0xdbe5, 0xe97e, 0xf8f7, 0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c,
		0x75b7, 0x643e, 0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff,
		0xe876, 0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
		0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5, 0x3183,
		0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c, 0xbdcb, 0xac42,
		0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974, 0x4204, 0x538d, 0x6116,
		0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb, 0xce4c, 0xdfc5, 0xed5e, 0xfcd7,
		0x8868, 0x99e1, 0xab7a, 0xbaf3, 0x5285, 0x430c, 0x7197, 0x601e, 0x14a1,
		0x0528, 0x37b3, 0x263a, 0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960,
		0xbbfb, 0xaa72, 0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630,
		0x17b9, 0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
		0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738, 0xffcf,
		0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70, 0x8408, 0x9581,
		0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7, 0x0840, 0x19c9, 0x2b52,
		0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff, 0x9489, 0x8500, 0xb79b, 0xa612,
		0xd2ad, 0xc324, 0xf1bf, 0xe036, 0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5,
		0x4f6c, 0x7df7, 0x6c7e, 0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7,
		0xc03c, 0xd1b5, 0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74,
		0x5dfd, 0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
		0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c, 0xc60c,
		0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3, 0x4a44, 0x5bcd,
		0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb, 0xd68d, 0xc704, 0xf59f,
		0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232, 0x5ac5, 0x4b4c, 0x79d7, 0x685e,
		0x1ce1, 0x0d68, 0x3ff3, 0x2e7a, 0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a,
		0xb0a3, 0x8238, 0x93b1, 0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb,
		0x0e70, 0x1ff9, 0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9,
		0x8330, 0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78 };

/* Define Models */
struct smadata2_model SMADATA2MODELS[] = {
		/* Inverter 1700TL/2100TL */
		{
			"%dTL",
			{0x63,0x00},
			{1700,2100},
			2,
		},
		/* Inverter 3000TL */
		{
			"%dTL",
			{0x71,0x00},
			{3000},
			1,
		},
		/* Inverter 3000TLHF */
		{
			"%dTLHF",
			{0x83,0x00},
			{3000},
			1,
		},
		/* Inverter 3000TLHF */
		{
			"%dTL",
			{0xe2,0x00},
			{3600},
			1,
		},
		/* Inverter 4000TL20/5000TL20 */
		{
			"%dTL20",
			{0x4e,0x00},
			{4000,5000},
			2,
		},
		/* Inverter 4000TL21/5000TL21 */
		{
			"%dTL21",
			{0x8a,0x00},
			{4000,5000},
			2,
		},
		/* Inverter 6000TL/7000TL */
		{
			"%dTL",
			{0x63,0x00},
			{6000,7000},
			2,
		},
		/* Inverter 8000TL/10000TL */
		{
			"%dTL",
			{0x80,0x00},
			{8000,10000},
			2,
		},
};


/* Define Value Structs */
struct smadata2_query SMADATA2PLUS_QUERIES[] = {
	/* Power AC */
	{
			0x09,			/* Query ctrl1 */
			0xa1,			/* Query ctrl2 */
			0x00,			/* ArchCD */
			0x00,			/* Zero */
			0x00,			/* C */
							/* Query Content */
			{ 0x80, 0x00, 0x02, 0x00, 0x51, 0x00, 0x3f, 0x26, 0x00, 0xFF, 0x3f, 0x26, 0x00, 0x0e },
			14,				/* Query Content Length */
			0x10,			/* Response ctrl1 */
			0x90,			/* Response ctrl2 */
			{
					{
						"power_ac", 	/* Value name */
						"W",			/* Unit */
						1.0,			/* Factor */
						0L,				/* Actual Value */
						0,				/* Timestamp */
						20,				/* Value Pos */
						3,				/* Value Len */
						16,				/* Timestamp Pos */
					},
			},
			1,				/* Value Count */
	},
	/* Yield in inverterlifetime */
	{
			0x09,			/* Query ctrl1 */
			0xa0,			/* Query ctrl2 */
			0x00,			/* ArchCD */
			0x00,			/* Zero */
			0x00,			/* C */
							/* Query Content */
			{ 0x80, 0x00, 0x02, 0x00, 0x54, 0x00, 0x01, 0x26, 0x00, 0xFF, 0x01, 0x26, 0x00 },
			13,				/* Query Content Length */
			0x0d,			/* Response ctrl1 */
			0x90,			/* Response ctrl2 */
			{
					{
						"yield_total", 	/* Value name */
						"kWh",			/* Unit */
						0.001,			/* Factor */
						0L,				/* Actual Value */
						0,				/* Timestamp */
						20,				/* Value Pos */
						8,				/* Value Len */
						16,				/* Timestamp Pos */
					},
			},
			1,				/* Value Count */

	},
	/* DC stuff finally */
	{
			0x44,			/* Query ctrl1 */
			0xa0,			/* Query ctrl2 */
			0x00,			/* ArchCD */
			0x00,			/* Zero */
			0x00,			/* C */
							/* Query Content */
			{0x80, 0x00, 0x02, 0x80, 0x53, 0x00, 0x00, 0x20, 0x00, 0xff, 0xff, 0x50, 0x00, 0x00},
			14,				/* Query Content Length */
			0x33,			/* Response ctrl1 */
			0x90,			/* Response ctrl2 */
			{
					{
						"power_dc_1", 	/* Value name */
						"W",			/* Unit */
						1.0,			/* Factor */
						0L,				/* Actual Value */
						0,				/* Timestamp */
						20,				/* Value Pos */
						3,				/* Value Len */
						16				/* Timestamp Pos */
					},
					{
						"power_dc_2", 	/* Value name */
						"W",			/* Unit */
						1.0,			/* Factor */
						0L,				/* Actual Value */
						0,				/* Timestamp */
						48,				/* Value Pos */
						3,				/* Value Len */
						44				/* Timestamp Pos */
					},
					{
						"voltage_dc_1", /* Value name */
						"V",			/* Unit */
						0.01,			/* Factor */
						0L,				/* Actual Value */
						0,				/* Timestamp */
						76,				/* Value Pos */
						3,				/* Value Len */
						72				/* Timestamp Pos */
					},
					{
						"voltage_dc_2", 	/* Value name */
						"V",			/* Unit */
						0.01,			/* Factor */
						0L,				/* Actual Value */
						0,				/* Timestamp */
						104,			/* Value Pos */
						3,				/* Value Len */
						100				/* Timestamp Pos */
					},
			},
			4,				/* Value Count */
	},
	/* AC stuff finally */
	{
			0x09,			/* Query ctrl1 */
			0xa1,			/* Query ctrl2 */
			0x00,			/* ArchCD */
			0x00,			/* Zero */
			0x00,			/* C */
							/* Query Content */
			{0x80, 0x00, 0x02, 0x00, 0x51, 0x00, 0x00, 0x20, 0x00, 0xff, 0xff, 0x50, 0x00, 0x0e},
			14,				/* Query Content Length */
			0x79,			/* Response ctrl1 */
			0x90,			/* Response ctrl2 */
			{
					{
						"power_ac_max_l1", 	/* Value name */
						"W",			/* Unit */
						1.0,			/* Factor */
						0L,				/* Actual Value */
						0,				/* Timestamp */
						48,				/* Value Pos */
						3,				/* Value Len */
						44				/* Timestamp Pos */
					},
					{
						"power_ac_max_l2", 	/* Value name */
						"W",			/* Unit */
						1.0,			/* Factor */
						0L,				/* Actual Value */
						0,				/* Timestamp */
						76,				/* Value Pos */
						3,				/* Value Len */
						72				/* Timestamp Pos */
					},
					{
						"power_ac_max_l3", 	/* Value name */
						"W",			/* Unit */
						1.0,			/* Factor */
						0L,				/* Actual Value */
						0,				/* Timestamp */
						104,			/* Value Pos */
						3,				/* Value Len */
						100				/* Timestamp Pos */
					},
					{
						"power_ac_l1", 	/* Value name */
						"W",			/* Unit */
						1.0,			/* Factor */
						0L,				/* Actual Value */
						0,				/* Timestamp */
						216	,			/* Value Pos */
						3,				/* Value Len */
						212				/* Timestamp Pos */
					},
					{
						"power_ac_l2", 	/* Value name */
						"W",			/* Unit */
						1.0,			/* Factor */
						0L,				/* Actual Value */
						0,				/* Timestamp */
						244	,			/* Value Pos */
						3,				/* Value Len */
						240				/* Timestamp Pos */
					},
					{
						"power_ac_l3", 	/* Value name */
						"W",			/* Unit */
						1.0,			/* Factor */
						0L,				/* Actual Value */
						0,				/* Timestamp */
						272	,			/* Value Pos */
						3,				/* Value Len */
						268				/* Timestamp Pos */
					},
					{
						"voltage_ac_l1", 	/* Value name */
						"V",			/* Unit */
						0.01,			/* Factor */
						0L,				/* Actual Value */
						0,				/* Timestamp */
						300	,			/* Value Pos */
						3,				/* Value Len */
						296				/* Timestamp Pos */
					},
					{
						"voltage_ac_l2", 	/* Value name */
						"V",			/* Unit */
						0.01,			/* Factor */
						0L,				/* Actual Value */
						0,				/* Timestamp */
						328,			/* Value Pos */
						3,				/* Value Len */
						324				/* Timestamp Pos */
					},
					{
						"voltage_ac_l3", 	/* Value name */
						"V",			/* Unit */
						0.01,			/* Factor */
						0L,				/* Actual Value */
						0,				/* Timestamp */
						356,			/* Value Pos */
						3,				/* Value Len */
						352				/* Timestamp Pos */
					},
					{
						"current_ac_l1", 	/* Value name */
						"A",			/* Unit */
						0.001,			/* Factor */
						0L,				/* Actual Value */
						0,				/* Timestamp */
						384,			/* Value Pos */
						3,				/* Value Len */
						380				/* Timestamp Pos */
					},
					{
						"current_ac_l2", 	/* Value name */
						"A",			/* Unit */
						0.001,			/* Factor */
						0L,				/* Actual Value */
						0,				/* Timestamp */
						412,			/* Value Pos */
						3,				/* Value Len */
						408				/* Timestamp Pos */
					},
					{
						"current_ac_l3", 	/* Value name */
						"A",			/* Unit */
						0.001,			/* Factor */
						0L,				/* Actual Value */
						0,				/* Timestamp */
						440,			/* Value Pos */
						3,				/* Value Len */
						436				/* Timestamp Pos */
					},
			},
			12,				/* Value Count */
	},

	/* 0E A0 FF FF FF FF FF FF 00 01 78 00 $UNKNOWN 00 01 00 00 00 00 $CNT 80 0C 04 FD FF 07 00 00 00 84 03 00 00 $TIME 00 00 00 00 $PASSWORD $CRC 7E $END;*/
};

#endif /* OPENSUNNY_IN_SMADATA2PLUS_STRUCTS_H_ */
