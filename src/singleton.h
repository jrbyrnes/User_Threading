#include <csetjmp>
#include <signal.h>
#include <sys/time.h>
#include <signal.h>
#include <stdio.h>


sigjmp_buf mainbuf;

class singleton
{
		int x = 0, jumpval;
		struct itimerval timer;


		//static singleton* singleton_;

	public:
		singleton();
		singleton(int x);
		static singleton* getInstance();		
		static void sigHandler(int signalType);
		void runThread();
		void incrementX();
		void operator=(const singleton &) = delete;
};

singleton* instance;


singleton::singleton()
{
	this->x = 0;
	instance = this;
	//singleton_ = this;
}

singleton::singleton(int x)
{
	this->x = x;
	instance = this;
	//singleton_ = this;
}


void singleton::runThread()
{
	signal(SIGVTALRM, singleton::sigHandler);
    
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 5;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 5;

    setitimer(ITIMER_VIRTUAL, &timer, NULL);
 

    jumpval = sigsetjmp(mainbuf,1);

    if (jumpval == 0)
    	incrementX();
 
}


void singleton::sigHandler(int signalType)
{
	//instance = globalDispatch;

	instance->incrementX();

}

void singleton::incrementX()
{

	//instance = this;
	printf("dispatch called incrementX\n");
	printf("x before increment: %d\n", x);
	++x;
	while (true)
		;
}
