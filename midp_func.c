#include <stdbool.h>

#include "midp_play.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// https://stackoverflow.com/a/448982/2476535

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>

//----------------------------------------------------------------------------- 
static  struct termios  kbdOrig;

//+============================================================================
static
void  kbdReset (void)
{
	tcsetattr(0, TCSANOW, &kbdOrig);
}

//+============================================================================
static
void  kbdConIO (void)
{
	struct termios  kbdNew;

	// Backup the current settings
	tcgetattr(0, &kbdOrig);
	memcpy(&kbdNew, &kbdOrig, sizeof(kbdNew));

	// Make sure we clean up when we leave
	atexit(kbdReset);

	// Set to single character mode
	cfmakeraw(&kbdNew);
	tcsetattr(0, TCSANOW, &kbdNew);
}

//+============================================================================
static
bool  kbhit (void)
{
	struct timeval  tv  = { 0L, 0L };
	fd_set          fds;

	FD_ZERO(&fds);
	FD_SET(0, &fds);

	return (select(1, &fds, NULL, NULL, &tv) > 0);
}

//+============================================================================
static
int  getch (void)
{
    int            r;
    unsigned char  c;

    return ((r = read(0, &c, sizeof(c))) < 0) ?  r : c ;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//----------------------------------------------------------------------------- 
#include <sys/time.h>

typedef
	enum {
		KEY_MAX   = 5,       // longest 'scancode'
		KEY_TIMER = 200000,  // multikey timeout - 1000uS = 1mS

		KEY_NONE  = 0xFF00,
		KEY_WAIT  = 0xFF01,
		KEY_TRUNC = 0xFF02,
		KEY_UNK   = 0xFF99,
		KEY_ERR   = 0xFFFF,

		KEY_GBP   = 0x0C00,

		KEY_LEFT  = 0x0108,
		KEY_DOWN  = 0x0104,
		KEY_UP    = 0x0102,
		KEY_RIGHT = 0x0101,

		KEY_HOME  = 0x0118,
		KEY_PGDN  = 0x0114,
		KEY_PGUP  = 0x0112,
		KEY_END   = 0x0111,

		KEY_INS   = 0x0121,
		KEY_DEL   = 0x0122,

		KEY_F1    = 0x0200 |  1,
		KEY_F2    = 0x0200 |  2,
		KEY_F3    = 0x0200 |  3,
		KEY_F4    = 0x0200 |  4,
		KEY_F5    = 0x0200 |  5,
		KEY_F6    = 0x0200 |  6,
		KEY_F7    = 0x0200 |  7,
		KEY_F8    = 0x0200 |  8,
		KEY_F9    = 0x0200 |  9,
		KEY_F10   = 0x0200 | 10,
		KEY_F11   = 0x0200 | 11,
		KEY_F12   = 0x0200 | 12,
		KEY_F13   = 0x0200 | 13,
		KEY_F14   = 0x0200 | 14,
		KEY_F15   = 0x0200 | 15,
		KEY_F16   = 0x0200 | 16,
		KEY_F17   = 0x0200 | 17,
		KEY_F18   = 0x0200 | 18,
		KEY_F19   = 0x0200 | 19,
		KEY_F20   = 0x0200 | 20,
	}
keycap_t;

//+============================================================================
static
int  getchw (void)
{
	static  int              buf[KEY_MAX];
	static  int*             ip = buf;
	static  struct  timeval  st;
	        struct  timeval  nd;

	if (!kbhit()) {
		// empty buffer?
		if (ip == buf)  return KEY_NONE ;

		// timeout?
		gettimeofday(&nd, NULL);  // check timer
		if (((nd.tv_sec * 1000000) + nd.tv_usec) - 
		    ((st.tv_sec * 1000000) + st.tv_usec) >= KEY_TIMER) {
			if (ip == buf + 1)  return (ip = buf),  *buf ;       // 1 chr waiting
			else                return (ip = buf),  KEY_TRUNC ;  // >1  char waiting
		}

		// wait some more
		return KEY_WAIT;

	// keystroke available
	} else {
		int  chr;

		if ((chr = getch()) < 0)  return (ip = buf),  chr ;
		else                      *ip = chr ;

		gettimeofday(&st, NULL);

		switch (ip - buf) {
			case 0 :  // 1st character
				if      (*buf == '\e')  return ip++,  KEY_WAIT ;  // esc
				else if (*buf <= 0x7F)  return *buf;              // ASCII
				else if (*buf == 0xC2)  return ip++,  KEY_WAIT ;  // £ [UK shift+3] == 0xC2, 0xA3
				else                    return KEY_UNK;

			case 1 :  // 2nd character
				// £ [UK shift+3] == 0xC2, 0xA3
				if (ip[-1] == 0xC2) {
					if (*ip == 0xA3)  return (ip = buf),  KEY_GBP ;
					else              return (ip = buf),  KEY_UNK ;
				}

				if ((ip[-1] == '\e') && ((*ip == '\e') || (*ip != '[' )))
					return (ip = buf),  chr ;
				else
					return ip++,  KEY_WAIT ;

			// from here we have \e[***
			case 2 :  // 3rd character
				switch (*ip) {
					case 'A' :  return (ip = buf),  KEY_UP ;
					case 'B' :  return (ip = buf),  KEY_DOWN ;
					case 'C' :  return (ip = buf),  KEY_RIGHT ;
					case 'D' :  return (ip = buf),  KEY_LEFT ;
					case 'H' :  return (ip = buf),  KEY_HOME ;   // Kali
					case 'F' :  return (ip = buf),  KEY_END ;    // Kali
					case 'O' :  //...(capital oh)                // Kali
					case '1' :  //...
					case '2' :  //...
					case '3' :  //...
					case '4' :  //...
					case '5' :  //...
					case '6' :  return  ip++,  KEY_WAIT ;
					default:    return (ip = buf),  KEY_UNK ;
				}

			case 3 :  // 4th character
				if (*ip == '~')  switch (ip[-1]) {  // (*, tilde)
					case '1' :  return (ip = buf),  KEY_HOME ;
					case '2' :  return (ip = buf),  KEY_INS ;
					case '3' :  return (ip = buf),  KEY_DEL ;
					case '4' :  return (ip = buf),  KEY_END ;
					case '5' :  return (ip = buf),  KEY_PGUP ;
					case '6' :  return (ip = buf),  KEY_PGDN ;
					default:    return (ip = buf),  KEY_UNK ;
				}

				if (ip[-1] == 'O')  switch (*ip) {  // (capital oh, *)
					case 'P' :  return (ip = buf),  KEY_F1 ;  // Kali
					case 'Q' :  return (ip = buf),  KEY_F2 ;  // Kali
					case 'R' :  return (ip = buf),  KEY_F3 ;  // Kali
					case 'S' :  return (ip = buf),  KEY_F4 ;  // Kali
					default:    return (ip = buf),  KEY_UNK ;
				}

				switch (ip[-1]) {
					case '1' :
						if ((*ip <= '0') || (*ip >  '9') || (*ip == '6'))
							return (ip = buf),  KEY_UNK ;
						else
							return ip++,  KEY_WAIT ;
					case '2' :
						if ((*ip <  '0') || (*ip >  '9') || (*ip == '2') || (*ip == '7'))
							return (ip = buf),  KEY_UNK ;
						else
							return ip++,  KEY_WAIT ;
					case '3' :
						if ((*ip <= '0') || (*ip >  '4'))
							return (ip = buf),  KEY_UNK ;
						else
							return ip++,  KEY_WAIT ;
					default :
						return (ip = buf),  KEY_UNK ;
				}

			case 4 :  // 5th character
				if (*ip != '~')  return (ip = buf),  KEY_UNK ;
				switch ((ip[-2] << 8) | ip[-1]) {
					case (('1' << 8) | '1') :  return (ip = buf),  KEY_F1 ;
					case (('1' << 8) | '2') :  return (ip = buf),  KEY_F2 ;
					case (('1' << 8) | '3') :  return (ip = buf),  KEY_F3 ;
					case (('1' << 8) | '4') :  return (ip = buf),  KEY_F4 ;
					case (('1' << 8) | '5') :  return (ip = buf),  KEY_F5 ;
					case (('1' << 8) | '7') :  return (ip = buf),  KEY_F6 ;
					case (('1' << 8) | '8') :  return (ip = buf),  KEY_F7 ;
					case (('1' << 8) | '9') :  return (ip = buf),  KEY_F8 ;
					case (('2' << 8) | '0') :  return (ip = buf),  KEY_F9 ;
					case (('2' << 8) | '1') :  return (ip = buf),  KEY_F10 ;
					case (('2' << 8) | '3') :  return (ip = buf),  KEY_F11 ;
					case (('2' << 8) | '4') :  return (ip = buf),  KEY_F12 ;
					case (('2' << 8) | '5') :  return (ip = buf),  KEY_F13 ;
					case (('2' << 8) | '6') :  return (ip = buf),  KEY_F14 ;
					case (('2' << 8) | '8') :  return (ip = buf),  KEY_F15 ;
					case (('2' << 8) | '9') :  return (ip = buf),  KEY_F16 ;
					case (('3' << 8) | '1') :  return (ip = buf),  KEY_F17 ;
					case (('3' << 8) | '2') :  return (ip = buf),  KEY_F18 ;
					case (('3' << 8) | '3') :  return (ip = buf),  KEY_F19 ;
					case (('3' << 8) | '4') :  return (ip = buf),  KEY_F20 ;
					default : return (ip = buf),  KEY_UNK ;
				}

			default :
				return (ip = buf),  KEY_UNK ;
		}
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//+============================================================================ ========================================
void  _cbKill (void)  {  (void)cbKill();  }

bool  cbInit (void)
{
	kbdConIO();
	
	atexit(_cbKill);

	return true;
}

//+============================================================================ ========================================
bool  cbKill (void)
{
	kbdReset();

	return true;
}

//+============================================================================ ========================================
bool  cb_play (mplayer_t* pPLy,  smf_t* pSmf)
{
	static int  sel = 0;
	       int  old;
	       int  ch;

	switch ((ch = getchw())) {
		case CTRL('C') :
			drawEos();
			return false ;

		case '1'     :  _kbd[ 0].mute = !_kbd[ 0].mute;  drawKbd(&_kbd[ 0]);  break ;
		case '2'     :  _kbd[ 1].mute = !_kbd[ 1].mute;  drawKbd(&_kbd[ 1]);  break ;
		case '3'     :  _kbd[ 2].mute = !_kbd[ 2].mute;  drawKbd(&_kbd[ 2]);  break ;
		case '4'     :  _kbd[ 3].mute = !_kbd[ 3].mute;  drawKbd(&_kbd[ 3]);  break ;
		case '5'     :  _kbd[ 4].mute = !_kbd[ 4].mute;  drawKbd(&_kbd[ 4]);  break ;
		case '6'     :  _kbd[ 5].mute = !_kbd[ 5].mute;  drawKbd(&_kbd[ 5]);  break ;
		case '7'     :  _kbd[ 6].mute = !_kbd[ 6].mute;  drawKbd(&_kbd[ 6]);  break ;
		case '8'     :  _kbd[ 7].mute = !_kbd[ 7].mute;  drawKbd(&_kbd[ 7]);  break ;
		case '9'     :  _kbd[ 8].mute = !_kbd[ 8].mute;  drawKbd(&_kbd[ 8]);  break ;
		case '0'     :  _kbd[ 9].mute = !_kbd[ 9].mute;  drawKbd(&_kbd[ 9]);  break ;
		case '!'     :  _kbd[10].mute = !_kbd[10].mute;  drawKbd(&_kbd[10]);  break ;
		case '"'     :  _kbd[11].mute = !_kbd[11].mute;  drawKbd(&_kbd[11]);  break ;
		case KEY_GBP :  _kbd[12].mute = !_kbd[12].mute;  drawKbd(&_kbd[12]);  break ;  // £ == {194,163}
		case '$'     :  _kbd[13].mute = !_kbd[13].mute;  drawKbd(&_kbd[13]);  break ;
		case '%'     :  _kbd[14].mute = !_kbd[14].mute;  drawKbd(&_kbd[14]);  break ;
		case '^'     :  _kbd[15].mute = !_kbd[15].mute;  drawKbd(&_kbd[15]);  break ;

		case '_'     :                          midpSpeed(pSmf, 100            ) ;  break ;
		case '-'     :  if (pSmf->speed > 1  )  midpSpeed(pSmf, pSmf->speed - 1) ;  break ;
		case '='     :  if (pSmf->speed < 300)  midpSpeed(pSmf, pSmf->speed + 1) ;  break ;

		case ' ' :  // Pause|Continue
			if (pSmf->running) {
				if (pSmf->paused) {
					intrCont(pSmf);
					pSmf->paused = false;
				} else {
					intrPause(pSmf);
					midpPanic(pSmf);
					pSmf->paused = true;
					LOGF("<Paused>");
				}
			}
			break;
			
		case 'm' :  _kbd[sel].mute = !_kbd[sel].mute;  drawKbd(&_kbd[sel]);  break ;

		case KEY_DOWN:
			if (sel < 15) {
				sel++;
				drawSel(&_kbd[sel - 1], &_kbd[sel]);
			}
			break;
			
		case KEY_UP:
			if (sel > 0) {
				sel--;
				drawSel(&_kbd[sel + 1], &_kbd[sel]);
			}
			break;
			
		case KEY_PGUP:
			old = sel;
			sel = 0;
			drawSel(&_kbd[old], &_kbd[sel]);
			break;
			
		case KEY_PGDN:
			old = sel;
			sel = 15;
			drawSel(&_kbd[old], &_kbd[sel]);
			break;
			
		case KEY_NONE: 
		case KEY_WAIT: 
		case KEY_TRUNC: 
		case KEY_UNK: 
		case KEY_ERR: 
		default:
			break;
	}

	return true;  // return 'false' to stop playback
}
