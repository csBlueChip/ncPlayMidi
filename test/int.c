#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>

#define PERIOD 99999

static sigset_t  block ;

//static void init( ) __attribute__((constructor));

void timer_handler(int sig){
	write(1, ".", 1); 
}

void timer_handler2(int sig){
	write(1, "*", 1); 
}
void timer_handler3(int sig){
	write(1, "+", 1); 
}

void  intr (unsigned long s,  unsigned long us,  void(*intfn)(int))
{
	struct sigaction  act      = {0};
	struct timeval    interval;
	struct itimerval  period;

	while (us >= 1000000)  s++, us -= 1000000 ;

	sigemptyset(&block);
	sigaddset(&block, SIGVTALRM);

	act.sa_handler = timer_handler;
	assert(sigaction(SIGVTALRM, &act, NULL) == 0);

	interval.tv_sec    = s;
	interval.tv_usec   = us;

	period.it_interval = interval;
	period.it_value    = interval;

	setitimer(ITIMER_VIRTUAL, &period, NULL);
}

int  main (void)
{
	intr(0, 1, timer_handler);
	for(int i=0;i<100000000;i++);
	intr(0, 1, timer_handler2);
	timer_handler3(0);
	intr(0, 1, timer_handler2);
	timer_handler3(0);
	intr(0, 1, timer_handler2);
	timer_handler3(0);
	intr(0, 1, timer_handler2);
	timer_handler3(0);
	intr(0, 1, timer_handler2);
	timer_handler3(0);
	intr(0, 1, timer_handler2);
	timer_handler3(0);
	intr(0, 1, timer_handler2);
	timer_handler3(0);
	intr(0, 1, timer_handler2);
	timer_handler3(0);
	intr(0, 1, timer_handler2);
	timer_handler3(0);
	intr(0, 1, timer_handler2);
	timer_handler3(0);


	return 0;
}
