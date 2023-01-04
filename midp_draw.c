#include <stdbool.h>
#include <string.h>

#include "midp_play.h"

//----------------------------------------------------------------------------- ----------------------------------------
#define  GOYX          "\e[%d;%dH"

#define  CLS_SOL       "\e[1K"          // clear to start of line
#define  CLS           "\e[2J\e[1;1H"   // cls() and goyx(1,1)

#define  ACLRN         "\e[0m"
#define  ACLR1(clr)    "\e["clr"m"
#define  ACLR2(fg,bg)  "\e["fg";"bg"m"

#define   fgBlk  "0;30"
#define   fgGry  "1;30"
#define   fgRed  "0;31"
#define  fgBRed  "1;31"
#define   fgGrn  "0;32"
#define  fgBGrn  "1;32"
#define   fgBrn  "0;33"
#define   fgYel  "1;33"
#define   fgBlu  "0;34"
#define  fgBBlu  "1;34"
#define   fgMag  "0;35"
#define  fgBMag  "1;35"
#define   fgCyn  "0;36"
#define  fgBCyn  "1;36"
#define   fgWht  "0;37"
#define  fgBWht  "1;37"

#define  bgBlk   "40"
#define  bgRed   "41"
#define  bgGrn   "42"
#define  bgYel   "43"
#define  bgBlu   "44"
#define  bgMag   "45"
#define  bgCyn   "46"
#define  bgWht   "47"
#define  bgDef   "49"  // default colour

// ▌█▐
//    │
//┕ ─ ┴ ┘");

//----------------------------------------------------------------------------- ----------------------------------------
const char* const  ansi_curOff   = "\e[?25l";
const char* const  ansi_curOn    = "\e[?25h";
const char* const  ansi_cls      = "\033[2J";

const char* const  clr_norm      =   ACLR1("0");
const char* const  clr_none      =   ACLR2(fgBlk , bgBlk );

const char* const  clr_kbLive[2] = { ACLR2(fgWht , bgBlk ), ACLR2(fgGry , bgBlk ) };
const char* const  clr_kbWait[2] = { ACLR2(fgBlu , bgBlk ), ACLR2(fgBlu , bgBlk ) };
const char* const  clr_kbMute[2] = { ACLR2(fgBRed, bgBlk ), ACLR2(fgRed , bgBlk ) };
//const char* const  clr_kbSel [2] = { ACLR2(fgBCyn, bgBlk ), ACLR2(fgCyn , bgBlk ) };

const char* const  clr_tempo     =   ACLR2(fgBWht, bgBlu );
const char* const  clr_keySig    =   ACLR2(fgBWht, bgBlu );
const char* const  clr_timeSig   =   ACLR2(fgBWht, bgBlu );
const char* const  clr_fname     =   ACLR2(fgBWht, bgBlu );

const char* const  clr_sel   [2] = { ACLR2(fgWht , bgBlk ), ACLR2(fgBWht, bgBlk ) };
const char* const  clr_chan  [2] = { ACLR2(fgBlk , bgGrn ), ACLR2(fgBlk , bgGrn ) };
const char* const  clr_pgm   [2] = { ACLR2(fgYel , bgBlu ), ACLR2(fgBrn , bgBlu ) };

const char* const  clr_ebOn  [2] = { ACLR2(fgYel , bgBlk ), ACLR2(fgYel , bgBlk ) };
const char* const  clr_ivOn  [2] = { ACLR2(fgGry , bgYel ), ACLR2(fgGry , bgYel ) };
const char* const  clr_keyOff[2] = { clr_kbLive[0]        , clr_kbLive[1]         };
const char* const  clr_ebOff [2] = { clr_keyOff[0]        , clr_keyOff[1]         };
const char* const  clr_ivOff [2] = { clr_keyOff[0]        , clr_keyOff[1]         };

box_t  box_fname   = {.y =  1,  .x = 87,  .w = 22,  .h = 1 } ;
box_t  box_tempo   = {.y =  1,  .x =  1,  .w = 28,  .h = 1 } ;
box_t  box_pgm     = {.y =  1,  .x =  0,  .w = 15,  .h = 1 } ;
box_t  box_kbd     = {.y =  0,  .x = 17,  .w =  0,  .h = 2 } ;
box_t  box_chan    = {.y =  1,  .x =  0,  .w =  2,  .h = 1 } ;
box_t  box_speed   = {.y =  2,  .x =  1,  .w = 19,  .h = 1 } ;
box_t  box_timeSig = {.y =  1,  .x = 47,  .w = 27,  .h = 1 } ;
box_t  box_keySig  = {.y =  1,  .x = 31,  .w = 15,  .h = 1 } ;

