#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "QnetTypeDefs.h"

void PrintUDPHeader(const unsigned char *buf)
{
	printf("UDP Header: IP %u.%u.%u.%u:%u --> %u.%u.%u.%u:%u\n",
		buf[26], buf[27], buf[28], buf[29], 256*buf[34]+buf[35],
		buf[30], buf[31], buf[32], buf[33], 256*buf[36]+buf[37]);
}

void PrintPacket(const int len, const unsigned char *buf)
{
	int packetlen = buf[39] - 8;

	if (0 == memcmp(buf+42, "DSRP", 4)) {

		// look for viable packets
		if (buf[4]==0xA && packetlen<=64)
			;	// this is a poll packet
		else if (packetlen!=49 && packetlen!=21) {
			printf("\n!!!!!!!!!!!!!!!!!!Unknown DSRP packet size=%d!!!!!!!!!!!!!!!!!!\n", packetlen);
			return;
		}

		// put it in a packet struct
		SDSRP pkt;
		memcpy(pkt.title, buf+42, packetlen);

		// print it
		printf("\n**** DSRP %s **** ", (packetlen==48) ? "Header" : ((packetlen==21) ? "Data" : "Poll") );
		if (pkt.tag == 0xA) {
			pkt.poll_msg[58] = '\0';	// make sure it's NULL terminated!
			printf("Msg=%s\n", pkt.poll_msg);
			return;
		}
		printf("tag=%02X id=%04X seq=%02X ", pkt.tag, pkt.header.id, pkt.header.seq);
		switch (packetlen) {
			case 49:
				printf("flag=%02X %02X %02X R2=%.8s R1=%.8s UR=%.8s MY=%.8s/%.4s checksum=%04X\n",
					pkt.header.flag[0], pkt.header.flag[1], pkt.header.flag[2],
					pkt.header.r2, pkt.header.r1, pkt.header.yr, pkt.header.my, pkt.header.nm, pkt.header.pfcs);
				break;
			case 21:
				printf("err=%02X ambe=", pkt.voice.err);
				for (int i=0; i<9; i++)
					printf("%02X", pkt.voice.ambe[i]);
				if (0x55==pkt.voice.ambe[9] && 0x2d==pkt.voice.ambe[10] && 0x16==pkt.voice.ambe[11])
					printf(" Slow data=<SYNC> {0x55, 0x2d, 0x16}\n");
				else
					printf(" Slow Data='%c%c%c'\n", pkt.voice.ambe[9] ^ 0x70, pkt.voice.ambe[10] ^ 0x4f, pkt.voice.ambe[11] ^ 0x93);
				break;
		}

	} else if (0 == memcmp(buf+42, "DSTR", 4)) {

		// Only accept expected lengths
		switch (packetlen) {
			case 58:
			case 29:
			case 26:
			case 32:
				break;
			default:
				printf("\n!!!!!!!!!!!!!!!!!!Unknown DSVT packet size=%d!!!!!!!!!!!!!!!!!!\n", packetlen);
				return;
		}

		// put it in a packet struct
		SDSTR pkt;
		memcpy(pkt.pkt_id, buf, packetlen);

		// print it
		printf("\n**** DSTR %s **** ", (packetlen==58) ? "Header" : ((packetlen==29) ? "Data" : ((packetlen==26) ? "Poll" : "Data+")));
		PrintUDPHeader(buf);
		printf("counter=%04X, flag=%02X %02X %02X remaining=%02X ", pkt.counter, pkt.flag[0], pkt.flag[1], pkt.flag[2], pkt.remaining);
		if (packetlen == 26) {
			printf("my=%.8s rpt=%.8s\n", pkt.spkt.mycall, pkt.spkt.rpt);
			return;
		}
		printf("irc_id=%02X dst_rptr_id=%02X snd_rptr_id=%02X snd_term_id=%02X streamid=%04X, ctrl=%02X\n",
			pkt.vpkt.icm_id, pkt.vpkt.dst_rptr_id, pkt.vpkt.snd_rptr_id, pkt.vpkt.snd_term_id, pkt.vpkt.streamid, pkt.vpkt.ctrl);
		switch (packetlen) {
			case 29:
				printf("ambe=");
				for (int i=0; i<9; i++)
					printf("%02X", pkt.vpkt.vasd.voice[i]);
				if (0x55==pkt.vpkt.vasd.voice[9] && 0x2d==pkt.vpkt.vasd.voice[10] && 0x16==pkt.vpkt.vasd.voice[11])
					printf(" Slow data=<SYNC> {0x55, 0x2d, 0x16}\n");
				else
					printf(" Slow Data='%c%c%c'\n", pkt.vpkt.vasd.voice[9] ^ 0x70, pkt.vpkt.vasd.voice[10] ^ 0x4f, pkt.vpkt.vasd.voice[11] ^ 0x93);
				break;
			case 58:
				printf("flag=%02X %02X %02X R2=%.8s R1=%.8s UR=%.8s MY=%.8s/%.4s checksum=%02X%02X\n",
					pkt.vpkt.hdr.flag[0], pkt.vpkt.hdr.flag[1], pkt.vpkt.hdr.flag[2],
					pkt.vpkt.hdr.r1, pkt.vpkt.hdr.r2, pkt.vpkt.hdr.ur, pkt.vpkt.hdr.my, pkt.vpkt.hdr.nm,
					pkt.vpkt.hdr.pfcs[0], pkt.vpkt.hdr.pfcs[1]);
				break;
			case 32:
				printf("extra=%02X %02X %02X ambe=", pkt.vpkt.vasd1.UNKNOWN[0], pkt.vpkt.vasd1.UNKNOWN[1], pkt.vpkt.vasd1.UNKNOWN[2]);
				for (int i=0; i<9; i++)
					printf("%02X", pkt.vpkt.vasd1.voice[i]);
				if (0x55==pkt.vpkt.vasd1.voice[9] && 0x2d==pkt.vpkt.vasd1.voice[10] && 0x16==pkt.vpkt.vasd1.voice[11])
					printf(" Slow data=<SYNC> {0x55, 0x2d, 0x16}\n");
				else
					printf(" Slow Data='%c%c%c'\n", pkt.vpkt.vasd1.voice[9] ^ 0x70, pkt.vpkt.vasd1.voice[10] ^ 0x4f, pkt.vpkt.vasd1.voice[11] ^ 0x93);
				break;
		}

	} else if (0 == memcmp(buf+42, "DSVT", 4)) {

		// Only accept expected lengths!
		if (packetlen!= 56 && packetlen!=27) {
			printf("\n!!!!!!!!!!!!!!!!!!Unknown DSVT packet size=%d!!!!!!!!!!!!!!!!!!\n", packetlen);
			return;
		}

		// put it in a packet struct
		SDSVT pkt;
		memcpy(pkt.title, buf+42, packetlen);

		// print it
		printf("\n**** DSVT %s **** ", (packetlen==56) ? "Header" : "Data" );
		PrintUDPHeader(buf);
		printf("config=%02X id=%02X streamid=%04X counter=%02X ", pkt.config, pkt.id, pkt.streamid, pkt.counter);
		printf("flaga=%02X %02X %02X flagb=%02X %02X %02X ", pkt.flaga[0], pkt.flaga[1], pkt.flaga[2], pkt.flagb[0], pkt.flagb[1], pkt.flagb[2]);
		switch (packetlen) {
			case 56:
				printf("flag=%02X %02X %02X R2=%.8s R1=%.8s UR=%.8s MY=%.8s/%.4s checksum=%02X%02X\n",
					pkt.hdr.flag[0], pkt.hdr.flag[1], pkt.hdr.flag[2],
					pkt.hdr.rpt2, pkt.hdr.rpt1, pkt.hdr.urcall, pkt.hdr.mycall, pkt.hdr.sfx, pkt.hdr.pfcs[0], pkt.hdr.pfcs[1]);
				break;
			case 27:
				printf("ambe=");
				for (int i=0; i<9; i++)
					printf("%02X", pkt.vasd.voice[i]);
				if (0x55==pkt.vasd.text[0] && 0x2d==pkt.vasd.text[1] && 0x16==pkt.vasd.text[2])
					printf(" Slow data=<SYNC> {0x55, 0x2d, 0x16}\n");
				else
					printf(" Slow Data='%c%c%c'\n", pkt.vasd.text[0] ^ 0x70, pkt.vasd.text[1] ^ 0x4f, pkt.vasd.text[2] ^ 0x93);
				break;
		}

	} else {
		printf("\n!!!!!!!!!!!!!!!!!!unknown packet type with length %d!!!!!!!!!!!!!!!!!!\n", len);
	}
}

