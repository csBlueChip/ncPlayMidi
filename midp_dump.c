#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>

#include "midp_play.h"

//+============================================================================ ========================================
// Dump an entire MIDI file : smf[n]
//
bool  fileDump (uint8_t n)
{
	smf_t*  pSmf = &_p.smf[n];
	
	for (int t = 0;  t < pSmf->mthd->trkCnt;  t++)  fileDumpTrack(pSmf, t);
	for (int c = 0;  c < pSmf->cCnt        ;  c++)  fileDumpChunk(pSmf, c);

	return SETERR(OK), true;
}

//+============================================================================ ========================================
// If your file goes over 16MB, you will need to increase the size of the buffer (and change the format string)
//
char*  fileFoffs (smf_t* pSmf,  void* vp)
{
	static       char   offs  [  2+1 +4 +1];  // non-reentrant (not thread safe)
	static const char*  fmt = "%02X:%04X" ;

	uint32_t  off = ((uint8_t*)vp) - pSmf->fBuf;

	snprintf(offs, sizeof(offs), fmt, (off >> 16), (off & 0xFFFF));
	offs[sizeof(offs) - 1] = '\0';

	return offs;
}

//+============================================================================ ========================================
// Dump a single track : smf[n].mtrk[t]
//
void  fileDumpTrack (smf_t* pSmf,  uint8_t t)
{
	mevent_t   ev;
	delta_t    delta = 0;
	mtrack_t*  trk   = &(pSmf->mtrk[t]);
	uint8_t*   bp    = trk->data;

	LOGF("# Dump Track #%d"EOL, t);

	LOGF( "#   fileOff => trkOff : [MsgNr] eDelta ->  tDelta -"EOL);
	LOGF( "#   ------- => ------ : ------- ------ -> ------- -"EOL);

	LOGF( ">   %s => %6d |                             %.4s[8 + %d = %d]"EOL,
	      fileFoffs(pSmf, trk->hdr), 0, trk->hdr->magic, trk->hdr->len, trk->hdr->len + IFFH_SZ );

	for (uint32_t i = 0;  bp < trk->eot;  i++) {
		uint32_t  dw = getEvent(pSmf, trk, &ev, bp);
		delta += ev.delta;
		LOGF( ">   %s => %6d | [%5d] %6d -> %7d - ", 
		      fileFoffs(pSmf, bp), (bp - (uint8_t*)trk->hdr), i, ev.delta, delta );
		showEvent(pSmf, &ev, t);
		LOGF(EOL);
		bp += dw;
	}

	if ((bp[-3] != 0xFF) || (bp[-2] != 0x2F) || (bp[-1] != 0x00))  // https://youtu.be/i2fhNVQPb5I?t=43
		LOGF("! Missing EOT Meta-Event"EOL);

	LOGF( "#   %s => %6d | EOT (last byte of track)"EOL,
	      fileFoffs(pSmf, bp - 1),  (bp - (uint8_t*)trk->hdr) - 1 );

}

//+============================================================================ ========================================
// Dump an unknown chunk : smf[n].mUNK[u]
//
void  fileDumpChunk (smf_t* pSmf,  uint8_t c)
{
	iffh_t*  cp = pSmf->chunk[c];

	LOGF( "# Dump Chunk #%d \"%.4s\"[%d] {%02X, %02X, %02X, %02x}"EOL, c, cp->magic, cp->len,
		  cp->magic[0], cp->magic[1], cp->magic[2], cp->magic[3] );
	hexDump(&cp->data, cp->len, (uint8_t*)cp - pSmf->fBuf);  // File offsets
//	hexDump(&cp->data, cp->len, 0);                          // Chunk offsets
}

//+============================================================================ ========================================
// Hexdump function
//
void  hexDump (const void* const buf,  const uint32_t len,  const uint32_t off)
{
	const uint8_t*  bp = buf;
	char            s[16];
	uint16_t        i;
	uint16_t        left;
	uint16_t        pad;

	for (i = 0;  i < len;  i++, bp++) {
		if (!(i & 0xF)) {
			if (i) printf("  %.4s %.4s  %.4s %.4s"EOL, s, s+4, s+8, s+12);
			memset(s, ' ', sizeof(s));
			printf("+   %02X:%04X:  ", (i + off) >> 16, (i + off) & 0xFFFF) ;
		}
		else if (!(i & 0x7))  printf("  ") ;
		else if (!(i & 0x3))  printf(" ") ;
		printf("%02x ", *bp);
		s[i & 0xF] = ((*bp >= 32) && (*bp <= 126)) ? *bp : '.';
	}
	left = (16 - (i & 0xF)) & 0xF;
	pad  = (left * 3) + (left >> 2) + (left >> 3);
	printf("%*s  %.4s %.4s  %.4s %.4s"EOL, pad,"", s, s+4, s+8, s+12);
}
