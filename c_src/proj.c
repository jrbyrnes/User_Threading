/***********************
Jeffrey Byrnes
Userspace Threading


***********************/

#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <time.h>
#include <semaphore.h>

#define SECOND 500000
#define TIME_QUANTUM 1*SECOND
#define STACK_SIZE 4096
#define MAX_THREADS 100




/***********************
black-box code

done
***********************/

#ifdef __x86_64__
// code for 64 bit Intel arch

typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

//A translation required when using an address of a variable
//Use this as a black box in your code.
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%fs:0x30,%0\n"
        "rol    $0x11,%0\n"
                 : "=g" (ret)
                 : "0" (addr));
    return ret;
}

#else
// code for 32 bit Intel arch

typedef unsigned int address_t;
#define JB_SP 4
#define JB_PC 5 

//A translation required when using an address of a variable
//Use this as a black box in your code.
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%gs:0x18,%0\n"
        "rol    $0x9,%0\n"
                 : "=g" (ret)
                 : "0" (addr));
    return ret;
}

#endif




/***********************
structs

done
***********************/

struct statistics
{
    int nRuns, nWaits, nSleeps;
    double runTime, waitTime, sleepTime;
    struct timeval timeStart, timeEnd;
    double avgRun, avgWait, avgSleep;
};

struct TCB
{
    char stack[STACK_SIZE];
    address_t sp, pc;
    int tid, state;
    struct statistics stats;
    int weight;
};


/***********************
function definitions


***********************/
void stateTransition(int);
struct TCB scheduler(int);
void dispatch(int);
void f(void);
void g(void);
void h(void);
int createThread(void (*fPtr)(void));
int getMyID(void);
void sleepThread(int);
void unsleep(int);
int getStatus(int, struct statistics *);
void go(void);
void initiateThreads(int, int);
void cleanup(void);
void getThreadResult(void);




/***********************
linked-list definition & API


API --
    createList() returns an initialized object of type linkedList
    addToEnd(TCB, linkedList) adds TCB to end of linkedList
        -if no elements, it creates head.
    pop(linkedList) pops and removes the head from linkedList
    popN(int, linkedList) pops and removes the nth element from linkedList
    getSize(linkedList) gets the size of the linkedList


done -- except popN    
***********************/

struct listNode
{
    struct TCB x;
    struct listNode *next;
};

struct linkedList
{
    int size;
    int totalWeight;
    struct listNode* head;
    struct listNode* tail;
};



struct linkedList createList()
{   
    struct linkedList myList;
    myList.head = (struct listNode *)malloc(sizeof(struct listNode));
    myList.tail = (struct listNode *)malloc(sizeof(struct listNode));
    myList.size = 0;
    myList.totalWeight = 0;

    return myList;
}



void createHead(struct TCB y, struct linkedList* list)
{
    list->head->x = y;
    list->head->next = (struct listNode *)malloc(sizeof(struct listNode));
    list->head->next = list->tail;
    ++list->size;
    list->totalWeight += y.weight;
}

struct listNode * add(struct TCB y, struct linkedList* list)
{
    /*
    must initialize tail->next in order to use struct returned
    */

    list->tail->x = y;
    list->tail->next = (struct listNode *)malloc(sizeof(struct listNode));

    return list->tail->next;

}

void addToEnd(struct TCB y, struct linkedList* list)
{
    if (list->size == 0)
    {
        createHead(y,list);
    }

    else
    {
        list->tail = add(y, list);
        ++list->size;
        list->totalWeight += y.weight;
    }
}

struct TCB pop(struct linkedList* list)
{
    /*
    extract necessary info from head 
    */

    struct listNode *temp = list->head;
    if (list->size > 1)
        list->head = list->head->next;
    else
    {
        //free(list->head);
        list->head = (struct listNode *)malloc(sizeof(struct listNode));
    }
    struct TCB y = temp->x;
    //free(temp);

    --list->size;
    list->totalWeight -= y.weight;

    return y;
}

void traverse(struct linkedList* list)
{
    struct listNode *temp = list->head;
    for (int i = 0; i < list->size; i++)
    {
        printf("%d\n", temp->x.tid);
        temp = temp->next;
    }


}

