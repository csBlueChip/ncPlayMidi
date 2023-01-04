#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <arpa/inet.h>

#include "midp_play.h"

//+============================================================================ ========================================
// Decode the specified event
//
void  showEvent (smf_t* pSmf,  mevent_t* ep,  uint8_t trk)
{
	uint16_t  w;
	uint32_t  dw;
	uint32_t  i;

	switch (ep->status) {
		case MSTAT_F1    :   // F1 - undefined
		case MSTAT_F4    :   // F4 - undefined
		case MSTAT_F5    :   // F5 - undefined
		case MSTAT_FD    :   // FD - undefined
			LOGF("Undefined event 0x%02X", ep->status);
			return;

		// {-,-,-,-,-,-,6,-,8,9,A,B,C,-,E,-}
		case MSTAT_TUNE  :  LOGF("Tune Request");    break;  // F6
		case MSTAT_TC    :  LOGF("Timing Clock");    break;  // F8
		case MSTAT_TICK  :  LOGF("Tick");            break;  // F9 - http://midi.teragonaudio.com/tech/midispec/tick.htm
		case MSTAT_START :  LOGF("Start");           break;  // FA
		case MSTAT_CONT  :  LOGF("Continue");        break;  // FB
		case MSTAT_STOP  :  LOGF("Stop");            break;  // FC
		case MSTAT_AS    :  LOGF("Active Sensing");  break;  // FE

		// {-,-,2,3,-,-,-,-,-,-,-,-,-,-,-,-}
		case MSTAT_SPP   :   // F2
			LOGF("Song Pointer: %d * 6 = %d", (w = PEEK14LE(ep->data)), w * 6);
			return;
		case MSTAT_SS    :   // F3 - Song Select
			LOGF("Song Select: %d", *ep->data);
			return;

		// {-,-,-,-,-,-,-,-,-,-,-,-,-,-,-,F}
		case MSTAT_META  :   // FF - Meta Event
			showMeta(pSmf, ep, trk);
			return;

		// MIDI
		default:
			showMidi(ep);
			return;

		// {0,-,-,-,-,-,-,7,-,-,-,-,-,-,-,-}
		case MSTAT_SYX   :   // F0 - SysEx Start
			LOGF("Syx  = {(F0), ");
			break;

		case MSTAT_ESC   :   // F7 - SysEx End
			LOGF("Esc  = {");
			break;
	}
	// MSTAT_SYX & MSTAT_ESC -> Dump data
	if ((dw = ep->dLen))  dw-- ;
	for (i = 0;  i < dw;  i++)  LOGF("%02X, ", ep->data[i]) ;
	if (ep->dLen) LOGF("%02X", ep->data[i]) ;
	if (ep->status == MSTAT_SYX)  LOGF("}[1 + %d = %d]", ep->dLen, ep->dLen + 1);
	else                          LOGF("}[%d]"         , ep->dLen);
}

