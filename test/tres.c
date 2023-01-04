#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

// Will add Seconds & uSeconds to a timeval time
// Eg. To add 1.23 seconds, use add_utime(dst, 1,230000)
#define  add_utime(t, s,u)      \
  do {                          \
    t.tv_sec  += s;             \
    t.tv_usec += u;             \
    if (t.tv_usec > 999999)  {  \
      t.tv_sec += 1;            \
      t.tv_usec -= 1000000;     \
  } } while(0)

// Will check if the 'tick' time is on-or-after the 'ref'erence time
#define  passed_utime(ref, tick)  \
  ( ( ((tick.tv_sec == ref.tv_sec) && (tick.tv_usec >= ref.tv_usec))  \
      || (tick.tv_sec > ref.tv_sec)  \
    ) ? 1 : 0 )

// Calculate the difference between two uTimes
// Result is an integer value in uS (Ie. 1S = 1,000,000uS)
// If end < start, the result will be a negative value
// Using a (signed) int limits the result to about 35mins
// or, if you use an unsigned int AND ensure start<end, about 1hr11mins
#define diff_utime(st,end)  \
  ( ((end.tv_sec - st.tv_sec) *1000000)  \
    + ( (st.tv_usec >= st.tv_usec) ? (end.tv_usec - st.tv_usec)  \
                                   : (-(st.tv_usec - end.tv_usec)) ) )

// diff_mtime works as diff_utime, but reults are in mS
// Therefore, results are limited to about 24days for signed results
// or 48 days for unsigned results
#define diff_mtime(st,end)  \
  ( ((end.tv_sec - st.tv_sec) *1000)  \
    + ( (st.tv_usec >= st.tv_usec) ? ((end.tv_usec - st.tv_usec)/1000)  \
                                   : (-(st.tv_usec - end.tv_usec)/1000) ) )

int  main(int argc, char* argv[], char* env[])  
{
  #define MAX (100000)

  struct timeval  tm[MAX];
  int             us[MAX - 1];
  int             tot = 0,  low = 0,  hi = 0;
  int             i;
  int             cnt = 100000;

  if (argc == 2)  cnt = atoi(argv[1]) ;
  if (cnt > MAX)  return(printf("Max samples %d\n", MAX), 1) ;

  gettimeofday(&tm[0], NULL);
  usleep(1);
  gettimeofday(&tm[1], NULL);
  i = ((diff_utime(tm[0], tm[1]) * cnt) / 1000000) + 1;

  printf("Collecting %d samples...\n", cnt);
  printf("Estimated Runtime: %d Sec%s\n", i, (i==1)?"":"s");

  for (i = 0;  i < cnt;  i++)  {
    gettimeofday(&tm[i], NULL);
    usleep(1);
  }

  printf("Taking readings...\n");
  for (i = 0;  i < cnt - 1;  i++)  {
    us[i] = diff_utime(tm[i], tm[i + 1]);
    tot += us[i];

    if ( (!low) || (low > us[i]) )  low = us[i] ;
    if ( (!hi)  || (hi  < us[i]) )  hi  = us[i] ;
  }

  printf("Timer Resolution (uS): Lowest=%d,  Highest=%d,  Average=%d\n", 
         low, hi, tot/(cnt-1));

  exit(0);
}  