struct TCB popN(int n, struct linkedList* list)
{
    int runningWeight = 0;
    if (n <= list->head->x.weight)
    {
        return pop(list);
    }

    else
    {
    
        struct TCB ret;
        if (n > list->totalWeight)
        {
            printf("Random value %d, total weight %d", n, list->totalWeight);
        }
    
        struct listNode *temp = list->head;
        struct listNode *prev = list->head;
        
    
        while (n > (runningWeight + temp->x.weight))
        {
            prev = temp;
            if (!temp->next->x.sp)
            {
                break;
            }
            temp = temp->next;
            runningWeight += temp->x.weight;
        }
    
    
        ret = temp->x;
    
        //node structuring & accounting
        prev->next = temp->next;
        --list->size;
        list->totalWeight -= ret.weight;
    
        return ret;
    }

}

int getSize(struct linkedList* list)
{
    return list->size;
}


/************************ 
globals


************************/

sigjmp_buf jbuf[MAX_THREADS];
int n = 0;
struct TCB curr;
struct TCB next;
struct TCB thrArr[MAX_THREADS]; //uncomment for thrArr
struct linkedList readyQueue;
struct linkedList sleepQueue;
enum State{Running = 0, Ready = 1, Asleep = 2, Done = 3};
struct itimerval timer;
int schedule;
int randomThreads;
int sleeping = 0; //flag to determine whether or not to put thread to sleep
struct statistics* thrStatus;
sigjmp_buf mainbuf;
int xyz = 0;
sem_t mutex;




/************************
functions

************************/


void stateTransition(int newState)
{
    gettimeofday(&curr.stats.timeEnd, NULL);
    double window = (double) (curr.stats.timeEnd.tv_usec - curr.stats.timeStart.tv_usec) / 1000000 + (double)(curr.stats.timeEnd.tv_sec - curr.stats.timeStart.tv_sec);

    switch(curr.state)
    {
        case 0:
            ++curr.stats.nRuns;
            curr.stats.runTime += window;
            curr.stats.avgRun = curr.stats.runTime / curr.stats.nRuns;
            break;
        case 1:
            ++curr.stats.nWaits;
            curr.stats.waitTime += window;
            curr.stats.avgWait = curr.stats.waitTime / curr.stats.nWaits;
            break;
        case 2:
            ++curr.stats.nSleeps;
            curr.stats.sleepTime += window;
            curr.stats.avgSleep = curr.stats.sleepTime / curr.stats.nSleeps;
            break;
        default:
            break;
    }

    curr.state = newState;
    gettimeofday(&curr.stats.timeStart, NULL);

    thrArr[curr.tid] = curr;


}


struct TCB scheduler(int sched)
{
    struct TCB popped;
    if (sched == 0)
    {
        return pop(&readyQueue);
    }
    
    else
    {
        //srand is called multiple times per second, generating same numbers
        //add curr.tid to add a randomization layer
        int r = (rand() % readyQueue.totalWeight)+1;
        printf("Randomly picked %d\n",r);
        popped = popN(r, &readyQueue);
        return popped;      
    }
}


void dispatch(int signum)
{
    if (signum)
        printf("timer interrupt\n");
    
    sem_wait(&mutex);


    if (sleeping == 0)
        stateTransition(Ready);

    else
    {
        stateTransition(Asleep);
    }


    //in the first run of dispatch, curr will be null
    if (curr.sp)
    {
        if (sleeping == 0)
        {
            addToEnd(curr, &readyQueue);
            thrArr[curr.tid] = curr;
        }

        else
        {
            addToEnd(curr, &sleepQueue);
            thrArr[curr.tid] = curr;
            sleeping = 0; //reset the flag
        }
    }

    next = scheduler(schedule);




    curr = next;
    stateTransition(Running);

    //for tracking & debugging
    /*
    if (curr.tid == 0)
    {
        getStatus(0, thrStatus);
    }
    */

    sem_post(&mutex);
    siglongjmp(jbuf[curr.tid],1);


}


