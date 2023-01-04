#include <stdio.h>
#include <string.h>

#include "midp_play.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#if PLATFORM == PLATFORM_PC
//+============================================================================ ========================================
void  _mdevPortClose (void)  {  (void)mdevPortClose();  }

bool  mdevPortOpen (char* name)
{
	strncpy(_p.portOut.name, name, sizeof(_p.portOut.name))[sizeof(_p.portOut.name) - 1] = '\0';

	LOGF("# Open port \"%s\""EOL, _p.portOut.name);

	if ( !(_p.portOut.fh = fopen(_p.portOut.name, "wb")) ) {
		LOGF("! Failed to open port"EOL);
		return SETERR(NOPORT), false;
	}
	
	atexit(_mdevPortClose);

	return true;
}

//+============================================================================ ========================================
bool  mdevPortClose (void)
{
	if (!_p.portOut.fh)  return true ;

	LOGF("# Close port \"%s\""EOL, _p.portOut.name);

	fclose(_p.portOut.fh);
	_p.portOut.fh      = NULL;
	_p.portOut.name[0] = '\0';

	return true;
}

//+============================================================================ ========================================
bool  mdevSendEvent (smf_t* pSmf,  mevent_t* ep)
{
	fwrite(&ep->status, 1, 1,        _p.portOut.fh);
	fwrite(ep->data,    1, ep->dLen, _p.portOut.fh);
	fflush(_p.portOut.fh);

	return true;
}

//+============================================================================ ========================================
// MIDI SyseEx event data does NOT include the prefixing 0xF0
// SysEx files (.syx) DO include the prefixing 0xF0
//
// http://midi.teragonaudio.com/tech/midifile/evts.html
//
bool  mdevSendEsc (uint8_t* bp,  uint32_t len)
{
	fwrite(bp, 1, len, _p.portOut.fh);
	fflush(_p.portOut.fh);

	return true;
}

//+============================================================================ 
bool  mdevSendSyx (uint8_t* bp,  uint32_t len)
{
	fputc(0xF0, _p.portOut.fh);

	return mdevSendEsc(bp, len);
}
#endif//PLATFORM_PC

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#if PLATFORM == PLATFORM_XXX
//+============================================================================ ========================================
// Open the output port
//
void  _mdevPortClose (void)  {  (void)mdevPortClose();  }

bool  mdevPortOpen (char* name)
{
	strncpy(_p.portOut.name, name, sizeof(_p.portOut.name))[sizeof(_p.portOut.name) - 1] = '\0';

	LOGF("# Open port \"%s\""EOL, _p.portOut.name);

	~~~[ OPEN(PORT) ]~~~
	
	if ( !( ~~~[ PORT_IS_OPEN ]~~~ )) ) {
		LOGF("! Failed to open port"EOL);
		return SETERR(NOPORT), false;
	}

	atexit(_mdevPortClose);

	return true;
}

//+============================================================================ ========================================
// Close the output port
//
bool  mdevPortClose (void)
{
	~~~[ CHECK_PORT_ALREADY_CLOSED(PORT) ]~~~

	LOGF("# Close port \"%s\""EOL, _p.portOut.name);

	~~~[ CLOSE(PORT) ]~~~

	return true;
}

//+============================================================================ ========================================
// Send the event to the output port
//
bool  mdevSendEvent (smf_t* pSmf,  mevent_t* ep)
{
	~~~[ SEND(ep->status)         ]~~~
	~~~[ SEND(ep->data, ep->dLen) ]~~~

	return true;
}

//+============================================================================ ========================================
// MIDI SyseEx event data does NOT include the prefixing 0xF0
// SysEx files (.syx) DO include the prefixing 0xF0
//
// http://midi.teragonaudio.com/tech/midifile/evts.html
//
bool  mdevSendEsc (uint8_t* bp,  uint32_t len)
{
	~~~[ SEND(bp, len) ]~~~

	return true;
}

//+============================================================================ 
bool  mdevSendSyx (uint8_t* bp,  uint32_t len)
{
	~~~[ SEND(0xF0) ]~~~

	return mdevSendEsc(bp, len);
}
#endif//PLATFORM_XXX
