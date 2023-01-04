#ifndef  PLAY_MIDI_H_
#define  PLAY_MIDI_H_

//----------------------------------------------------------------------------- ----------------------------------------
// Currently defines:
// # [midp_dev.c ] Device controls    - midi port open/close/send/...
// # [midp_intr.c] Interrupt controls - interrupt timer start/stop/pause/...
//
#define PLATFORM_PC   1
#define PLATFORM_XXX  2

#ifndef PLATFORM                  // Allow PLATFORM to be defined on the compiler command line
#	define PLATFORM  PLATFORM_PC  // Default platform
#endif

//----------------------------------------------------------------------------- ----------------------------------------
// Magic number shenanigans
//
#define  MTHD_MAGIC  "MThd"
#define  MTHD_LEN    (3 * (16/8))  // 3 x 16bit values
#define  MTRK_MAGIC  "MTrk"
#define  RIFF_MAGIC  "RIFF"

//----------------------------------------------------------------------------- ----------------------------------------
// Compiler and platform specific stuff
//
#define  PACK  __attribute__((__packed__))
#define  EOL  "\r\n"

//----------------------------------------------------------------------------- ----------------------------------------
// Debug logging on/off
//
#define  LOGGING 1  // 1=On, 0=Off

#if LOGGING == 1
#	include <stdio.h>
#	define  LOGF(...)  do {  printf(__VA_ARGS__);  fflush(stdout);  } while(0)
//#	define  LOGF(...)  do {  printf(__VA_ARGS__);  } while(0)
#else
#	define  LOGF(...)
#endif

#define  DRAW(...)  do {  printf(__VA_ARGS__);  fflush(stdout);  } while(0)

//----------------------------------------------------------------------------- ----------------------------------------
#define  SETERR(n) (0)

#include <stdlib.h>
#define  FREE(p)   do { if (p) { free(p);  (p) = NULL; } } while(0)

//----------------------------------------------------------------------------- ----------------------------------------
// I have taken my controller list from here:
//    http://midi.teragonaudio.com/tech/midispec.htm
//    https://en.wikipedia.org/wiki/General_MIDI
//    https://en.wikipedia.org/wiki/General_MIDI_Level_2
// Note that, generally, LSB = (MSB +  32 ) ...  MSB = (LSB -   32 )
//    or, if you prefer: LSB = (MSB | 0x20) ...  MSB = (LSB & ~0x20)
//
// A [trigger] controller triggers/happens when ANY value is written to it
//
// The six Pedal controllers {64..69} are:
//     0 <= n <=  63 : Pedal off
//    64 <= n <= 127 : Pedal on
//  ie. bool pedalActive = !!(n & 0x40)
//
// ...I have grouped them logically, not numerically
//

// -- PatchBank selection
#define  CTRL_BANK_MSB            0                 // [GS][GM2] Bank Select MSB
#define  CTRL_BANK_LSB           32                 // [GS][GM2] Bank Select LSB

// -- Basic mixing - GM/GM2/GS/XG only specify Coarse/MSB tuning
#define  CTRL_BAL_MSB             8                 // Stereo Balance MSB
#define  CTRL_BAL_LSB            40                 // Stereo Balance LSB
#define  CTRL_BAL               CTRL_BAL_MSB        // Stereo Balance

#define  CTRL_PAN_MSB            10                 // Mono Position MSB
#define  CTRL_PAN_LSB            42                 // Mono Position LSB
#define  CTRL_PAN               CTRL_PAN_MSB        // [GM][GM2] Mono Position

#define  CTRL_VOLUME_MSB          7                 // Volume MSB
#define  CTRL_VOLUME_LSB         39                 // Volume LSB
#define  CTRL_VOL               CTRL_VOLUME_MSB     // [GM][GM2] Volume

#define  CTRL_EXPR_MSB           11                 // Expression (fade) MSB
#define  CTRL_EXPR_LSB           43                 // Expression (fade) LSB
#define  CTRL_EXPR              CTRL_EXPR_MSB       // [GM] Expression (fade)

