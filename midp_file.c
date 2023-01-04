#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <arpa/inet.h>

#include "midp_play.h"

//+============================================================================ ========================================
// "Unload" the MIDI file from SMF slot n
//
bool  fileUnload (int n)
{
	smf_t*  pSmf = NULL;

	if (n >= _p.smfMax) {
		LOGF("! Unload - Invalid slot number: %d"EOL, n);
		return SETERR(BAD_PRM), false ;
	}

	pSmf = &_p.smf[n];

	if (!pSmf->fBuf) {
		LOGF("! Unload - Slot[%d] already empty"EOL, n);
		return SETERR(EMPTY), false;
	}

	LOGF("# Unload Slot[%d] - \"%s\""EOL, n, pSmf->fn);
	FREE(pSmf->mtrk);
	FREE(pSmf->chunk);
	FREE(pSmf->fn);
	FREE(pSmf->fBuf);
//	memset(pSmf, 0, sizeof(*pSmf));

	return true;
}

//+============================================================================ ========================================
// Load a MIDI file in to memory, analyse it, and index it
//
// http://www.music.mcgill.ca/~ich/classes/mumt306/StandardMIDIfileformat.html
//
// The first word, <format>, specifies the overall organisation of the file. Only three values of <format> are specified:
// 0-the file contains a single multi-channel track
// 1-the file contains one or more simultaneous tracks (or MIDI outputs) of a sequence
// 2-the file contains one or more sequentially independent single-track patterns
//
bool  fileLoad (int n,  char* fn)
{
	FILE*     fh   = NULL;
	uint8_t*  bp   = NULL;
	smf_t*    pSmf = NULL;  // This SMF slot

	// Sanity checks
	if (!_p.smf) {
		LOGF("! Player not initialised"EOL);
		return SETERR(STOPPED), false ;
	}

	if ((n >= _p.smfMax) || (!fn)) {
		LOGF("! Load - Invalid slot number: %d"EOL, n);
		return SETERR(BAD_PRM), false ;
	}

	// Start
	pSmf = &_p.smf[n];
	memset(pSmf, 0, sizeof(*pSmf));
	LOGF("# Load Slot[%d] - \"%s\""EOL, n, fn);

	// Load file to RAM
	// ...a 200K MIDI file is *massive*
	// ...This might change if you start using them to send sample sets!
	if ( !(fh = fopen(fn, "rb")) ) {
		LOGF("! Cannot open file \"%s\""EOL, fn);
		return SETERR(NO_FILE), false ;
	}

	fseek(fh, 0, SEEK_END);
	pSmf->fSiz = ftell(fh);

	if ( !(pSmf->fBuf = calloc(pSmf->fSiz, 1)) ) {
		fclose(fh);
		LOGF("# Out of memory. Requested %d"EOL, pSmf->fSiz);
		return SETERR(MALLOC), false ;
	}

	fseek(fh, 0, SEEK_SET);
	fread(pSmf->fBuf, 1, pSmf->fSiz, fh);

	fclose(fh);

	// BoF: address of 1st byte of MThd header - we may skip a (20 byte) RIFF header
	// EoF: address of 1st byte after the file (in memory)
	pSmf->fBof = pSmf->fBuf;
	pSmf->fEof = pSmf->fBuf + pSmf->fSiz;

	// Grab a copy of the filename
	if (!(pSmf->fn = strdup(fn))) {
		FREE(pSmf->fBuf);
		LOGF("# Out of memory. Requested %d"EOL, strlen(fn) + 1);
		return SETERR(MALLOC), false ;
	}

	// Identify the file type...
	if (!fileCheckSmf(pSmf) && !fileCheckSyx(pSmf)) {
		FREE(pSmf->fBuf);
		LOGF("# Unrecognised file contents"EOL);
		return SETERR(UNK_FILE), false ;
	}

	// Parse (iff) mthd file
	if (pSmf->mthd && !fileParseSmf(pSmf)) {
		fileUnload(n);
		LOGF("# SMF file is corrupt"EOL);
		return SETERR(BAD_FILE), false ;
	}

	return SETERR(OK), true ;
}

//+============================================================================ ========================================
bool  fileCheckRiff (uint8_t* bp)
{
	return ( (strncmp(bp    , RIFF_MAGIC, sizeof(((iffh_t*)NULL)->magic)) == 0) && 
	         (strncmp(bp + 8, "RMIDdata",               8               ) == 0)   ) ;
}

