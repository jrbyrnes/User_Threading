#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "threading.h"
#include "thread.h"
#include "node.h"
#include "linkedList.h"





//state transition



class dispatcher {
		int n, nThreads, lockThread, scheduleType,  jumpval;//, signum, numFuncs,;
		int nSwitches;
		node** threadArr;

		linkedList* threadList;
		linkedList* sleepList;
		struct itimerval timer;
		node* curr;
		node* next;
		sem_t mutex;

		//static dispatcher* dispatcher_;


		//void (*funPtrArr[])(void);
		//int sleeping = 0; //not yet supported

	public: 
		dispatcher(int nThreads, int lockThread, int schedule, void (*funPtrArr[])(void), int numFuncs);
		dispatcher();
		int assignTID();
		void initiateThreads(void (*funPtrArr[])(void), int numFuncs);
		int runThreads();
		void switcher(int signalType);
		struct node* scheduler(); 
		void cleanUp();
		static void sigHandler(int signalType);
		//static dispatcher* getInstance();


};

dispatcher* instance;

dispatcher::dispatcher() {
	n = nThreads = nSwitches = 0;//numFuncs = 0;//signum = 0;
	lockThread = scheduleType = jumpval = -1;
	threadArr = nullptr;
	threadList = nullptr;
	sleepList = nullptr;
	curr = nullptr;
	next = nullptr;
	instance = this;
	//dispatcher_ = nullptr;
}

dispatcher::dispatcher(int nThread, int lockThread, int schedule, void (*funPtrArr[])(void), int numFuncs) {
	n = nSwitches = 0;
	this->nThreads = nThread;
	this->lockThread = lockThread;
	this->scheduleType = schedule;
	/*this->numFuncs = numFuncs;
	this->funPtrArr = funPtrArr;*/

	threadArr = new node *[nThreads];
	//node * threadArr[nThreads];

	threadList = new linkedList;
	sleepList = new linkedList;

	curr = nullptr;
	next = nullptr;

	initiateThreads(funPtrArr,numFuncs);

	instance = this;
	//dispatcher_ = this;
}



int dispatcher::assignTID() {
	int temp = n;
	++n;
	return(temp);
}

void dispatcher::initiateThreads(void (*funPtrArr[])(void), int numFuncs) {
	for (int i = 0; i < nThreads; i++) {
		thread *temp = new thread(funPtrArr[i%numFuncs],assignTID(),scheduleType);
		node *tempNode = new node(temp);


    	sigsetjmp(jbuf[temp->tid],1);
    	(jbuf[temp->tid]->__jmpbuf)[JB_SP] = translate_address(temp->sp);
    	(jbuf[temp->tid]->__jmpbuf)[JB_PC] = translate_address(temp->pc);
    	sigemptyset(&jbuf[temp->tid]->__saved_mask);     


		threadList->addNode(tempNode);
		threadArr[temp->tid] = tempNode;
	}
}


void dispatcher::sigHandler(int signalType){
	instance->switcher(signalType);

}



int dispatcher::runThreads() {
	signal(SIGVTALRM, dispatcher::sigHandler);
    
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = TIME_QUANTUM;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = TIME_QUANTUM;

    setitimer(ITIMER_VIRTUAL, &timer, NULL);
 

    sem_init(&mutex, 0, 1);



    jumpval = sigsetjmp(mainbuf,1);

    if (jumpval == 0)
    	switcher(0);
 
    else
        return 0;

}


void dispatcher::switcher(int signum) {
	if (signum) {
        printf("timer interrupt\n");
    }


    ++nSwitches;

    if (nSwitches >= MAX_SWITCHES) {
    	cleanUp();
    }
    
    sem_wait(&mutex);
	

    //not sure whats up with the following lines
    //why not put them in the curr->value->sp conditional
    
    if (curr != nullptr) {
    	if (true)  {
            //sleeping == 0
        	curr->value->stateTransition(Ready);
        	threadArr[curr->value->tid] = curr;
    	}
	
    	else {
        	curr->value->stateTransition(Asleep);
        	threadArr[curr->value->tid] = curr;
    	}
    }

    //in the first run of dispatch, curr will be null
    if (curr != nullptr) {
        if (true) {   
            //sleeping == 0
        	threadList->addNode(curr);    
        }

        else {
            sleepList->addNode(curr);
            //sleeping = 0; //reset the flag
        }

        threadArr[curr->value->tid] = curr;
    }

    //start here on first run of dispatch
    next = scheduler();




    curr = next;
    curr->value->stateTransition(Running);

    //for tracking & debugging
    /*
    if (curr.tid == 0)
    {
        getStatus(0, thrStatus);
    }
    */

    sem_post(&mutex);
    siglongjmp(jbuf[curr->value->tid],1);
}


node* dispatcher::scheduler() {
    node *popped;
    if (scheduleType == 0) {
        return threadList->pop();
    }
    
    else {
        //srand is called multiple times per second, generating same numbers
        //add curr.tid to add a randomization layer
        int r = (rand() % threadList->getTotalWeight())+1;
        printf("Randomly picked %d\n",r);
        popped = threadList->popN(r);
        return popped;      
    }

}


void dispatcher::cleanUp() {
    //print summary statistics for each node
    //if further analys

	//sleep not supported
	/*
    while (sleepQueue->getSize())
    {
        unsleep(0);
    }
    */

    curr->value->stateTransition(Ready);
    threadArr[curr->value->tid] = curr;
    threadList->addNode(curr);

    //prints stats -- should probably traverse array to get inorder
    threadList->traverse(threadList->getHead()); 


    //turn off timer (shut down scheduling)
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0; 

    setitimer(ITIMER_VIRTUAL, &timer, NULL);

    /*
    node deletion: --not yet implemented
    Note: 1 copy kept in the queues (on heap), and another kept in stack
    in thrArr. I can free the nodes of the queues, but can not free the 
    nodes in the array. However, when main returns, the stack will be deleted.
    Main will return shortly after cleaning up the heap.


    free(sleepQueue.tail);
    free(sleepQueue.head);
    free(readyQueue.tail);
    free(readyQueue.head);
	*/

    printf("\n\nAll clean -- exiting\n\n");

    siglongjmp(mainbuf,1);


}

#endif