//+============================================================================ ========================================
// Decode Meta Event
//
void  showMeta (smf_t* pSmf,  mevent_t* ep,  uint8_t trk)
{
	LOGF("Meta = %02X[%2d] - ", ep->meta.type, ep->meta.dLen);
	switch (ep->meta.type) {
		case META_SEQUENCE_NUM        :    // 0x00
			LOGF("Sequence Number");
			if (ep->meta.dLen)  LOGF(": %d", ntohs(*((uint16_t*)ep->meta.data)));
			else                LOGF(": %d", trk);
			return;

		case META_MIDI_CHANNEL_PREFIX :    // 0x20 - https://www.recordingblogs.com/wiki/midi-channel-prefix-meta-message
			LOGF("Midi Channel Prefix: %d", ep->meta.data[0]);
			return;

		case META_MIDI_PORT           :    // 0x21
			LOGF("Midi Port Prefix: %d", ep->meta.data[0]);
			return;

		case META_END_OF_TRACK        :    // 0x2F - REQUIRED
			LOGF("End of Track");
			return;

		case META_TEMPO               : {  // 0x51
			uint32_t  ui24 = PEEK24BE(ep->meta.data);
			LOGF( "Tempo: %d uS/qn / %d tpqn = %d uS/dt = %d bpm",
			      ui24, pSmf->mthd->tpqn, ui24 / pSmf->mthd->tpqn,  (60 * 1000000) / ui24 );
			return;
		}

		case META_SMPTE_OFFSET        :    // 0x54
			LOGF( "SMPTE Offset: %d:%d:%d.%d_%d  (fps=%d_fpf=100)",  // frames/sec fractions/frame
			      ep->meta.data[0], ep->meta.data[1], ep->meta.data[2],
				  ep->meta.data[3], ep->meta.data[4], pSmf->mthd->fps );
			return;

		case META_TIME_SIGNATURE      :    // 0x58 - http://midi.teragonaudio.com/tech/midifile/time.htm
			LOGF( "Time Signature: %d/%d ; %d clocks/click ; %d 32nds/qn",
			      ep->meta.data[0], (1 << ep->meta.data[1]), ep->meta.data[2], ep->meta.data[3] );
			return;

		case META_KEY_SIGNATURE       :    // 0x59
			LOGF("Key Signature: ");
			showKeySig(ep->meta.data);
			return;

		case META_SEQUENCER_EVENT     : {  // 0x7F
			uint32_t  dw = 0;
			uint8_t*  bp = ep->meta.data;
			bp += getVlq(&dw, bp);
			LOGF("Sequencer Event: {");
			if (dw)  dw-- ;
			for (uint32_t i = 0;  i < dw;  i++)  LOGF("%02X, ", *bp++) ;
			LOGF("%02X}", *bp) ;
			return;
		}

		default:
			LOGF("-UNKNOWN-");
			return;

		case META_TEXT            :  LOGF("Text"               );  break;  // 0x01
		case META_COPYRIGHT       :  LOGF("Copyright"          );  break;  // 0x02
		case META_SEQUENCE_NAME   :  LOGF("Sequence/Track Name");  break;  // 0x03
		case META_INSTRUMENT_NAME :  LOGF("Instrument Name"    );  break;  // 0x04
		case META_LYRIC           :  LOGF("Lyric"              );  break;  // 0x05
		case META_MARKER          :  LOGF("Marker"             );  break;  // 0x06
		case META_CUE_POINT       :  LOGF("Cue Point"          );  break;  // 0x07
		case META_PROGRAM_NAME    :  LOGF("Patch/Program Name" );  break;  // 0x08
		case META_DEVICE_NAME     :  LOGF("Device Name"        );  break;  // 0x09
	}
	// Show text for text events
	LOGF(": \"%.*s\"", ep->meta.dLen, ep->meta.data);

	return;
}