void f( void ) {

    //printf("My ID is: %d\n", GetMyID()); 
    int i=0;

    while(1) 
    {
        ++i;
        if (i % 99999555 == 0) 
        {
            //printf("f: sleeping\n");
            //sleepThread(2);

            printf("f: switching\n");
            dispatch(0);
        }

        int j;
        while(j<99995555) j++;
    }
}

void g( void )
{
    //printf("My ID is: %d\n", GetMyID()); 
    int i=0;
    
    
    ++xyz;
    if (xyz == 10)
    {
        printf("putting thread %d to sleep --\n", curr.tid);
        sleepThread(2); //for test
    }

    if (xyz == 35)
       cleanup();
    

    while(1)
    {
        ++i;
        if (i % 99999555 == 0) 
        {
            printf("g: switching\n");
            dispatch(0);
        }

        int j;
        while(j<99995555) 
            j++;
    }


}

void h( void )
{
    //printf("My ID is: %d\n", getMyID()); 
    int i=0;

    while(1)
    {
        ++i;
        if (i % 99999555 == 0) 
        {
           printf("h: switching\n");
           dispatch(0);
        }
    
        int j;
        while(j<99995555) j++;
    }
}

void getThreadResult()
{
    sem_wait(&mutex);

    for (int i = 0; i < 5; i++)
    {
        printf("Locked function, iteration %d complete\n", i);
        sleep(1);
    }

    printf("Function complete, unlocking\n\n");

    sem_post(&mutex);
    dispatch(0);
}


void sleepThread(int sec)
{
    sleeping = 1;
    signal(SIGALRM, unsleep);
    alarm(sec);
    dispatch(0);
}

void unsleep(int signum)
{
    struct TCB temp = pop(&sleepQueue);

    /*
    code re-use from stateChange
    if I want to use the stateChange function then
    add a second parameter that takes the TCB node
    return that TCB node from stateChange
    have the node passed be the value that receives assign
    (LHS)
    apply fixes to dispatch
    this function would look like
        temp = stateChange(Ready, temp);
        thrArr[temp.tid] = temp;
        addToEnd(temp, &readyQueue);
    */
    gettimeofday(&temp.stats.timeEnd, NULL);
    double window = (double) (temp.stats.timeEnd.tv_usec - temp.stats.timeStart.tv_usec) / 1000000 + (double)(temp.stats.timeEnd.tv_sec - temp.stats.timeStart.tv_sec);
    ++temp.stats.nSleeps;
    temp.stats.sleepTime += window;
    temp.stats.avgSleep = curr.stats.sleepTime / curr.stats.nSleeps;
    temp.state = Ready;
    gettimeofday(&temp.stats.timeStart, NULL);


    thrArr[temp.tid] = temp;
    addToEnd(temp, &readyQueue);
    

}



int createThread(void (*fPtr)(void))
{
    struct TCB threadNode;

    
    threadNode.sp = (address_t)threadNode.stack + STACK_SIZE - sizeof(address_t);
    threadNode.pc = (address_t)fPtr;
    threadNode.tid = n;
    //threadNode.stats = (struct stats)malloc(sizeof(struct stats)); //comment out maybe
    if (schedule == 0)
        threadNode.weight = 0;

    else
    {
        if (randomThreads == 1)
        {
            threadNode.weight = (rand() % 100) + 1;
            printf("Thread %d has weight %d\n", threadNode.tid, threadNode.weight);  
        }

        else
        {
            do
            {
                printf("Enter weight for thread %d: ", threadNode.tid);
                scanf("%d", &threadNode.weight);
            } while (threadNode.weight < 1 || threadNode.weight > 100);
        }
         
    }

    

    threadNode.stats.runTime = 0;
    threadNode.stats.waitTime = 0;
    threadNode.stats.sleepTime = 0;
    threadNode.stats.nRuns = 0;
    threadNode.stats.nWaits = 0;
    threadNode.stats.nSleeps = 0;
    gettimeofday(&threadNode.stats.timeStart, NULL);

    threadNode.state = Ready;

    addToEnd(threadNode, &readyQueue);
    thrArr[n] = threadNode;


    sigsetjmp(jbuf[threadNode.tid],1);
    (jbuf[n]->__jmpbuf)[JB_SP] = translate_address(threadNode.sp);
    (jbuf[n]->__jmpbuf)[JB_PC] = translate_address(threadNode.pc);
    sigemptyset(&jbuf[threadNode.tid]->__saved_mask);     

    ++n;    
    return threadNode.tid;
}

