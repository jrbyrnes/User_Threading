#include <stdio.h>
#include <time.h>
#include <unistd.h>

int main()
{
	time_t start, stop;
	time_t running = 0;

	int runs = 0;
	double avgRun;

	start = time(NULL);
	sleep(3);
	stop = time(NULL);

	running += (stop - start);
	++runs;


	start = time(NULL);
	sleep(2);
	stop = time(NULL);

	running += (stop - start);
	++runs;

	avgRun = (double)running / (double)runs;

	printf("%f",avgRun);

}