//+============================================================================ ========================================
void  _drawKill (void)  {  (void)drawKill();  }

bool  drawInit (void)
{
	int  o1 = 1;
	int  oN = 8;

	DRAW("%s%s", ansi_cls, ansi_curOff);
	atexit(_drawKill);

	for (int c = 0;  c < 16;  c++) {
		_kbd[c] = (kbd_t){
			.chan = c,
			.live = 1,
			.mute = 0,
			.o1   = o1,
			.oN   = oN,
			.box  = {
				.x = box_kbd.x,
				.y = (c * 3) + 4,
				.w = ((oN - o1 + 1) * (7 * 2)) + 1,
				.h = box_kbd.h
			}
		};
		drawKbd(&_kbd[c]);
	}

	box_fname   = (box_t){.y =  1,  .x = 1                                ,  .w = 22,  .h = 1 } ;
	box_timeSig = (box_t){.y =  1,  .x = box_fname.x   + box_fname.w   + 2,  .w = 37,  .h = 1 } ;
	box_keySig  = (box_t){.y =  1,  .x = box_timeSig.x + box_timeSig.w + 2,  .w = 15,  .h = 1 } ;

	box_tempo   = (box_t){.y =  2,  .x =  1                               ,  .w = 37,  .h = 1 } ;
	box_speed   = (box_t){.y =  2,  .x = box_tempo.x   + box_tempo.w   + 2,  .w = 19,  .h = 1 } ;

	box_chan    = (box_t){.y =  1,  .x =  0,  .w =  2,  .h = 1 } ;
	box_pgm     = (box_t){.y =  1,  .x =  0,  .w = 15,  .h = 1 } ;
	box_kbd     = (box_t){.y =  0,  .x = 17,  .w =  0,  .h = 2 } ;

	return true;
}

//+============================================================================ ========================================
bool  drawKill (void)
{
	DRAW(ansi_curOn);
}

//+============================================================================ ========================================
void  drawKbd (kbd_t* kp)
{
	int  i;

	DRAW(GOYX"%s%2d", kp->box.y,1, clr_chan[kp->chan & 1], kp->chan + 1);

	if      (kp->mute)  DRAW(clr_kbMute[kp->chan & 1]);
	else if (kp->live)  DRAW(clr_kbLive[kp->chan & 1]);
	else                DRAW(clr_kbWait[kp->chan & 1]);

	DRAW(GOYX"│", kp->box.y  , kp->box.x);  for (i = kp->o1;  i <= kp->oN;  i++)  DRAW("%d█ █ │ █ █ █ │", i) ;
	DRAW(GOYX"│", kp->box.y+1, kp->box.x);  for (i = kp->o1;  i <= kp->oN;  i++)  DRAW( "c│d│e│f│g│a│b│"   ) ;
	DRAW(GOYX"┕", kp->box.y+2, kp->box.x);  for (i = kp->o1;  i <= kp->oN;  i++)  DRAW( "─┴─┴─┴─┴─┴─┴─┴"   ) ;
	DRAW("\b┘");
}

//+============================================================================ ========================================
void  drawPgm (mevent_t* ep)
{
	box_t*  box = &box_pgm;

	char*   name = patch2nameGM(ep->data[0]);
	int     len  = strlen(name);
	int     chan = ep->status & 0x0F;
	int     y    = _kbd[chan].box.y + 1;

	if (len > box->w)  len = box->w ;

	DRAW(GOYX "%s" CLS_SOL GOYX "%.*s", y,(box->x + box->w), clr_pgm[chan & 1], y,box->x, len,name);
}

//+============================================================================ ========================================
void  drawSel (kbd_t* old,  kbd_t* new)
{
	DRAW(GOYX "%s \n\b ", old->box.y, old->box.x - 1, clr_none);
	DRAW(GOYX "%s§\n\b§", new->box.y, new->box.x - 1, clr_sel[new->chan & 1]);
	DRAW(GOYX "%s \n\b ", old->box.y, old->box.x + old->box.w, clr_none);
	DRAW(GOYX "%s§\n\b§", new->box.y, new->box.x + new->box.w, clr_sel[new->chan & 1]);
}

