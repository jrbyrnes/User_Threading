#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

int n;

void f(void);
void g(void);

void timer_handle()
{

  n++;

  if (n % 2 == 0)
  {
    f();
  }

  else if (n % 2 == 1)
  {
    g();
  }
}

void f()
{
	for (int i = 0; i < 1000000; i++)
	{
		printf("%d\n",i);
	}

  return;
}

void g()
{
  while (1)
  {
    sleep(1);
    printf("in g\n");
  }
}

int main()
{
  n = 0;
	struct itimerval timer;


	signal(SIGVTALRM,timer_handle);


  // Time to the next timer expiration.
  timer.it_value.tv_sec = 0; //time of first timer
  timer.it_value.tv_usec = 2500; //time of first timer

  // Value to put into "it_value" when the timer expires.
  timer.it_interval.tv_sec = 0; //time of all timers but the first one
  timer.it_interval.tv_usec = 2500; //time of all timers but the first one

  
  int x = setitimer(ITIMER_VIRTUAL, &timer, NULL);	

  f();

}