//+============================================================================ ========================================
// Decode a MIDI Event
//
void  showMidi (mevent_t* ep)
{
	char  name[4] = {'\0', '\0', '\0', '\0'};

	LOGF("MIDI = @%2d : ", (ep->status & 0x0F) +1);  // User-friendly channel number is n+1

	switch (ep->status & 0xF0) {
		case MCMD_OFF   :    // 0x80 - [1] Note Off
			LOGF("Note OFF %s / %d"                              , note2name(name, ep->data[0]), ep->data[1]);
			break;
		case MCMD_ON    :    // 0x90 - [1] Note On
			LOGF("Note %s %s / %d", (ep->data[1] ? "ON " : "off"), note2name(name, ep->data[0]), ep->data[1]);
			break;
		case MCMD_KEYAFT:    // 0xA0 - [1] Key Aftertouch
			LOGF("Key Aft %s / %d"                               , note2name(name, ep->data[0]), ep->data[1]);
			break;

		//! This could do with some more work : BANK & all (N)RPN controls
		case MCMD_CC    :    // 0xB0 - [1] Controller Change
			LOGF("Ctrl Chg ");
			switch (ep->data[0]) {
				case CTRL_PORT_CTRL:
					LOGF( "(PortRoot):   %3d = %3d (%s) .. %s", ep->data[0], ep->data[1],
					      note2name(name, ep->data[1]), ctrl2name(ep->data[0]) );
					break;

				case CTRL_DATA_INC:
				case CTRL_DATA_DEC:
					LOGF("(Data+/-) :   %3d (%3d) .. %s", ep->data[0], ep->data[1], ctrl2name(ep->data[0]));
					break;

				case CTRL_LOCAL:
					LOGF( "(System)  :   %3d = %3d (%s) .. %s", ep->data[0], ep->data[1],
					      (ep->data[1] == 0) ? "OFF" : ((ep->data[1] == 127) ? "ON" : "???"),
					      ctrl2name(ep->data[0]) );
					break;

				case CTRL_MONO:
					LOGF("(System)  :   %3d = %3d .. %s", ep->data[0], ep->data[1], ctrl2name(ep->data[0]));
					break;

				case CTRL_PAN_MSB:
				case CTRL_BAL_MSB:
					if      (ep->data[1] < 64)
						LOGF( "(User)    :   %3d = %3d (%d%%<--) .. %s", ep->data[0], ep->data[1],
						      (((((64 - ep->data[1]) * 1000) / 64) + 5) / 10), ctrl2name(ep->data[0]) );
					else if (ep->data[1] > 64)
						LOGF( "(User)    :   %3d = %3d (-->%d%%) .. %s", ep->data[0], ep->data[1],
						      (((((ep->data[1] - 63) * 1000) / 64) + 5) / 10), ctrl2name(ep->data[0]) );
					else
						LOGF( "(User)    :   %3d = %3d (centre) .. %s",
						      ep->data[0], ep->data[1], ctrl2name(ep->data[0]) );
					break;
							  
				default:
					if      ((ep->data[0] >= CTRL_PDL_DAMPER) && (ep->data[0] <= CTRL_PDL_HOLD))
						LOGF("(Pedal)   :   %3d = %3d (%s) .. %s", ep->data[0], ep->data[1],
							(ep->data[1] < 64) ? "OFF" : "ON", ctrl2name(ep->data[0]));
		
					else if (ep->data[0] >= CTRL_ALL_SOUND_OFF)
						LOGF("(System)  :   %3d = (zero=%d) .. %s", ep->data[0], ep->data[1], ctrl2name(ep->data[0]));
		
					else
						LOGF("(User)    :   %3d = %3d (%3d%) .. %s",
							ep->data[0], ep->data[1], (ep->data[1] * 100ul) / 127, ctrl2name(ep->data[0]));
					break;
			}
			break;

		case MCMD_PGM   :    // 0xC0 - [3] Patch/Program Change
			LOGF("Patch change       :   %3d ~ %s", ep->data[0], patch2nameGM(ep->data[0]));
			break;

		case MCMD_CHNAFT:    // 0xD0 - [3] Channel Aftertouch
			LOGF("Channel Aftertouch =   %3d", ep->data[0]);
			break;

		case MCMD_WHEEL : {  // 0xE0 - [1] Pitch Wheel
			uint16_t  pw = PEEK14LE(&ep->data[0]);
			LOGF("Pitch Wheel        = ");
			if      (pw < 0x2000)  LOGF("-%04X (-%4d)", (0x2000 - pw), (0x2000 - pw)) ;
			else if (pw > 0x2000)  LOGF("+%04X (+%4d)", (pw - 0x2000), (pw - 0x2000)) ;
			else                   LOGF("    0 (centre)");
			break;
		}

		default:
			LOGF("-UNKNOWN-");
			break;
	}
}

//+============================================================================ ========================================
// Show the key signature from Meta data
//
void  showKeySig (uint8_t* bp)
{
	int8_t  sf = (int8_t)*bp++;
	if (*bp) {
		if        (sf < 0) {  // flats
			LOGF("%c%sm (%d flats)",  "DGCFBEA"[-sf], ((sf >= -4) ? "" : "b"), sf);
		} else if (sf > 0) {  // sharps
			LOGF("%c%sm (%d sharps)", "EBFCGDA"[sf] , ((sf <=  2) ? "" : "#"), sf);
		} else {                      // C/Am
			LOGF("Am");
		}
	} else {
		if        (sf < 0) {  // flats
			LOGF("%c%s (%d flats)",  "FBEADGC"[-sf], ((sf == -1) ? "" : "b"), sf);
		} else if (sf > 0) {  // sharps
			LOGF("%c%s (%d sharps)", "GDAEBFC"[sf] , ((sf <=  5) ? "" : "#"), sf);
		} else {                      // C/Am
			LOGF("C");
		}
	}
}

//+============================================================================ ========================================
// You MUST supply a >=4 byte buffer
//
char*  note2name (char* name,  uint8_t val)
{
	sprintf( name, "%c%s%d%s",
	         "CCDDEFFGGAAB"[val % 12],
	         " # #  # # # "[val % 12] == '#' ? "#" : "",
	         (val / 12) - 1,
			 " # #  # # # "[val % 12] == '#' ? "" : " " );
	return name;
}

