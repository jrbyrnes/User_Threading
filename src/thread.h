#ifndef THREAD_H
#define THREAD_H



class statistics {
    public:
    	int nRuns, nWaits, nSleeps;
    	double runTime, waitTime, sleepTime;
    	struct timeval timeStart, timeEnd;
    	double avgRun, avgWait, avgSleep;
    	statistics();
    	void setTimeStart();
    	void setTimeEnd();
    	void printStats(int tid, int state, int weight);
};

statistics::statistics() {
	nRuns = nWaits = nSleeps = 0;
    runTime = waitTime = sleepTime = 0;
    avgRun = avgWait = avgSleep = 0;   
}

void statistics::setTimeStart() {
	gettimeofday(&timeStart, NULL);
}

void statistics::setTimeEnd() {
	gettimeofday(&timeEnd, NULL);
}

/******************************************************************/

///need thread copy assignment for node

class thread {
	public:
    	char stack[STACK_SIZE];
    	address_t sp, pc;
    	int tid, state, scheduleType;
    	struct statistics* stats;
    	int weight;
    	thread(void (*fPtr)(void), int nexTID, int scheduleType);
    	thread();
    	int getTid();
    	void stateTransition(int newState);
    	void printStats();		
};

thread::thread() {
	//stack = NULL;
	sp =  pc = weight =0;
	tid = state = scheduleType = -1;
	stats = nullptr;
}

thread::thread(void (*fPtr)(void), int nextTID, int scheduleType) {
	this->scheduleType = scheduleType; //should be global?
    
    sp = (address_t)stack + STACK_SIZE - sizeof(address_t);
    pc = (address_t)fPtr;
    tid = nextTID;

    //equal weights
    if (scheduleType == 0) 
        weight = 0;

    else
    {
        //random weights
        if (scheduleType == 1) {
            weight = (rand() % 100) + 1;
            printf("Thread %d has weight %d\n", tid, weight);  
        }

        //custom weights
        if (scheduleType == 2)  {
            do {
                printf("Enter weight for thread %d (1-100) : ", tid);
                scanf("%d", &weight);
            } while (weight < 1 || weight > 100);
        }
         
    }

    stats = new statistics;
    stats->setTimeStart();
  
    state = Ready;


}

int thread::getTid() {
	return tid;
}

void thread::stateTransition(int newState) {
	stats->setTimeEnd();
    double window = (double) (stats->timeEnd.tv_usec - stats->timeStart.tv_usec) / 1000000 + (double)(stats->timeEnd.tv_sec - stats->timeStart.tv_sec);

    switch(state) {
        case 0:
            ++stats->nRuns;
            stats->runTime += window;
            stats->avgRun = stats->runTime / stats->nRuns;
            break;
        case 1:
            ++stats->nWaits;
            stats->waitTime += window;
            stats->avgWait = stats->waitTime / stats->nWaits;
            break;
        case 2:
            ++stats->nSleeps;
            stats->sleepTime += window;
            stats->avgSleep = stats->sleepTime / stats->nSleeps;
            break;
        default:
            break;
    }

    state = newState;
    stats->setTimeStart();
}

void thread::printStats() {
    printf("\nSummary Statistics thread:  %d\n", tid);
    printf("Thread currently in state: ");
    switch(state) {
        case 0:
            printf(" Running\n");
            break;
        case 1:
            printf(" Waiting\n");
            break;
        case 2:
            printf(" Sleeping\n");
            break;
        default:
            break;
    }

    if (scheduleType == 1 || scheduleType == 2) {
        printf("Weight for Lottery:  %15d\n", weight);    
    }

    printf("Number of Times Ran: %8d\n", stats->nRuns);
    printf("Total Run Time:      %15lf\n", stats->runTime);
    printf("Total Sleep Time:    %15lf\n", stats->sleepTime);
    printf("Average Wait Time:   %15lf\n", stats->avgWait);
    printf("Average Run Time:    %15lf\n\n\n", stats->avgRun);
}


/******************************************************************/




#endif