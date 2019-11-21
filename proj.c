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

#define SECOND 1000000
#define STACK_SIZE 4096
#define MAX_THREADS 100



/***********************
black-box code


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
};


/***********************
function definitions


***********************/
void stateTransition(int);
struct TCB scheduler(int);
void dispatch(void);
void f(void);
void g(void);
void h(void);
int createThread(void (*fPtr)(void));
int getMyID(void);
void sleepThread(int);
void unsleep(int);



/***********************
linked-list definition & API


API --
    createList() returns an initialized object of type linkedList
    addToEnd(TCB, linkedList) adds TCB to end of linkedList
        -if no elements, it creates head.
    pop(linkedList) pops and removes the head from linkedList
    popN(int, linkedList) pops and removes the nth element from linkedList
    getSize(linkedList) gets the size of the linkedList
***********************/

struct listNode
{
    struct TCB x;
    struct listNode *next;
};

struct linkedList
{
    int size;
    struct listNode* head;
    struct listNode* tail;
};



struct linkedList createList()
{   
    struct linkedList myList;
    myList.head = (struct listNode *)malloc(sizeof(struct listNode));
    myList.tail = (struct listNode *)malloc(sizeof(struct listNode));
    myList.size = 0;

    return myList;
};



void createHead(struct TCB y, struct linkedList* list)
{
    list->head->x = y;
    list->head->next = list->tail;
    ++list->size;
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
    }
}

struct TCB pop(struct linkedList* list)
{
    /*
    extract necessary info from head then
    free to prevent memory leak
    */

    struct listNode *temp = list->head;
    if (list->size > 1)
        list->head = list->head->next;
    else
    {
        list->head = (struct listNode *)malloc(sizeof(struct listNode));
    }
    struct TCB y = temp->x;
    //free(temp);

    --list->size;

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
    if (n == 0)
    {
        return pop(list);
    }

    else
    {
    
        struct TCB ret;
        if (n > list->size)
        {
            printf("List only has %d elements, %d requested", list->size, n);
        }
    
        struct listNode *temp = list->head;
        struct listNode *prev = list->head;
        
    
        for (int i = 0; i < n; i++)
        {
            prev = temp;
            temp = temp->next;
        }
    
    
        ret = temp->x;
    
        //node structuring & accounting
        prev->next = temp->next;
        --list->size;
    
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
int sleeping = 0; //flag to determine whether or not to put thread to sleep




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
        srand(time(NULL));
        int r = rand() % getSize(&readyQueue);
        int n = getSize(&readyQueue);
        popped = popN(r, &readyQueue);
        return popped;      
    }
}


void dispatch()
{

    next = scheduler(schedule);
    
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


    curr = next;
    stateTransition(Running);
    if (curr.tid == 0)
    {
        printf("\n\nthread 0 runtime: %lf\n", curr.stats.runTime);
        printf("thread 0 waittime: %lf\n", curr.stats.waitTime);
        printf("thread 0 sleeptime: %lf\n", curr.stats.sleepTime);
    }

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
            printf("f: sleeping\n");
            sleepThread(2);
        }

        int j;
        while(j<99995555) j++;
    }
}

void g( void )
{
    //printf("My ID is: %d\n", GetMyID()); 
    int i=0;

    //CleanUp(); //for test
    while(1)
    {
        ++i;
        if (i % 99999555 == 0) 
        {
            printf("g: switching\n");
            dispatch();
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
           dispatch();
        }
    
        int j;
        while(j<99995555) j++;
    }
}


void sleepThread(int sec)
{
    sleeping = 1;
    alarm(sec);
    dispatch();
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
    
    dispatch();
}



int createThread(void (*fPtr)(void))
{
    struct TCB threadNode;

    
    threadNode.sp = (address_t)threadNode.stack + STACK_SIZE - sizeof(address_t);
    threadNode.pc = (address_t)fPtr;
    threadNode.tid = n;
    thrArr[n] = threadNode; //uncomment for thrArr
    

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


/************************
main

************************/
int main()
{
    do
    {
        printf("Please enter 0 for Round Robin, 1 for Lottery: \n");
        scanf("%d", &schedule);
    } while (schedule != 0 && schedule != 1);
    

    //most of this stuff should be in go()
    readyQueue = createList();
    sleepQueue = createList();


    createThread(f);         
    createThread(g);
    createThread(h);


    signal(SIGVTALRM, dispatch);
    signal(SIGALRM, unsleep);
    
    timer.it_value.tv_sec = 2;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 2;
    timer.it_interval.tv_usec = 0; 

    setitimer(ITIMER_VIRTUAL, &timer, NULL);


    dispatch();
    
    return 0;
}