//+============================================================================ ========================================
// Simply Displays the GM instrument name
// Does NOT factor in the Bank Number!
//
char*  patch2nameGM (uint8_t val)
{
//#define  NAME(group,instrument,abbr)  group " - " instrument " (" abbr ")"
#define  NAME(group,instrument,abbr)  abbr

	switch (val) {
		case   0:  return(NAME("Piano"               , "Acoustic Grand Piano"             , "Grand Piano"    ));
		case   1:  return(NAME("Piano"               , "Bright Acoustic Piano"            , "Bright Piano"   ));
		case   2:  return(NAME("Piano"               , "Electric Grand Piano"             , "Electric Grand" ));
		case   3:  return(NAME("Piano"               , "Honky-tonk Piano"                 , "Honky-tonk Pno" ));
		case   4:  return(NAME("Piano"               , "Electric Piano 1 (Rhodes Piano)"  , "Rhodes Piano"   ));
		case   5:  return(NAME("Piano"               , "Electric Piano 2 (Chorused Piano)", "Chorus Piano"   ));
		case   6:  return(NAME("Piano"               , "Harpsichord"                      , "Harpsichord"    ));
		case   7:  return(NAME("Piano"               , "Clavinet"                         , "Clavinet"       ));
		case   8:  return(NAME("Chromatic Percussion", "Celesta"                          , "Celesta"        ));
		case   9:  return(NAME("Chromatic Percussion", "Glockenspiel"                     , "Glockenspiel"   ));
		case  10:  return(NAME("Chromatic Percussion", "Music Box"                        , "Music Box"      ));
		case  11:  return(NAME("Chromatic Percussion", "Vibraphone"                       , "Vibraphone"     ));
		case  12:  return(NAME("Chromatic Percussion", "Marimba"                          , "Marimba"        ));
		case  13:  return(NAME("Chromatic Percussion", "Xylophone"                        , "Xylophone"      ));
		case  14:  return(NAME("Chromatic Percussion", "Tubular Bells"                    , "Tubular Bells"  ));
		case  15:  return(NAME("Chromatic Percussion", "Dulcimer (Santur)"                , "Dulcimer"       ));
		case  16:  return(NAME("Organ"               , "Drawbar Organ (Hammond)"          , "Hammond Organ"  ));
		case  17:  return(NAME("Organ"               , "Percussive Organ"                 , "Perc. Organ"    ));
		case  18:  return(NAME("Organ"               , "Rock Organ"                       , "Rock Organ"     ));
		case  19:  return(NAME("Organ"               , "Church Organ"                     , "Church Organ"   ));
		case  20:  return(NAME("Organ"               , "Reed Organ"                       , "Reed Organ"     ));
		case  21:  return(NAME("Organ"               , "Accordion (French)"               , "French Accord." ));
		case  22:  return(NAME("Organ"               , "Harmonica"                        , "Harmonica"      ));
		case  23:  return(NAME("Organ"               , "Tango Accordion (Band neon)"      , "Tango Accord."  ));
		case  24:  return(NAME("Guitar"              , "Acoustic Guitar (nylon)"          , "Nylon Guitar"   ));
		case  25:  return(NAME("Guitar"              , "Acoustic Guitar (steel)"          , "Steel Guitar"   ));
		case  26:  return(NAME("Guitar"              , "Electric Guitar (jazz)"           , "Jazz Guitar"    ));
		case  27:  return(NAME("Guitar"              , "Electric Guitar (clean)"          , "Clean Guitar"   ));
		case  28:  return(NAME("Guitar"              , "Electric Guitar (muted)"          , "Muted Guitar"   ));
		case  29:  return(NAME("Guitar"              , "Overdriven Guitar"                , "OverDrive Gtr"  ));
		case  30:  return(NAME("Guitar"              , "Distortion Guitar"                , "Distorted Gtr"  ));
		case  31:  return(NAME("Guitar"              , "Guitar harmonics"                 , "Gtr Harmonics"  ));
		case  32:  return(NAME("Bass"                , "Acoustic Bass"                    , "Acoustic Bass"  ));
		case  33:  return(NAME("Bass"                , "Electric Bass (fingered)"         , "Fingered Bass"  ));
		case  34:  return(NAME("Bass"                , "Electric Bass (picked)"           , "Picked Bass"    ));
		case  35:  return(NAME("Bass"                , "Fretless Bass"                    , "Fretless Bass"  ));
		case  36:  return(NAME("Bass"                , "Slap Bass 1"                      , "Slap Bass #1"   ));
		case  37:  return(NAME("Bass"                , "Slap Bass 2"                      , "Slap Bass #2"   ));
		case  38:  return(NAME("Bass"                , "Synth Bass 1"                     , "Synth Bass #1"  ));
		case  39:  return(NAME("Bass"                , "Synth Bass 2"                     , "Synth Bass #2"  ));
		case  40:  return(NAME("Strings"             , "Violin"                           , "Violin"         ));
		case  41:  return(NAME("Strings"             , "Viola"                            , "Viola"          ));
		case  42:  return(NAME("Strings"             , "Cello"                            , "Cello"          ));
		case  43:  return(NAME("Strings"             , "Contrabass"                       , "Contrabass"     ));
		case  44:  return(NAME("Strings"             , "Tremolo Strings"                  , "Trem. Strings"  ));
		case  45:  return(NAME("Strings"             , "Pizzicato Strings"                , "Pizz. Strings"  ));
		case  46:  return(NAME("Strings"             , "Orchestral Harp"                  , "Harp"           ));
		case  47:  return(NAME("Strings"             , "Timpani"                          , "Timpani"        ));
		case  48:  return(NAME("Ensemble"            , "String Ensemble 1 (strings)"      , "String Ensemble"));
		case  49:  return(NAME("Ensemble"            , "String Ensemble 2 (slow strings)" , "Slow Strings"   ));
		case  50:  return(NAME("Ensemble"            , "SynthStrings 1"                   , "SynthStrings #1"));
		case  51:  return(NAME("Ensemble"            , "SynthStrings 2"                   , "SynthStrings #2"));
		case  52:  return(NAME("Ensemble"            , "Choir Aahs"                       , "Choir Aahs"     ));
		case  53:  return(NAME("Ensemble"            , "Voice Oohs"                       , "Voice Oohs"     ));
		case  54:  return(NAME("Ensemble"            , "Synth Voice"                      , "Synth Voice"    ));
		case  55:  return(NAME("Ensemble"            , "Orchestra Hit"                    , "Orchestra Hit"  ));
		case  56:  return(NAME("Brass"               , "Trumpet"                          , "Trumpet"        ));
		case  57:  return(NAME("Brass"               , "Trombone"                         , "Trombone"       ));
		case  58:  return(NAME("Brass"               , "Tuba"                             , "Tuba"           ));
		case  59:  return(NAME("Brass"               , "Muted Trumpet"                    , "Muted Trumpet"  ));
		case  60:  return(NAME("Brass"               , "French Horn"                      , "French Horn"    ));
		case  61:  return(NAME("Brass"               , "Brass Section"                    , "Brass Section"  ));
		case  62:  return(NAME("Brass"               , "SynthBrass 1"                     , "SynthBrass 1"   ));
		case  63:  return(NAME("Brass"               , "SynthBrass 2"                     , "SynthBrass 2"   ));
		case  64:  return(NAME("Reed"                , "Soprano Sax"                      , "Soprano Sax"    ));
		case  65:  return(NAME("Reed"                , "Alto Sax"                         , "Alto Sax"       ));
		case  66:  return(NAME("Reed"                , "Tenor Sax"                        , "Tenor Sax"      ));
		case  67:  return(NAME("Reed"                , "Baritone Sax"                     , "Baritone Sax"   ));
		case  68:  return(NAME("Reed"                , "Oboe"                             , "Oboe"           ));
		case  69:  return(NAME("Reed"                , "English Horn"                     , "English Horn"   ));
		case  70:  return(NAME("Reed"                , "Bassoon"                          , "Bassoon"        ));
		case  71:  return(NAME("Reed"                , "Clarinet"                         , "Clarinet"       ));
		case  72:  return(NAME("Pipe"                , "Piccolo"                          , "Piccolo"        ));
		case  73:  return(NAME("Pipe"                , "Flute"                            , "Flute"          ));
		case  74:  return(NAME("Pipe"                , "Recorder"                         , "Recorder"       ));
		case  75:  return(NAME("Pipe"                , "Pan Flute"                        , "Pan Flute"      ));
		case  76:  return(NAME("Pipe"                , "Blown Bottle"                     , "Blown Bottle"   ));
		case  77:  return(NAME("Pipe"                , "Shakuhachi"                       , "Shakuhachi"     ));
		case  78:  return(NAME("Pipe"                , "Whistle"                          , "Whistle"        ));
		case  79:  return(NAME("Pipe"                , "Ocarina"                          , "Ocarina"        ));
		case  80:  return(NAME("Synth Lead"          , "Lead 1 (square wave)"             , "Lead: Square"   ));
		case  81:  return(NAME("Synth Lead"          , "Lead 2 (sawtooth wave)"           , "Lead: Sawtooth" ));
		case  82:  return(NAME("Synth Lead"          , "Lead 3 (calliope)"                , "Lead: Calliope" ));
		case  83:  return(NAME("Synth Lead"          , "Lead 4 (chiffer)"                 , "Lead: Chiffer"  ));
		case  84:  return(NAME("Synth Lead"          , "Lead 5 (charang)"                 , "Lead: Charang"  ));
		case  85:  return(NAME("Synth Lead"          , "Lead 6 (voice solo)"              , "Lead: Voice"    ));
		case  86:  return(NAME("Synth Lead"          , "Lead 7 (fifths)"                  , "Lead: Fifths"   ));
		case  87:  return(NAME("Synth Lead"          , "Lead 8 (bass + lead)"             , "Lead: Bass+Lead"));
		case  88:  return(NAME("Synth Pad"           , "Pad 1 (new age Fantasia)"         , "Pad: Fantasia"  ));
		case  89:  return(NAME("Synth Pad"           , "Pad 2 (warm)"                     , "Pad: Warm"      ));
		case  90:  return(NAME("Synth Pad"           , "Pad 3 (polysynth)"                , "Pad: Polysynth" ));
		case  91:  return(NAME("Synth Pad"           , "Pad 4 (choir space voice)"        , "Pad: Choir"     ));
		case  92:  return(NAME("Synth Pad"           , "Pad 5 (bowed glass)"              , "Pad: BowedGlass"));
		case  93:  return(NAME("Synth Pad"           , "Pad 6 (metallic pro)"             , "Pad: Metallic"  ));
		case  94:  return(NAME("Synth Pad"           , "Pad 7 (halo)"                     , "Pad: Halo"      ));
		case  95:  return(NAME("Synth Pad"           , "Pad 8 (sweep)"                    , "Pad: Sweep"     ));
		case  96:  return(NAME("Synth Effects"       , "FX 1 (rain)"                      , "FX: Rain"       ));
		case  97:  return(NAME("Synth Effects"       , "FX 2 (soundtrack)"                , "FX: Soundtrack" ));
		case  98:  return(NAME("Synth Effects"       , "FX 3 (crystal)"                   , "FX: Crystal"    ));
		case  99:  return(NAME("Synth Effects"       , "FX 4 (atmosphere)"                , "FX: Atmosphere" ));
		case 100:  return(NAME("Synth Effects"       , "FX 5 (brightness)"                , "FX: Brightness" ));
		case 101:  return(NAME("Synth Effects"       , "FX 6 (goblins)"                   , "FX: Goblins"    ));
		case 102:  return(NAME("Synth Effects"       , "FX 7 (echoes, drops)"             , "FX: Echo Drop"  ));
		case 103:  return(NAME("Synth Effects"       , "FX 8 (sci-fi, star theme)"        , "FX: Star Theme" ));
		case 104:  return(NAME("Ethnic"              , "Sitar"                            , "Sitar"          ));
		case 105:  return(NAME("Ethnic"              , "Banjo"                            , "Banjo"          ));
		case 106:  return(NAME("Ethnic"              , "Shamisen"                         , "Shamisen"       ));
		case 107:  return(NAME("Ethnic"              , "Koto"                             , "Koto"           ));
		case 108:  return(NAME("Ethnic"              , "Kalimba"                          , "Kalimba"        ));
		case 109:  return(NAME("Ethnic"              , "Bag pipe"                         , "Bag pipe"       ));
		case 110:  return(NAME("Ethnic"              , "Fiddle"                           , "Fiddle"         ));
		case 111:  return(NAME("Ethnic"              , "Shanai"                           , "Shanai"         ));
		case 112:  return(NAME("Percussive"          , "Tinkle Bell"                      , "Tinkle Bell"    ));
		case 113:  return(NAME("Percussive"          , "Agogo"                            , "Agogo"          ));
		case 114:  return(NAME("Percussive"          , "Steel Drums"                      , "Steel Drums"    ));
		case 115:  return(NAME("Percussive"          , "Woodblock"                        , "Woodblock"      ));
		case 116:  return(NAME("Percussive"          , "Taiko Drum"                       , "Taiko Drum"     ));
		case 117:  return(NAME("Percussive"          , "Melodic Tom"                      , "Melodic Tom"    ));
		case 118:  return(NAME("Percussive"          , "Synth Drum"                       , "Synth Drum"     ));
		case 119:  return(NAME("Percussive"          , "Reverse Cymbal"                   , "Reverse Cymbal" ));
		case 120:  return(NAME("Sound Effects"       , "Guitar Fret Noise"                , "FX: Fret Noise" ));
		case 121:  return(NAME("Sound Effects"       , "Breath Noise"                     , "FX: Breath"     ));
		case 122:  return(NAME("Sound Effects"       , "Seashore"                         , "FX: Seashore"   ));
		case 123:  return(NAME("Sound Effects"       , "Bird Tweet"                       , "FX: Bird Tweet" ));
		case 124:  return(NAME("Sound Effects"       , "Telephone Ring"                   , "FX: Telephone"  ));
		case 125:  return(NAME("Sound Effects"       , "Helicopter"                       , "FX: Helicopter" ));
		case 126:  return(NAME("Sound Effects"       , "Applause"                         , "FX: Applause"   ));
		case 127:  return(NAME("Sound Effects"       , "Gunshot"                          , "FX: Gunshot"    ));
		default:   return("*bad patch number*");                                                               
	}
#undef NAME
}

