#include "midp_play.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#if PLATFORM == PLATFORM_PC
//+============================================================================ ========================================
// Linux PC interrupt timer
//
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <assert.h>

#define DEJITTER 1  // we can try to dejitter the interrupts - but without an RTOS it's all fudgery

static    uint32_t        uspdt;    // the interrupt needs to know this
volatile  delta_t*        pDelta;   // pointer to the track delta we are using
static    float           deltaF;   // we track the delta as a float

#if DEJITTER
	static    struct timeval  tv[2];    // best chance at accurate delta timing :/
	static    int             tvi = 0;  // 'last' and 'now'
#endif 

//+============================================================================ ========================================
// Interrupt handler
//
static
void  intrHandler (int signal)
{
	(void)signal;

#if DEJITTER
	gettimeofday(&tv[tvi = !tvi], NULL);

	float  tick = (float)(((tv[ tvi].tv_sec * 1000000) + tv[ tvi].tv_usec) - 
	                      ((tv[!tvi].tv_sec * 1000000) + tv[!tvi].tv_usec)  ) / uspdt ;

	*pDelta = (delta_t)(deltaF += tick);

#else
	(*pDelta)++;
#endif

//	write(2, ".", 1);  // 1 dot to 2=stderr (stdout is 1)

//	write(2, "\033[1;31m\033[1;1H", 13);  // 1 dot to 2=stderr (stdout is 1)
//	static  int i = 0;
//	write(2, &("-/|\\"[(i=(i+1)&3)]), 1);  // 1 dot to 2=stderr (stdout is 1)
}

//+============================================================================ ========================================
// Initialise the timer interrupt
//
void  _intrKill (void)  {  (void)intrKill();  }

bool  intrInit (void)
{
	sigset_t          block;
	struct sigaction  act   = { .sa_handler = intrHandler };

	sigemptyset(&block);
	sigaddset(&block, SIGALRM);

	sigemptyset(&act.sa_mask);
	
	atexit(_intrKill);

	return !sigaction(SIGALRM, &act, NULL);
}

//+============================================================================ ========================================
// Stop the timer interrupt
//
bool  intrKill (void)
{
	intrStop(NULL);
	pDelta = NULL;

	return true;
}

//+============================================================================ ========================================
// Start the timer interrupt
//
bool  intrStart (smf_t* pSmf)
{
	pSmf->uspdt = ((60 * 1000000) / BPM_DEFAULT) / pSmf->mthd->tpqn;
	
#if DEJITTER
	deltaF = 0;
#endif	
	
	pSmf->delta = 0;
	pDelta      = &pSmf->delta;

#if DEJITTER
	gettimeofday(&tv[tvi = 0], NULL);
#endif	

	return intrPeriod(pSmf);
}

//+============================================================================ ========================================
// Set interrupt period for 1 delta tick (specified in microSeconds 0.000'001)
//
bool  intrPeriod (smf_t* pSmf)
{
	uspdt = (pSmf->uspdt * 100) / pSmf->speed;
	
	struct itimerval  period = {  // fire as fast as possible (10mS on my machine)
		.it_interval.tv_sec = 0,  .it_interval.tv_usec = uspdt, 
		.it_value.tv_sec    = 0,  .it_value.tv_usec    = uspdt
	};
	
#if DEJITTER
	gettimeofday(&tv[tvi], NULL);
#endif
	
	return !setitimer(ITIMER_REAL, &period, NULL);
}

//+============================================================================ ========================================
// Pause the timer interrupt
//
bool  intrPause (smf_t* pSmf)
{
	return intrPeriod(&(smf_t){ .uspdt = 0,  .speed = 1 });
}

//+============================================================================ ========================================
// Un-pause the timer interrupt
//
bool  intrCont (smf_t* pSmf)
{
	return intrPeriod(pSmf);
}

//+============================================================================ ========================================
// Stop the timer interrupt
//
bool  intrStop (smf_t* pSmf)
{
	return intrPeriod(&(smf_t){ .uspdt = 0,  .speed = 1 });
}
#endif//PLATFORM_PC

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#if PLATFORM == PLATFORM_XXX
//+============================================================================ ========================================
// Template interrupt timer
//
volatile  delta_t*  pDelta;  // pointer to the track delta we are using

//+============================================================================ ========================================
// Interrupt handler
//
static
void  intrHandler (int signal)
{
	(void)signal;
	(*pDelta)++;
}

//+============================================================================ ========================================
// Initialise the timer interrupt
//
bool  intrInit (void)
{
	~~~[ INITIALISE(hardware) ]~~~

	return true;
}

//+============================================================================ ========================================
// Stop the timer interrupt
//
bool  intrKill (void)
{
	intrStop();
	pDelta = NULL;

	~~~[ RELEASE(hardware) ]~~~
	
	return true;
}

//+============================================================================ ========================================
// Start the timer interrupt
//
bool  intrStart (smf_t* pSmf)
{
	pSmf->uspdt = ((60 * 1000000) / BPM_DEFAULT) / pSmf->mthd->tpqn;
	
	pSmf->delta = 0;
	pDelta      = &pSmf->delta;

	intrPeriod(pSmf->uspdt);
	
	~~~[ START_INTERRUPT() ]~~~

	return true;
}

//+============================================================================ ========================================
// Set interrupt period for 1 delta tick (specified in microSeconds 0.000'001)
//
bool  intrPeriod (smf_t* pSmf)
{
	~~~[ CHANGE_INTERRUPT_PERIOD(pSmf->uspdt) ]~~~
	
	return true;
}

//+============================================================================ ========================================
// Pause the timer interrupt
//
bool  intrPause (smf_t* pSmf)
{
	~~~[ PAUSE_INTERRUPT() ]~~~

	return true;
}

//+============================================================================ ========================================
// Un-pause the timer interrupt
//
bool  intrCont (smf_t* pSmf)
{
	~~~[ UNPAUSE_INTERRUPT() ]~~~

	return true;
}

//+============================================================================ ========================================
// Stop the timer interrupt
//
bool  intrStop (void)
{
	~~~[ STOP_INTERRUPT() ]~~~

	return true;
}

#endif//PLATFORM_XXX
