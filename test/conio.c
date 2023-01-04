#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>

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

	/* take two copies - one for now, one for later */
	tcgetattr(0, &kbdOrig);
	memcpy(&kbdNew, &kbdOrig, sizeof(kbdNew));

	/* register cleanup handler, and set the new terminal mode */
	atexit(kbdReset);
	cfmakeraw(&kbdNew);
	tcsetattr(0, TCSANOW, &kbdNew);
}

//+============================================================================ 
static
int  kbhit (void)
{
	struct timeval  tv  = { 0L, 0L };
	fd_set          fds;
	
	FD_ZERO(&fds);
	FD_SET(0, &fds);
	
	return select(1, &fds, NULL, NULL, &tv);
}

//+============================================================================ 
static
int  getch (void)
{
	int            r;
	unsigned char  c;

	return ((r = read(0, &c, sizeof(c))) < 0) ?  r : c ;
}



#include <stdio.h>
int main(int argc, char *argv[])
{
	int i;

	kbdConIO();

ten:
	i = 0;
	while (!kbhit()) {
//		if (!(i++ % 50000)) {printf(".");fflush(stdout);}
		/* do some work */
	}

	int c = getch(); /* consume the character */
	if (c == '\e')  { printf("\r\nESC¦"); fflush(stdout); }
//	else            { printf("%c¦", c);   fflush(stdout); }
	else            { printf("%02X¦", c); fflush(stdout); }

	if (c==3) exit(0);
goto ten;
}