// -- Extended effects
#define  CTRL_USRFX1_MSB         12                 // Effects Controller #1 MSB
#define  CTRL_USRFX1_LSB         44                 // Effects Controller #1 LSB

#define  CTRL_USRFX2_MSB         13                 // Effects Controller #2 MSB
#define  CTRL_USRFX2_LSB         45                 // Effects Controller #2 LSB

// - Control sliders
#define  CTRL_SLIDER1            16                 // Slider #1
#define  CTRL_SLIDER2            17                 // Slider #2
#define  CTRL_SLIDER3            18                 // Slider #3
#define  CTRL_SLIDER4            19                 // Slider #4

//- Control buttons
#define  CTRL_BUTTON1            80                 // Button #1
#define  CTRL_BUTTON2            81                 // Button #2
#define  CTRL_BUTTON3            82                 // Button #3
#define  CTRL_BUTTON4            83                 // Button #4

// -- Piano pedals : https://www.flowkey.com/en/piano-guide/piano-pedals
// -- 0 <= off < 64 <= on <= 127
#define  CTRL_PDL_DAMPER         64                 // [GM][GM2] Damper Pedal (aka. "sustain" or "hold-1")
#define  CTRL_PDL_PORT           65                 // [GS][GM2] Portamento Pedal
#define  CTRL_PDL_SUSTENUTO      66                 // [GS][GM2] Sustenuto Pedal
#define  CTRL_PDL_SOFT           67                 // [GS][GM2] Soft Pedal
#define  CTRL_PDL_LEGATO         68                 // Legato Pedal
#define  CTRL_PDL_HOLD           69                 // Hold Pedal (longer decay) (aka "hold-2")

// - Basic sound controllers - GM/GM2/GS/XG only specify Coarse/MSB tuning
#define  CTRL_MOD_MSB             1                 // Modulation wheel MSB
#define  CTRL_MOD_LSB            33                 // Modulation wheel LSB
#define  CTRL_MOD               CTRL_MOD            // [GM][GM2] Modulation wheel

#define  CTRL_BREATH_MSB          2                 // Breath Control MSB
#define  CTRL_BREATH_LSB         34                 // Breath Control LSB
#define  CTRL_BREATH            CTRL_BREATH_MSB     // [GM2] Breath Control

#define  CTRL_FOOT_MSB            4                 // Foot Control MSB
#define  CTRL_FOOT_LSB           36                 // Foot Control LSB
#define  CTRL_FOOT              CTRL_FOOT_MSB       // [GM2] Foot Control

// - Portamento controls
#define  CTRL_PORT_TIME         CTRL_PORT_TIME_MSB  // [GM2] Portamento Time
#define  CTRL_PORT_TIME_MSB       5                 // Portamento Time
#define  CTRL_PORT_TIME_LSB      37                 // Portamento Time
#define  CTRL_PORT_CTRL          84                 // [GS][XG] Portamento Control [trigger]

// - Basic Effects
#define  CTRL_FX1                91                 // FX-1 Send level
#define  CTRL_REVERB            CTRL_FX1            // FX-1 : Reverb

#define  CTRL_FX2                92                 // [GS] FX-2 Send level
#define  CTRL_TREMOLO           CTRL_FX2            // [GS] FX-2 : Termulo/tremolo
#define  CTRL_TREMULO           CTRL_FX2            // [GS] FX-2 : Termulo/tremolo

#define  CTRL_FX3                93                 // [GS] FX-2 Send level
#define  CTRL_CHORUS            CTRL_FX3            // [GS] FX-3 Send: Chorus

#define  CTRL_FX4                94                 //  FX-4 Send level
#define  CTRL_DELAY             CTRL_FX4            // [GS] FX-4 Send: Delay/Variation
#define  CTRL_DETUNE            CTRL_FX4            // [GM] FX-4 Send: Detune
#define  CTRL_CELESTE           CTRL_FX4            // FX-4 Send: Celeste

#define  CTRL_FX5                95                 //  FX-5 Send level
#define  CTRL_PHASER            CTRL_FX5            //  [GM] FX-5 Send level

