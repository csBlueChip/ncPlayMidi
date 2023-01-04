#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "midp_play.h"

//----------------------------------------------------------------------------- ----------------------------------------
mplayer_t  _p = {.smf = NULL} ;

//+============================================================================ ========================================
// size <-- getVlq(*dest, *src)
//
// Read a VLQ from *src, store in *dest, return number of bytes used
//
uint8_t  getVlq (uint32_t* pN,  uint8_t* pVlq)
{
	uint8_t  cnt = 1;
	uint8_t  b;

	if ((*pN = *pVlq++) & 0x80) {
		*pN &= 0x7f;
		do {
			*pN = (*pN << 7) | ((b = *pVlq++) & 0x7F);
		} while (cnt++, b & 0x80) ;
	}
	return cnt;
}

//+============================================================================ ========================================
// Decode the event at bp
// returns the number of bytes used
//
uint32_t  getEvent (smf_t* pSmf,  mtrack_t* pTrk,  mevent_t* ep,  uint8_t* bp)
{
	memset(ep, 0, sizeof(*ep));

	bp += (ep->len = getVlq(&ep->delta, bp));

	// http://midi.teragonaudio.com/tech/midispec/run.htm
	ep->status = *bp;
	if (*bp & 0x80) {
		if      (*bp <= 0xEF)  pTrk->runStat = ep->status ;
		else if (*bp <= 0xF7)  pTrk->runStat = 0 ;
		bp++;
		ep->len++ ;

	} else  ep->status = pTrk->runStat ;

	ep->data = bp;
	switch (ep->status) {
		// {-,1,-,-,4,5,-,-,-,-,-,-,-,D,-,-} (not 9)
		case MSTAT_F1    :    // F1 - undefined
		case MSTAT_F4    :    // F4 - undefined
		case MSTAT_F5    :    // F5 - undefined
		case MSTAT_FD    :    // FD - undefined
		// {-,-,-,-,-,-,6,-,8,9,A,B,C,-,E,-}
		case MSTAT_TUNE  :    // F6 - Tune Request
		case MSTAT_TC    :    // F8 - Timing Clock
		case MSTAT_TICK  :    // F9 - (undefined) / Tick http://midi.teragonaudio.com/tech/midispec/tick.htm
		case MSTAT_START :    // FA - Start
		case MSTAT_CONT  :    // FB - Cont
		case MSTAT_STOP  :    // FC - Stop
		case MSTAT_AS    :    // FE - Active Sensing
			ep->dLen = 0;
			break;

		// {-,-,2,3,-,-,-,-,-,-,-,-,-,-,-,-}
		case MSTAT_SPP   :    // F2 - Song Position Pointer
			ep->len += (ep->dLen = 2);
			break;

		case MSTAT_SS    :    // F3 - Song Select
			ep->len += (ep->dLen = 1);
			break;

		// {0,-,-,-,-,-,-,7,-,-,-,-,-,-,-,-}
		case MSTAT_SYX   :    // F0 - SysEx {F0 ... F7}
		case MSTAT_ESC   : {  // F7 - SysEx {?? ... ??}
			uint32_t  dw = getVlq(&ep->dLen, bp);

			ep->data = bp + dw;
			ep->len += dw + ep->dLen;
			break;
		}

		// http://midi.teragonaudio.com/tech/midispec/reset.htm
		// From a host/device (ie. not a file) 0xFF is a RESET REQUEST
		// {-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,F}
		case MSTAT_META  : {  // FF - Meta Event
			uint32_t  vLen;

			ep->meta.type = *bp++; //& 0x7F;
			bp += (vLen = getVlq(&ep->meta.dLen, bp));
			ep->meta.data = bp;

			ep->len += (ep->dLen = 1 + vLen + ep->meta.dLen);
			break;
		}

		// MIDI
		default:
			ep->len += (ep->dLen = (2 - ((ep->status & 0xE0) == 0xC0)));
			//switch (*bp & 0xF0) {
			//	// {x,x,x,x,x,x,x,x,8,9,A,B,-,-,E,-} : dLen = 3
			//	case 0x80: // Note OFF
			//	case 0x90: // Note ON
			//	case 0xA0: // Key Aftertouch
			//	case 0xB0: // Control Change
			//	case 0xE0: // Pitch Wheel (channel)
			//		ep->len += (ep->dLen = 3);
			//		break;
			//	// {x,x,x,x,x,x,x,x,-,-,-,-,C,D,-,-} : dLen = 2
			//	case 0xC0: // Patch/Program Change
			//	case 0xD0: // Channel Aftertouch
			//		ep->len += (ep->dLen = 2);
			//		break;
			//}
			break;
	}

	return ep->len;
}