//+============================================================================ ========================================
// There is a LOT more checking we could do here
// If the file Magic fails, an error will NOT be sent to LOGF
bool  fileCheckSmf (smf_t* pSmf)
{
	uint32_t  dw;

	// Too small even for a magic number
	if (pSmf->fSiz < sizeof(((iffh_t*)NULL)->magic))
		return SETERR(BAD_HDR), false ;

	// "RIFF"?
	if (fileCheckRiff(pSmf->fBuf) &&  ((pSmf->fBof = pSmf->fBuf + 20) >= pSmf->fEof)) {
		LOGF("! RIFF header is truncated"EOL);
		return SETERR(FILE_TRUNC), false ;
	}

	// "MThd"?
	if (strncmp(pSmf->fBof, MTHD_MAGIC, sizeof(((iffh_t*)NULL)->magic)) != 0)
		return SETERR(BAD_HDR), false ;

	if (pSmf->fEof - pSmf->fBof < MTHD_LEN) {
		LOGF("! MThd header is truncated"EOL);
		return SETERR(FILE_TRUNC), false ;
	}

	if ( (dw = ntohl(((mthd_t*)pSmf->fBof)->len)) != MTHD_LEN) {
		LOGF("! Header length invalid: %d"EOL, dw);
		return SETERR(BAD_HDR), false ;
	}

	if ( (dw = ntohs(((mthd_t*)pSmf->fBof)->fmt)) > 2) {
		LOGF("! Unknown MIDI format: %d"EOL, dw);
		return SETERR(BAD_FMT), false ;
	}

	// Map mthd to BoF
	pSmf->mthd = (mthd_t*)pSmf->fBof;

	// Fix endian-ness of header fields
	pSmf->mthd->len    = ntohl(pSmf->mthd->len   );
	pSmf->mthd->fmt    = ntohs(pSmf->mthd->fmt   );
	pSmf->mthd->trkCnt = ntohs(pSmf->mthd->trkCnt);
	pSmf->mthd->div    = ntohs(pSmf->mthd->div   );

	// Fix SMPTE fps (2's complement)
	if (pSmf->mthd->divtype == DIV_SMPTE)
		pSmf->mthd->fps = (~pSmf->mthd->fps) + 1;

	return SETERR(OK), true ;
}

//+============================================================================ ========================================
// A SysEx stream is in the form {0xF0 .. 0xF7}
// The payload may be ANY number (>=0) of 7bit values
// Many SysEx streams may run back-to-back
bool  fileCheckSyx (smf_t* pSmf)
{
	enum { SYX_SEEK, SYX_RUN }  state = SYX_SEEK ;

	uint8_t*  bp = NULL;

	if (pSmf->fSiz < 2)
		return SETERR(BAD_SYX), false ;

	for (bp = pSmf->fBof;  bp < pSmf->fEof;  bp++)
		if (state == SYX_SEEK) {
			if (*bp == MSTAT_SYX)  state = SYX_RUN ;
			else                   break ;
		} else {
			if (*bp == MSTAT_ESC)  pSmf->syxCnt++,  state = SYX_SEEK ;
			else if (*bp & 0x80)   break ;
		}

	if ((bp < pSmf->fEof) || (state == SYX_RUN)) {
		LOGF("! Found %d SysEx's, but file is corrupt"EOL, pSmf->syxCnt);
		pSmf->syxCnt = 0;
		return SETERR(BAD_SYX), false ;
	}
	
	return SETERR(OK), true ;
}