// - Sound Controllers
#define  CTRL_SOUND1             70                 // Sound controller #1
#define  CTRL_SOUND2             71                 // Sound controller #2
#define  CTRL_SOUND3             72                 // Sound controller #3
#define  CTRL_SOUND4             73                 // Sound controller #4
#define  CTRL_SOUND5             74                 // Sound controller #5
#define  CTRL_SOUND6             75                 // Sound controller #6
#define  CTRL_SOUND7             76                 // Sound controller #7
#define  CTRL_SOUND8             77                 // Sound controller #8
#define  CTRL_SOUND9             78                 // Sound controller #9
#define  CTRL_SOUND10            79                 // Sound controller #10

// - GM2
#define  CTRL_GM_VARIATION      CTRL_SOUND1         // [GM2] Sound variaion

#define  CTRL_TIMBRE            CTRL_SOUND2         // [GM2] Timbre (or FILTER_CUTOFF)
#define  CTRL_FILTER_RESONANCE  CTRL_SOUND2         // [GM2] Filter Resonance

#define  CTRL_BRIGHTNESS        CTRL_SOUND5         // [GM2] BRIGHTNESS (or FILTER_RESONANCE)
#define  CTRL_FILTER_CUTOFF     CTRL_SOUND5         // [GM2] Filter Cutoff (Timbre)

#define  CTRL_VIBRATO_DEPTH     CTRL_SOUND8         // [GM2] Vibrato Depth
#define  CTRL_VIBRATO_RATE      CTRL_SOUND7         // [GM2] Vibrato Rate
#define  CTRL_VIBRATO_DELAY     CTRL_SOUND9         // [GM2] Vibrato Delay

#define  CTRL_ATTACK            CTRL_SOUND4         // [GM2] Attack Time
#define  CTRL_DECAY             CTRL_SOUND6         // [GM2] Decay Time   - err, where is "Sustain"?
#define  CTRL_RELEASE           CTRL_SOUND3         // [GM2] Release Time

// - (N)RPN controls
#define  CTRL_RPN_LSB           100                 // [GS][GM][GM2] RPN LSB
#define  CTRL_RPN_MSB           101                 // [GS][GM][GM2] RPN MSB

#define  CTRL_NRPN_LSB           98                 // [GS] NRPN LSB [0lll'llll]
#define  CTRL_NRPN_MSB           99                 // [GS] NRPN MSB [0mmm'mmmm]

#define  CTRL_DATA_MSB            6                 // [GM2] Data Entry MSB
#define  CTRL_DATA_LSB           38                 // [GM2] Data Entry LSB

#define  CTRL_DATA_INC           96                 // Data Increment (+1) [trigger]
#define  CTRL_DATA_DEC           97                 // Data Decrement (-1) [trigger]

// - System Controllers
#define  CTRL_ALL_SOUND_OFF      120                // http://midi.teragonaudio.com/tech/midispec/sndoff.htm
#define  CTRL_ALL_CTRL_OFF       121                // http://midi.teragonaudio.com/tech/midispec/ctloff.htm
#define  CTRL_LOCAL              122                // http://midi.teragonaudio.com/tech/midispec/local.htm
#define  CTRL_ALL_NOTES_OFF      123                // http://midi.teragonaudio.com/tech/midispec/ntnoff.htm
#define  CTRL_OMNI_OFF           124                // http://midi.teragonaudio.com/tech/midispec/modes.htm
#define  CTRL_OMNI_ON            125                // http://midi.teragonaudio.com/tech/midispec/modes.htm
#define  CTRL_MONO               126                // http://midi.teragonaudio.com/tech/midispec/mono.htm
#define  CTRL_POLY               127                // http://midi.teragonaudio.com/tech/midispec/poly.htm

//----------------------------------------------------------------------------- ----------------------------------------
#include <stdint.h>
#include <stdbool.h>