//+============================================================================ ========================================
// Play smf[n]
//
bool  midpPlay (uint8_t n,  char* port)
{
	smf_t*  pSmf = &_p.smf[n];

	if (!mdevPortOpen(port))  return SETERR(BADPORT), false ;

	if (pSmf->mthd) {
		LOGF("# Play \"%s\" (format %d)"EOL, pSmf->fn, pSmf->mthd->fmt);

		switch (pSmf->mthd->fmt) {
			case 0:
				LOGF("! fmt0: not written"EOL);
				midpPlay1(pSmf);
				break;
			case 1:
				drawInit();       // Initialise the user interface
				midpPlay1(pSmf);
				break;
			case 2:
				LOGF("! fmt2: not written"EOL);
				break;
			default:
				LOGF("! Unknown format: %d"EOL, pSmf->mthd->fmt);
				break;
		}
	} else if (pSmf->syxCnt) {
		LOGF("# Send SysEx \"%s\""EOL, pSmf->fn);
		mdevSendEsc(pSmf->fBuf, pSmf->fSiz);  // Yes, SendEsc(), NOT SendSyx()
	} else {
		LOGF("# Unrecognised file contents \"%s\""EOL, pSmf->fn);
	}

	// This may cut the fade on some tracks
//! TODO : wait, errr, ¿4 beats? for the last notes to fade.
	LOGF("# Send MIDI \"Panic\""EOL);
	midpPanic(pSmf);

	mdevPortClose();

	return SETERR(OK), true ;
}

//+============================================================================ ========================================
static inline
void  midpTempo (smf_t* pSmf,  mevent_t* ep)
{
	pSmf->uspdt = PEEK24BE(ep->meta.data) / pSmf->mthd->tpqn ;  // uS/QN -> uS/deltaTick
	intrPeriod(pSmf);
	drawTempo(pSmf);
	drawSpeed(pSmf);
}

//+============================================================================ ========================================
void  midpSpeed (smf_t* pSmf,  uint16_t speed)
{
	pSmf->speed = speed; // %
	drawSpeed(pSmf);
	intrPeriod(pSmf);
}