//+============================================================================ ========================================
// Convert controller number to known name
//
char*  ctrl2name (uint8_t val)
{
	switch (val) {
		// -- PatchBank selection
		case CTRL_BANK_MSB        :  return("[GS][GM2] Bank Select MSB"      );  //   0
		case CTRL_BANK_LSB        :  return("[GS][GM2] Bank Select LSB"      );  //  32

		// -- Basic mixing - GM/GM2/GS/XG only specify Coarse/MSB tuning
		case CTRL_BAL_MSB         :  return("Stereo Balance MSB*"            );  //   8
		case CTRL_BAL_LSB         :  return("Stereo Balance LSB"             );  //  40

		case CTRL_PAN_MSB         :  return("Mono Position (Pan) MSB*"       );  //  10
		case CTRL_PAN_LSB         :  return("Mono Position (Pan) LSB"        );  //  42

		case CTRL_VOLUME_MSB      :  return("[GM][GM2] Volume MSB*"          );  //   7
		case CTRL_VOLUME_LSB      :  return("[GM][GM2] Volume LSB"           );  //  39

		case CTRL_EXPR_MSB        :  return("Expression (fade) MSB*"         );  //  11
		case CTRL_EXPR_LSB        :  return("Expression (fade) LSB"          );  //  43

		// -- Extended effects
		case CTRL_USRFX1_MSB      :  return("Effects Controller #1 MSB"      );  //  12
		case CTRL_USRFX1_LSB      :  return("Effects Controller #1 LSB"      );  //  44

		case CTRL_USRFX2_MSB      :  return("Effects Controller #2 MSB"      );  //  13
		case CTRL_USRFX2_LSB      :  return("Effects Controller #2 LSB"      );  //  45

		// -- Control sliders
		case CTRL_SLIDER1         :  return("Slider #1"                      );  //  16
		case CTRL_SLIDER2         :  return("Slider #2"                      );  //  17
		case CTRL_SLIDER3         :  return("Slider #3"                      );  //  18
		case CTRL_SLIDER4         :  return("Slider #4"                      );  //  19

		// -- Control buttons
		case CTRL_BUTTON1         :  return("Button #1"                      );  //  80
		case CTRL_BUTTON2         :  return("Button #2"                      );  //  81
		case CTRL_BUTTON3         :  return("Button #3"                      );  //  82
		case CTRL_BUTTON4         :  return("Button #4"                      );  //  83

		// -- Piano pedals : https://www.flowkey.com/en/piano-guide/piano-pedals
		// -- 0 <= off < 64 <= on <= 127
		case CTRL_PDL_DAMPER      :  return("[GM][GM2] Damper Pedal"         );  //  64 - pedal - aka "sustain"/"hold-1"
		case CTRL_PDL_PORT        :  return("[GS][GM2] Portamento Pedal"     );  //  65 - pedal
		case CTRL_PDL_SUSTENUTO   :  return("[GS][GM2] Sustenuto Pedal"      );  //  66 - pedal
		case CTRL_PDL_SOFT        :  return("[GS][GM2] Soft Pedal"           );  //  67 - pedal
		case CTRL_PDL_LEGATO      :  return("Legato Pedal"                   );  //  68 - pedal
		case CTRL_PDL_HOLD        :  return("Hold Pedal"                     );  //  69 - pedal - aka "hold-2" - longer decay

		// -- Basic sound controllers - GM/GM2/GS/XG only specify Coarse/MSB tuning
		case CTRL_MOD_MSB         :  return("[GM][GM2] Modulation wheel MSB*");  //   1
		case CTRL_MOD_LSB         :  return("[GM][GM2] Modulation wheel LSB" );  //  33

		case CTRL_BREATH_MSB      :  return("[GM2] Breath Control MSB*"      );  //   2
		case CTRL_BREATH_LSB      :  return("[GM2] Breath Control LSB"       );  //  34
		case CTRL_FOOT_MSB        :  return("[GM2] Foot Control MSB*"        );  //   4
		case CTRL_FOOT_LSB        :  return("[GM2] Foot Control LSB"         );  //  36

		// -- Portamento controls
		case CTRL_PORT_TIME_MSB   :  return("[GM2] Portamento Time MSB*"     );  //   5
		case CTRL_PORT_TIME_LSB   :  return("[GM2] Portamento Time LSB"      );  //  37
		case CTRL_PORT_CTRL       :  return("[GS][XG] Portamento Control"    );  //  84 - trigger

		// -- Basic Effects
		case CTRL_FX1             :  return("FX-1 Send ([GS] Reverb)"        );  //  91
		case CTRL_FX2             :  return("FX-2 Send ([GS] Termulo)"       );  //  92
		case CTRL_FX3             :  return("FX-3 Send ([GS] Chorus)"        );  //  93
		case CTRL_FX4             :  return("FX-4 Send (Celeste, [GS] Delay/Variation, [GM] Detune)");  //  94
		case CTRL_FX5             :  return("FX-5 Send (Phaser)"             );  //  95

		// -- Sound Controllers
		case CTRL_SOUND1          :  return("Sound #1 ([GM2] Sound variaion)");  //  70
		case CTRL_SOUND2          :  return("Sound #2 ([GM2] ? Timbre/Filter Cutoff/Filter Resonance)");  //  71
		case CTRL_SOUND3          :  return("Sound #3 ([GM2] Release Time)"  );  //  72
		case CTRL_SOUND4          :  return("Sound #4 ([GM2] Attack Time)"   );  //  73
		case CTRL_SOUND5          :  return("Sound #5 ([GM2] ? Brightness/Filter Resonance/Filter Cutoff/Timbre)");  //  74
		case CTRL_SOUND6          :  return("Sound #6 ([GM2] Decay Time)"    );  //  75
		case CTRL_SOUND7          :  return("Sound #7 ([GM2] Vibrato Rate "  );  //  76
		case CTRL_SOUND8          :  return("Sound #8 ([GM2] Vibrato Depth " );  //  77
		case CTRL_SOUND9          :  return("Sound #9 ([GM2] Vibrato Delay " );  //  78
		case CTRL_SOUND10         :  return("Sound #10"                      );  //  79

		// -- (N)RPN controls
		case CTRL_RPN_LSB         :  return("[GS][GM][GM2] RPN LSB"          );  // 100
		case CTRL_RPN_MSB         :  return("[GS][GM][GM2] RPN MSB"          );  // 101

		case CTRL_NRPN_LSB        :  return("[GS] NRPN LSB"                  );  //  98
		case CTRL_NRPN_MSB        :  return("[GS] NRPN MSB"                  );  //  99

		case CTRL_DATA_MSB        :  return("[GM2] Data Entry MSB"           );  //   6
		case CTRL_DATA_LSB        :  return("[GM2] Data Entry LSB"           );  //  38

		case CTRL_DATA_INC        :  return("[GM2] Data Increment (+1)"      );  //  96 - trigger
		case CTRL_DATA_DEC        :  return("[GM2] Data Decrement (-1)"      );  //  97 - trigger

		// -- System Controllers
		case CTRL_ALL_SOUND_OFF   :  return("Sys: All Sound off"             );  // 120
		case CTRL_ALL_CTRL_OFF    :  return("Sys: All Controllers off"       );  // 121
		case CTRL_LOCAL           :  return("Sys: Local Mode"                );  // 122
		case CTRL_ALL_NOTES_OFF   :  return("Sys: All Notes off"             );  // 123
		case CTRL_OMNI_OFF        :  return("Sys: Omni Mode off"             );  // 124
		case CTRL_OMNI_ON         :  return("Sys: Omni Mode on"              );  // 125
		case CTRL_MONO            :  return("Sys: Mono Mode"                 );  // 126
		case CTRL_POLY            :  return("Sys: Poly Mode"                 );  // 127

		default                   :  return("Undefined");
	}
}