//----------------------------------------------------------------------------- ----------------------------------------
// MThd header
//
typedef
	struct PACK {
		char      magic[4];                  // "MThd"
		uint32_t  len;                       // 6

		uint16_t  fmt;                       // file format {0,1,2}
		uint16_t  trkCnt;                    // number of tracks

		union PACK {
			union PACK {
				struct PACK {
					uint16_t  tpqn    : 15;  // ticks Per Quarter Note
					uint16_t  divtype : 1;   // 0=TPQN, 1=SMPTE
				};
				struct PACK {
					uint16_t  tpf     : 8;   // SMPTE: ticks per frame
					uint16_t  fps     : 7;   // SMPTE: frames/sec. Generally {24,25,29,30}
					uint16_t          : 1;   // (divtype)
				};
			};
			uint16_t  div;                   // the 16bit div value
		};
	}
mthd_t;

// div values
#define  DIV_TPQN   (0)
#define  DIV_SMPTE  (1)

//----------------------------------------------------------------------------- ----------------------------------------
#define  BPM_DEFAULT  (120)  // uspdt = ((60s * 1000000uS) / bpm) / mthd.tpqn;

//----------------------------------------------------------------------------- ----------------------------------------
typedef  uint32_t   delta_t;               // delta time (relative or absolute)
#define  DELTA_MAX  ((delta_t)UINT32_MAX)  // max delta time

//----------------------------------------------------------------------------- ----------------------------------------
// IFF Chunk header
//
// NB. IFF/MIDI files are Big-Endian - aka "network endian"
//     ...so we can use ntohs/ntohl (network-to-host short==16bits|long==32bits) and htons/htonl
//      ...these are MACROS so, if not required, they will optimise out - at compile time
//
typedef
	struct PACK {
		char      magic[4];
		uint32_t  len;
		uint8_t   data[];
	}
iffh_t;

// sizeof header
#include <stddef.h>
#define IFFH_SZ  (offsetof(iffh_t, data))

//----------------------------------------------------------------------------- ----------------------------------------
// Tempos are stored as 24-bit Big Endian values
//
#define  PEEK24BE(p)  ((((uint32_t)((uint8_t*)(p))[0]) << 16) | (((uint8_t*)(p))[1] << 8) | ((uint8_t*)(p))[2])

//----------------------------------------------------------------------------- ----------------------------------------
// Read a MIDI 14bit value 0lllllll'0mmmmmmm --> 00mmmmmm'mlllllll
//
#define  PEEK14LE(p)  ((uint16_t)(((uint8_t*)(p))[0]) | ((((uint8_t*)(p))[1]) << 7))

//----------------------------------------------------------------------------- ----------------------------------------
// MIDI "Status" values
//
typedef
	enum {
		MSTAT_SYX   = 0xF0,  // SysEx Start
		MSTAT_F1    = 0xF1,  // undefined
		MSTAT_SPP   = 0xF2,  // Song Position Pointer
		MSTAT_SS    = 0xF3,  // Song Select
		MSTAT_F4    = 0xF4,  // undefined
		MSTAT_F5    = 0xF5,  // undefined
		MSTAT_TUNE  = 0xF6,  // Tine Request
		MSTAT_ESC   = 0xF7,  // SysEx continue
		MSTAT_TC    = 0xF8,  // Timing Clock
		MSTAT_TICK  = 0xF9,  // (undefined) / Tick
		MSTAT_START = 0xFA,  // Start
		MSTAT_CONT  = 0xFB,  // Cont
		MSTAT_STOP  = 0xFC,  // Stop
		MSTAT_FD    = 0xFD,  // undefined
		MSTAT_AS    = 0xFE,  // Active Sensing
		MSTAT_META  = 0xFF,  // Meta Event
	}
mstatus_t;

//----------------------------------------------------------------------------- ----------------------------------------
// (Known) Meta Events
//
// Source: 010 Editor template by Jack Andersen
//
typedef
	enum {
		META_SEQUENCE_NUM        = 0x00,
		META_TEXT                = 0x01,
		META_COPYRIGHT           = 0x02,
		META_SEQUENCE_NAME       = 0x03,
		META_INSTRUMENT_NAME     = 0x04,
		META_LYRIC               = 0x05,
		META_MARKER              = 0x06,
		META_CUE_POINT           = 0x07,
		META_PROGRAM_NAME        = 0x08,
		META_DEVICE_NAME         = 0x09,
		META_MIDI_CHANNEL_PREFIX = 0x20,
		META_MIDI_PORT           = 0x21,
		META_END_OF_TRACK        = 0x2F,
		META_TEMPO               = 0x51,
		META_SMPTE_OFFSET        = 0x54,
		META_TIME_SIGNATURE      = 0x58,
		META_KEY_SIGNATURE       = 0x59,
		META_SEQUENCER_EVENT     = 0x7F,
	}