//+============================================================================ ========================================
// Play format 1 track
//
bool  midpPlay1 (smf_t* pSmf)
{
	delta_t    next = DELTA_MAX;
	mtrack_t*  trk  = NULL;
	uint16_t   t;
	mevent_t   event;

	drawFilename(pSmf);

	midpSpeed(pSmf, 100); // %
	pSmf->running = true;

	// Initialise the track Delta Ticks ...and note the first(/next) event
	for (t = 0,  trk = pSmf->mtrk;  t < pSmf->mthd->trkCnt;  t++,  trk++) {
		(void)getVlq(&trk->dNext, (trk->eNext = trk->data));
		if (trk->dNext < next)  next = trk->dNext ;
	}

	intrStart(pSmf);

	while (next != DELTA_MAX) {
		// Wait for interrupt timer to reach the next event
		while (pSmf->delta < next) {
			// callback may return `false` to stop playback
			if (_p.cbPlay) {
				if (!_p.cbPlay(&_p, pSmf)) {
					midpPanic(pSmf);
					pSmf->running = false;
					return SETERR(OK), true ;
				}
			}
			if (pSmf->paused)  next = pSmf->delta + 1 ;
		}

		for (t = 0,  trk = pSmf->mtrk;  t < pSmf->mthd->trkCnt;  t++,  trk++) {
			// Process all events which should happen now, or that we have missed
			while (trk->dNext <= next) {
				// Parse the new event
				trk->eNext += getEvent(pSmf, trk, &event, trk->eNext);
				// Process the event
				if (event.status == MSTAT_META) {
					if (event.meta.type == META_END_OF_TRACK) {
						trk->dNext = DELTA_MAX;
						break; // break while
					}
					
					switch (event.meta.type) {
						case META_TEMPO          :  midpTempo(pSmf, &event);  break ;
						case META_KEY_SIGNATURE  :  drawKeySig(&event);       break ;
						case META_TIME_SIGNATURE :  drawTimeSig(&event);      break ;
					}

				} else if (event.status >= 0xF0) {
					switch (event.status) {
						case MSTAT_SYX :  mdevSendSyx(event.data, event.dLen);  break ;
						case MSTAT_ESC :  mdevSendEsc(event.data, event.dLen);  break ;
					}

				} else {
					uint8_t  cmd  = event.status & 0xF0;
					uint8_t  chan = event.status & 0x0F;
					// if track is musted, do NOT process note-on commands
					// noteOn(n,0) has the same effect noteOff(n,127)
//					if (!trk->mute       || (cmd != MCMD_ON) || (event.data[1] == 0))  // mute note-on
					if (!_kbd[chan].mute || (cmd != MCMD_ON) || (event.data[1] == 0))  // mute note-on
						mdevSendEvent(pSmf, &event);
					drawNote(&event) ;  // On and/or Off

					switch (cmd) {
						case MCMD_PGM :  drawPgm(&event);  break ;
					}
				}

				// Update next delta tick for this track
				if (*trk->eNext) {
					uint32_t  dw;
					(void)getVlq(&dw, trk->eNext);
					trk->dNext += dw;
					break;
				}
			}
		}

		// Find delta tick of next event (we can count down here!)
		next = pSmf->mtrk->dNext;
		for (trk = &pSmf->mtrk[pSmf->mthd->trkCnt - 1];  trk > pSmf->mtrk;  trk--)
			if (trk->dNext < next)  next = trk->dNext ;
	}

	return SETERR(OK), true ;
}

//+============================================================================ ========================================
void  midpPanic (smf_t* pSmf)
{
	uint8_t  off[] = {MCMD_CC, 120, 0, MCMD_CC, 121, 0, MCMD_CC, 123, 0};

	for (int c = 0;  c < 16;  off[0] = off[3] = off[6] = MCMD_CC | ++c)
		mdevSendEsc(off, sizeof(off));
}

//+============================================================================ ========================================
// Initialise the file player engine with 'n' file slots
//
bool  midpInit (int slots)
{
	if (_p.smf)  return SETERR(RUNNING), false ;

	memset(&_p, 0, sizeof(_p));

	if ( !(_p.smf = calloc(slots, sizeof(*_p.smf))) )  return SETERR(MALLOC), false ;
	_p.smfMax = slots;

	LOGF("# Created %d SMF slots"EOL, slots);

	return true;
}

//+============================================================================ ========================================
// Kill the file player engine
//
bool  midpKill (void)
{
	if (!_p.smf)  return SETERR(STOPPED), false;

	for (int i = 0;  i < _p.smfMax;  i++)  if (_p.smf[i].fBuf)  (void)fileUnload(i) ;

	_p.smfMax = 0;
	FREE(_p.smf);

	LOGF("# Destroyed SMF slots"EOL);
	return true;
}

//+============================================================================ ========================================
int  main (int argc,  char* argv[])
{
	(void)midpInit(2);               // Initialise the player engine - room for 2 files {0, .., n-1}
	(void)cbInit();                  // Initialise the user callbacks

	(void)fileLoad(0, argv[1]);      // Load filename to file[0]  (SMF or Syx)

	if (argc == 3) {
		_p.cbPlay = cb_play;         // Set playback callback
		(void)intrInit();            // Initialise the timer interrupt
		(void)midpPlay(0, argv[2]);  // Play song|sysex[0]
		(void)intrKill();            // Initialise the timer interrupt
	} else {
		(void)fileDump(0);           // Dump song[0]
	}

	(void)fileUnload(0);             // Unload song[0]

	(void)midpKill();                // Kill the engine

	return 0;
}