int main(int argc, char *argv[])
{
	int o = 0;
	int count = 0;
	do {
		printf("%s, Version 0.1, Copyright (C) 2018 by Thomas A. Early N7TAE\n", argv[0]);
		printf("%s comes with ABSOLUTELY NO WARRANTY; This is free software, and\n", argv[0]);
		printf("you are welcome to redistribute it under certain conditions.\nPlease see the LICENSE file.\n");
		char line[200];
		char *s = fgets(line, 200, stdin);
		if (s == NULL)
			break;
		if (strlen(s) < 5)
			continue;
		line[53] = '\0';	// clip the text portion of the output
		unsigned char u[200];
		int count = sscanf(line, "%*4x %2hhx %2hhx %2hhx %2hhx %2hhx %2hhx %2hhx %2hhx %2hhx %2hhx %2hhx %2hhx %2hhx %2hhx %2hhx %2hhx",
			u+o, u+o+1, u+o+2, u+o+3, u+o+4, u+o+5, u+o+6, u+o+7, u+o+8, u+o+9, u+o+10, u+o+11, u+o+12, u+o+13, u+o+14, u+o+15);
		o += count;
		if (count != 16 && o) {
			PrintPacket(o, u);
			o = 0;
		}
	} while (count != EOF);
	return 0;
}