int getMyID()
{
    return curr.tid;
}

int getStatus(int id, struct statistics *tempStat)
{
    struct TCB temp = thrArr[id];
    tempStat = &temp.stats;

    printf("\nSummary Statistics thread:  %d\n", temp.tid);
    printf("Thread currently in state: ");
    switch(temp.state)
    {
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

    if (schedule == 1)
    {
        printf("Weight for Lottery:  %15d\n", temp.weight);    
    }

    printf("Number of Times Ran: %8d\n", temp.stats.nRuns);
    printf("Total Run Time:      %15lf\n", temp.stats.runTime);
    printf("Total Sleep Time:    %15lf\n", temp.stats.sleepTime);
    printf("Average Wait Time:   %15lf\n", temp.stats.avgWait);
    printf("Average Run Time:    %15lf\n\n\n", temp.stats.avgRun);

    return temp.tid;
}


void initiateThreads(int nThreads, int lockThread)
{
    readyQueue = createList();
    sleepQueue = createList();

    if (schedule == 1)
    {
        printf("\nThe weights are: \n");
    }

    

    for (int i = 0; i < nThreads; i++)
    {
        if (i % 3 == 0)
            createThread(f);
        else if (i % 3 == 1)
            createThread(g);
        else
            createThread(h);
    }
    
    if (lockThread == 1)
    {
        createThread(getThreadResult);
    }




}

void go()
{
    signal(SIGVTALRM, dispatch);

    
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = TIME_QUANTUM;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = TIME_QUANTUM; 

    setitimer(ITIMER_VIRTUAL, &timer, NULL);

    sem_init(&mutex, 0, 1);


    dispatch(0);
}

void cleanup()
{
    //print summary statistics for each node
    //if further analys

    while (getSize(&sleepQueue))
    {
        unsleep(0);
    }
    stateTransition(Ready);
    for (int i = 0; i < n; i++)
    {
        getStatus(i, thrStatus);
    }


    //turn off timer (shut down scheduling)
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0; 

    setitimer(ITIMER_VIRTUAL, &timer, NULL);

    /*
    node deletion:
    Note: 1 copy kept in the queues (on heap), and another kept in stack
    in thrArr. I can free the nodes of the queues, but can not free the 
    nodes in the array. However, when main returns, the stack will be deleted.
    Main will return shortly after cleaning up the heap.
    */

    free(sleepQueue.tail);
    free(sleepQueue.head);
    free(readyQueue.tail);
    free(readyQueue.head);


    printf("\n\nAll clean -- exiting\n\n");

    siglongjmp(mainbuf,1);


}


/************************
main

************************/
int main()
{
    int jumpval;
    int numThreads;
    int lockThread;


    do
    {
        printf("Please enter 0 for Round Robin, 1 for Lottery: \n");
        scanf("%d", &schedule);
    } while (schedule != 0 && schedule != 1);

    if (schedule == 1)
    {
        do
        {
            printf("Randomized Weights (0 for no, 1 for yes): \n");
            scanf("%d", &randomThreads);
        } while (randomThreads != 0 && randomThreads != 1);
    }
    
    do
    {
        printf("Do you want locked thread demo (0 for no, 1 for yes): \n");
        scanf("%d", &lockThread);
    } while (lockThread != 1 && lockThread != 0);

    do
    {
        printf("Please enter number of threads (1 - 100  --including locked thread): \n");
        scanf("%d", &numThreads);
    } while (numThreads < 1 || numThreads > 100);

    
    initiateThreads(numThreads,lockThread);
    

    jumpval = sigsetjmp(mainbuf,1);

    if (jumpval == 0)
        go();
    
    else
        return 0;
}