//+============================================================================ ========================================
void  drawFilename (smf_t* pSmf)
{
	box_t*  box = &box_fname;
	char*   cp  = strrchr(pSmf->fn, '/');

	DRAW(GOYX"%s %.*s ", box->y,box->x, clr_fname, (box->w - 2), cp ? (cp + 1) : pSmf->fn);
	for (int i = strlen(pSmf->fn);  i <= box->w;  i++)  DRAW(" ") ;
}

//+============================================================================ ========================================
void  drawTempo (smf_t* pSmf)
{
	box_t*  box = &box_tempo;

	DRAW( GOYX"%s %6.2fBPM (%5d uS/dt, %4d dt/qn) ", box->y,box->x, clr_tempo,
	      ((60 * 1000000.0) / (pSmf->uspdt * pSmf->mthd->tpqn)), pSmf->uspdt, pSmf->mthd->tpqn );
}

//+============================================================================ ========================================
void  drawSpeed (smf_t* pSmf)
{
	box_t*  box = &box_speed;

	DRAW( GOYX"%s %3d%% -> %6.2fBPM (%4d dt/qn) ", box->y,box->x, clr_tempo, pSmf->speed,
	      (((60 * 1000000.0) / (pSmf->uspdt * pSmf->mthd->tpqn)) * pSmf->speed) / 100,
	      (pSmf->uspdt * 100) / pSmf->speed );
}

//+============================================================================ ========================================
void  drawKeySig (mevent_t* ep)
{
	box_t*  box = &box_keySig;
	int8_t  sf  = (int8_t)ep->meta.data[0];

	DRAW(GOYX"%s Key: ", box->y,box->x, clr_keySig);
	if (ep->meta.data[1]) {
		if      (sf < 0)  DRAW("%c%sm (%d♭) ", "DGCFBEA"[-sf], ((sf >= -4) ? "" : "b"), sf);
		else if (sf > 0)  DRAW("%c%sm (%d♯) ", "EBFCGDA"[sf] , ((sf <=  2) ? "" : "#"), sf);
		else              DRAW("Am ");
	} else {
		if      (sf < 0)  DRAW("%c%s (%d♭) ", "FBEADGC"[-sf], ((sf == -1) ? "" : "b"), sf);
		else if (sf > 0)  DRAW("%c%s (%d♯) ", "GDAEBFC"[sf] , ((sf <=  5) ? "" : "#"), sf);
		else              DRAW("C ");
	}
}

//+============================================================================ ========================================
void  drawTimeSig (mevent_t* ep)
{
	box_t*  box = &box_timeSig;
	DRAW(GOYX"%s Time: ", box->y,box->x, clr_timeSig);
	DRAW( "%d/%d (%d clok/clik, %d 32nd/qn) ",
	      ep->meta.data[0], (1 << ep->meta.data[1]), ep->meta.data[2], ep->meta.data[3] );
}

//+============================================================================ ========================================
void  drawEos (void)
{
	DRAW(GOYX ACLRN, 52,1);
}

//+============================================================================ ========================================
void  drawNote (mevent_t* ep)
{
	enum { KEY_NONE, KEY_ON, KEY_OFF }  key = KEY_NONE ;

	switch (ep->status & 0xF0) {
		case MCMD_ON  :  key = ep->data[1] ? KEY_ON : KEY_OFF ;  break ;
		case MCMD_OFF :  key = KEY_OFF;                          break ;
	}

	if (key) {
		kbd_t*  kp   = &_kbd[ep->status & 0x0F];
		int     cs   = kp->chan & 1;                 // colour scheme
		int     oct  = (ep->data[0] / 12) - 1;
		int     note =  ep->data[0] % 12;            // 0 => C

		if ((oct >= kp->o1) && (oct <= kp->oN)) {    // is this key visible
			const char*  clr;
			char         chr[8] = {0,0};
			bool         sharp;

			oct -= kp->o1;                           // relative octave

			if (note > 4)  note++ ;                  // insert/skip e#

			sharp = (bool)(note & 1);                // ebony notes

			if (sharp)  strcpy(chr, "█") ;           // Note icon
			else        *chr = "cdefgab"[note>>1] ;

			if (key == KEY_ON)  clr = sharp    ? clr_ebOn[cs]   : clr_ivOn[cs] ;
			else                clr = kp->mute ? clr_kbMute[cs] : clr_keyOff[cs] ;

			DRAW(GOYX"%s%s", (kp->box.y + !sharp), (kp->box.x + 1 + note + (oct * (12 + 2))), clr, chr);
		}
	}
}