mmeta_t;

//----------------------------------------------------------------------------- ----------------------------------------
// MIDI Commands
//
enum {
	MCMD_OFF     = 0x80,  // [2] Note Off
	MCMD_ON      = 0x90,  // [2] Note On
	MCMD_KEYAFT  = 0xA0,  // [2] Key Aftertouch
	MCMD_CC      = 0xB0,  // [2] Controller Change
	MCMD_PGM     = 0xC0,  // [3] Patch/Program Change
	MCMD_CHNAFT  = 0xD0,  // [3] Channel Aftertouch
	MCMD_WHEEL   = 0xE0,  // [2] Pitch Wheel
};

//----------------------------------------------------------------------------- ----------------------------------------
// MIDI event information
//
// mid_getEvent(uint8_t* ep) will parse an event in to this structure
//
typedef
	struct {
		delta_t    delta;    // delta offset (as an integer)
		uint32_t   len;      // length of payload

		mstatus_t  status;   // status byte (event type)
		uint8_t*   data;     // (pointer to) data
		uint32_t   dLen;     // length of data

		struct {             // meta information:
			mmeta_t   type;  //   meta event type
			uint8_t*  data;  //   (pointer to) meta data
			uint32_t  dLen;  //   length of meta data
		}         meta;
	}
mevent_t;

//----------------------------------------------------------------------------- ----------------------------------------
// MIDI Track information
//
typedef
	struct {
		iffh_t*   hdr;      // pointer to track header
		uint8_t*  data;     // pointer to track data
		uint8_t*  eot;      // address of 1st byte after the track (in memory)

		bool      mute;     // Disable Note-On events

		delta_t   dNext;    // Next track delta
		uint8_t*  eNext;    // Next track event (pointer to)
		uint8_t   runStat;  // "running status" byte

		bool      trans;    // Allow transposition (eg. don't transpose drums)
	}
mtrack_t;

//----------------------------------------------------------------------------- ----------------------------------------
// MIDI File information
//
typedef
	struct {
		char*      fn;       // file name
		uint8_t*   fBuf;     // file buffer
		int        fSiz;     // file size
		uint8_t*   fBof;     // address of 1st byte of MThd header
		uint8_t*   fEof;     // address of 1st byte after the file (in memory)

		uint32_t   syxCnt;   // file is n SysEx commands

		mthd_t*    mthd;     // (pointer to) header chunk
		mtrack_t*  mtrk;     // array of tracks  {0 .. mthd->trkCnt - 1}

		int        cCnt;     // number of unknown chunks
		iffh_t**   chunk;    // array of (pointers to) unknown chunks {0 .. cCnt - 1}

		bool       running;  // 0=stopped, 1=running
		bool       paused;   // if (running) 0=paused, 1=playing

		uint32_t   uspdt;    // uS per Delta tick (1s == 1,000,000uS)
		uint16_t   speed;    // playback speed (percentage)
		
		volatile  delta_t  delta;  // running delta **VOLATILE**
	}
smf_t;

//----------------------------------------------------------------------------- ----------------------------------------
// MIDI out port
//
//! device specific code
typedef
	struct {
		char   name[32];
		FILE*  fh;
	}
mport_t;

//----------------------------------------------------------------------------- ----------------------------------------
// MIDI player information
//
typedef
	struct mplayer {
		bool(*cbPlay)(struct mplayer*, smf_t*);  // Playback callback

		mport_t   portOut;  // MIDI outport

		bool      run;      // player is: 1=running 0=stopped
		bool      pause;    // if running,  player is 1=paused  0=running

		int8_t    trans;    // transpose note events (on transpose-enabled tracks)

		smf_t*    smf;      // array of Simple Midi Files
		uint8_t   smfMax;   // size of array
	}
