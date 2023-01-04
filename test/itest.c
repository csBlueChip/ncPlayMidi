#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <assert.h>

static    sigset_t  intrBlock;
volatile  int       intrTick;

//+============================================================================ ========================================
// Interrupt handler
//
static
void  intrHandler (int signal)
{
	(void)signal;
	intrTick++;
//	write(2, ".", 1);  // dot to stderr
}

//+============================================================================ ========================================
// This is VERY lazy code - but it works - well enough for a PoC||GTFO
//
void  intrRun (int n)
{
	struct sigaction  act      = {0};
	struct timeval    interval;
	struct itimerval  period;

	sigemptyset(&intrBlock);
	sigaddset(&intrBlock, SIGVTALRM);

	act.sa_handler = intrHandler;
	assert(sigaction(SIGVTALRM, &act, NULL) == 0);

	interval.tv_sec    = 0;
	interval.tv_usec   = n;

	period.it_interval = interval;
	period.it_value    = interval;

	setitimer(ITIMER_VIRTUAL, &period, NULL);
}

//+============================================================================ ========================================
// Calculate the difference between two uTimes
// Result is an integer value in uS (Ie. 1S = 1,000,000uS)
// If end < start, the result will be a negative value
// Using a (signed) int limits the result to about 35mins
// or, if you use an unsigned int AND ensure start<end, about 1hr11mins
#define diff_utime(st,end)  \
  ( ((end.tv_sec - st.tv_sec) *1000000)  \
    + ( (st.tv_usec >= st.tv_usec) ? (end.tv_usec - st.tv_usec)  \
                                   : (-(st.tv_usec - end.tv_usec)) ) )

void  main (int argc, char* argv[])
{
	struct timeval  st, nd;

	int n = atoi(argv[1]);
	printf("%d : Waiting...\n", n);

	intrRun(n);

	gettimeofday(&st, NULL);
	intrTick = 0;
	while (intrTick < 100) ;
	gettimeofday(&nd, NULL);

	intrRun(0);

	int x = diff_utime(st,nd);
	printf("=>%d.%06d\n", x/1000000, x%1000000);

}