//+============================================================================ ========================================
bool  fileParseSmf (smf_t* pSmf)
{
	uint8_t*  bp     = NULL;  // a byte pointer
	uint32_t  dw     = 0;     // a dword

	iffh_t*   iffh   = NULL;  // (pointer to) IFF chunk header

	int       tCnt   = 0;     // track counter
	int       cCnt   = 0;     // unknown chunk counter

#	if LOGGING > 0
		uint32_t  lMax = 0;   // size of longest chunk
		int       cW   = 0;   // width (in base 10) of highest chunk MAX(tCnt, uCnt)
		int       lW   = 0;   // width (in base 10) of longest chunk
#	endif


	LOGF("#   Found header: \"%.4s\"[%d]"  EOL, pSmf->mthd->magic, pSmf->mthd->len);
	LOGF("#     wFormat    = %d"           EOL, pSmf->mthd->fmt                   );
	LOGF("#     wTrackCnt  = %d"           EOL, pSmf->mthd->trkCnt                );
	LOGF("#     wDiv       = 0x%04X"       EOL, pSmf->mthd->div                   );
	LOGF("#       type     +--[15:15] = %x"EOL, pSmf->mthd->divtype               );
	if (pSmf->mthd->divtype == DIV_SMPTE) {
		LOGF("#       -SMPTE   +--[14: 8] = %d"EOL, pSmf->mthd->fps );
		LOGF("#       TicksPF  +--[ 7: 0] = %d"EOL, pSmf->mthd->tpf );
	} else {
		LOGF("#       TicksPQN +--[14: 0] = %d"EOL, pSmf->mthd->tpqn);
	}

	// Sanity check the file
	LOGF("#   Pass #1 - Analyse"EOL);
	pSmf->cCnt = 0;
	for (bp = pSmf->fBof + IFFH_SZ + pSmf->mthd->len;  bp < pSmf->fEof;  bp += IFFH_SZ + iffh->len) {
		iffh   = (iffh_t*)bp;

		iffh->len = ntohl(iffh->len);  // endian fix the length

		// Keep some stats
#		if LOGGING > 0
			if (iffh->len > lMax)  lMax = iffh->len ;  // note longest track
#		endif		
		if (memcmp(iffh->magic, MTRK_MAGIC, sizeof(iffh->magic)) == 0)  tCnt++ ;
		else                                                            pSmf->cCnt++ ;
	}
	if (bp != pSmf->fEof) {
		LOGF("!   Truncated file. Missing %d bytes of last chunk"EOL, bp - pSmf->fEof);
		return SETERR(FILE_TRUNC), false;
	}

#	if LOGGING > 0
		// Find width specifiers
		for (cW = 0, dw = ((tCnt > cCnt) ? tCnt : cCnt);  dw;  cW++, dw /= 10) ;
		for (lW = 0, dw = lMax                         ;  dw;  lW++, dw /= 10) ;
#	endif		

	// Check track count
	if (tCnt != pSmf->mthd->trkCnt) {
		LOGF("!   TrackCount mismatch. Declared %d, found %d"EOL, pSmf->mthd->trkCnt, tCnt);
		return SETERR(TRACKS), false;
	}
	LOGF("#     Identified %d track%s, and %d unknown chunk%s"EOL,
	     tCnt, ((tCnt==1)?"":"s"), pSmf->cCnt, ((pSmf->cCnt==1)?"":"s"));

	// Allocate arrays
	if ( !(pSmf->mtrk = calloc(pSmf->mthd->trkCnt, sizeof(*pSmf->mtrk))) ) {
		LOGF("# Out of memory. Requested %d"EOL, pSmf->mthd->trkCnt * sizeof(*pSmf->mtrk));
		return SETERR(MALLOC), false ;
	}
//!	for (int i = 0;  i < pSmf->mthd->trkCnt;  i++)  pSmf->mtrk[i].chan = -1 ;

	if ( !(pSmf->chunk = calloc(pSmf->cCnt, sizeof(*pSmf->chunk))) ) {
		LOGF("# Out of memory. Requested %d"EOL, pSmf->cCnt * sizeof(*pSmf->chunk));
		return SETERR(MALLOC), false ;
	}

	// Fill in chunk pointers
	LOGF("#   Pass #2 - Index"EOL);

	// Declare (skipped) RIFF header
	if (fileCheckRiff(pSmf->fBuf))
		LOGF("#     @ 00:0000 : Found RIFF Hdr%*s: \"%.4s\""EOL, cW-1,"", pSmf->fBuf);

	tCnt = cCnt = 0;
	for (bp = pSmf->fBof;  bp < pSmf->fEof;  bp += IFFH_SZ + iffh->len) {
		iffh = (iffh_t*)bp;

		// File offset
		LOGF("#     @ %02X:%04X : Found ", (bp - pSmf->fBuf) >> 16, (bp - pSmf->fBuf) & 0xFFFF);

		// MThd
		if        (memcmp(iffh->magic, MTHD_MAGIC, sizeof(iffh->magic)) == 0) {
			LOGF("header %*s: \"%.4s\"[%*d]"EOL, cW,"", iffh->magic, lW, iffh->len);

		// MTrk
		} else if (memcmp(iffh->magic, MTRK_MAGIC, sizeof(iffh->magic)) == 0) {
			uint8_t*  pEot = iffh->data + iffh->len - 3;

			LOGF("track #%*d: \"%.4s\"[%*d]"EOL, cW,tCnt, iffh->magic, lW, iffh->len);

			if ((pEot[0] != 0xFF) || (pEot[1] != 0x2F) || (pEot[2] != 0x00)) {
				LOGF("!   Missing EOT Meta-Event"EOL);
				return SETERR(BAD_TRACK), false ;
			}

			pSmf->mtrk[tCnt].hdr   = iffh;
			pSmf->mtrk[tCnt].data  = ((uint8_t*)iffh) + IFFH_SZ;
			pSmf->mtrk[tCnt].eot   = pSmf->mtrk[tCnt].data + pSmf->mtrk[tCnt].hdr->len;
			pSmf->mtrk[tCnt].trans = (tCnt == (10-1)) ? false : true ;  // don't transpose track 10
			tCnt++;

		// Unknown
		} else {
			LOGF("chunk #%*d: \"%.4s\"[%*d] (0x%02X%02X'%02X%02x)"EOL, cW,cCnt, iffh->magic, lW, iffh->len,
			     iffh->magic[0], iffh->magic[1], iffh->magic[2], iffh->magic[3]);
			pSmf->chunk[cCnt++] = iffh;
		}
	}

	return SETERR(OK), true;
}