mplayer_t;

extern  mplayer_t  _p;

//----------------------------------------------------------------------------- ----------------------------------------
// ANSI boxes
//
typedef
	struct {
		int  y;  // top left
		int  x;
		int  w;  // overall 
		int  h;
	}
box_t;

//----------------------------------------------------------------------------- ----------------------------------------
// ANSI keyboards
//
typedef
	struct {
		int    chan;
		int    live;
		int    mute;
		int    o1;
		int    oN;
		box_t  box;
	}
kbd_t;

kbd_t  _kbd[16];

//----------------------------------------------------------------------------- ----------------------------------------
uint8_t   getVlq        (uint32_t* pN,  uint8_t* pVlq) ;
uint32_t  getEvent      (smf_t* pSmf,  mtrack_t* trk,  mevent_t* ep,  uint8_t* bp) ;

bool      midpInit      (int slots) ;
bool      midpKill      (void) ;
bool      midpPlay      (uint8_t n,  char* port) ;
bool      midpPlay1     (smf_t* pSmf) ;
void      midpPanic     (smf_t* pSmf) ;
void      midpSpeed     (smf_t* pSmf,  uint16_t speed) ;

bool      fileLoad      (int n,  char* fn) ;
bool      fileCheckSmf  (smf_t* pSmf) ;
bool      fileCheckSyx  (smf_t* pSmf) ;
bool      fileCheckRiff (uint8_t* bp) ;
bool      fileParseSmf  (smf_t* pSmf) ;
bool      fileUnload    (int n) ;
char*     fileFoffs     (smf_t* pSmf,  void* vp) ;

bool      fileDump      (uint8_t n) ;
void      fileDumpTrack (smf_t* pSmf,  uint8_t t) ;
void      fileDumpChunk (smf_t* pSmf,  uint8_t c) ;
void      hexDump       (const void* const buf,  const uint32_t len,  const uint32_t offs) ;

void      showEvent     (smf_t* pSmf,  mevent_t* ep,  uint8_t trk) ;
void      showMeta      (smf_t* pSmf,  mevent_t* ep,  uint8_t trk) ;
void      showMidi      (mevent_t* ep) ;
void      showKeySig    (uint8_t* bp) ;
char*     patch2nameGM  (uint8_t val) ;
char*     ctrl2name     (uint8_t val) ;
char*     note2name     (char* name,  uint8_t val) ;

bool      drawInit      (void) ;
bool      drawKill      (void) ;
void  drawSel (kbd_t* old,  kbd_t* new) ;
void  drawEos (void) ;
void      drawFilename  (smf_t* pSmf) ;
void      drawKbd       (kbd_t* kp) ;
void      drawPgm       (mevent_t* ep) ;
void      drawTempo     (smf_t* pSmf) ;
void      drawSpeed     (smf_t* pSmf) ;
void      drawKeySig    (mevent_t* ep) ;
void      drawTimeSig   (mevent_t* ep) ;
void      drawNote      (mevent_t* ep) ;

bool      mdevPortOpen  (char* name) ;
bool      mdevPortClose (void) ;
bool      mdevSendEvent (smf_t* pSmf,  mevent_t* ep) ;
bool      mdevSendSyx   (uint8_t* bp,  uint32_t len) ;
bool      mdevSendEsc   (uint8_t* bp,  uint32_t len) ;

bool      intrInit      (void) ;
bool      intrKill      (void) ;
bool      intrStart     (smf_t* pSmf) ;
bool      intrPeriod    (smf_t* pSmf) ;
bool      intrPause     (smf_t* pSmf) ;
bool      intrCont      (smf_t* pSmf) ;
bool      intrStop      (smf_t* pSmf) ;

bool      cbInit        (void) ;
bool      cbKill        (void) ;
bool      cb_play       (mplayer_t* pPLy,  smf_t* pSmf) ;

#endif // PLAY_MIDI_H_
