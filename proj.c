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

struct TCB
{
    char stack[STACK_SIZE];
    address_t sp, pc;
    int tid, state; 
};



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
    list->head = list->head->next;
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
    int i = 0;
    struct TCB ret;
    if (n-- > list->size)
    {
        printf("List only has %d elements, %d requested", list->size, n);
    }

    struct listNode *temp = list->head;
    struct listNode *prev = list->head;
    
    while (temp->x.tid)
    {
        if (i == n)
        {
            //printf("requested element found with element # %d \n", temp->x);
            break;
        }
        prev = temp;
        temp = temp->next;
        ++i;
    }

    ret = temp->x;

    //node structuring & accounting
    prev->next = temp->next;
    free(temp);
    --list->size;

    return ret;

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
enum State{Running = 0, Waiting = 1, Asleep = 2, Done = 3};
struct itimerval timer;



/************************
functions

************************/



void dispatch()
{
    next = pop(&readyQueue);

    //in the first run of dispatch, curr will be null
    if (curr.sp)
    {
        addToEnd(curr, &readyQueue);
    }

    curr.state = Waiting;
    curr = next;
    
    /*
    int ret_val = sigsetjmp(jbuf[x],1);
    printf("SWITCH: ret_val=%d\n", ret_val); 
    if (ret_val == 1) {
        return;
    }
    */
    
    int x = curr.tid;
    printf("switching to thread %d\n", curr.tid);
    curr.state = Running; 
    siglongjmp(jbuf[x],1);
}

/*
void f( void )
{
    printf("b\n");
    int a=1;
    printf("c\n");
    int i=0;
    while(1) {
        ++i;
        printf("in f (%d)\n",i);
        if (i % 3 == 0) {
            printf("f: switching\n");
            dispatch();
        }
        usleep(SECOND);
    }
}

void g( void )
{
    int i=0;
    while(1){
        ++i;
        printf("in g (%d)\n",i);
        if (i % 5 == 0) {
            printf("g: switching\n");
            dispatch();
        }
        usleep(SECOND);
    }
}

void h( void)
{
    int i = 0;
    while (1)
    {
        ++i;
        printf("in h (%d)\n",i);
        if (i % 8 == 0){
            printf("h: switching\n");
            dispatch();
        }
        usleep(SECOND);
    }
}
*/


void f( void ) {
    //printf("My ID is: %d\n", GetMyID()); 
    int i=0;

    while(1) {
        ++i;
        if (i % 99999555 == 0) {
      printf("f: switching\n");
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
    while(1){
        ++i;
        if (i % 99999555 == 0) {
       printf("g: switching\n");
        }
    int j;
        while(j<99995555) j++;
    }
}

void h( void )
{
    //printf("My ID is: %d\n", GetMyID()); 
    int i=0;

    while(1){
        ++i;
        if (i % 99999555 == 0) {
           printf("h: switching\n");
        }
    int j;
        while(j<99995555) j++;
    }
}




int createThread(void (*fPtr)(void))
{
    struct TCB threadNode;
    
    threadNode.sp = (address_t)threadNode.stack + STACK_SIZE - sizeof(address_t);
    threadNode.pc = (address_t)fPtr;
    threadNode.tid = n;
    threadNode.state = Waiting;
    thrArr[n] = threadNode; //uncomment for thrArr
    addToEnd(threadNode, &readyQueue);

    sigsetjmp(jbuf[threadNode.tid],1);
    (jbuf[n]->__jmpbuf)[JB_SP] = translate_address(threadNode.sp);
    (jbuf[n]->__jmpbuf)[JB_PC] = translate_address(threadNode.pc);
    sigemptyset(&jbuf[threadNode.tid]->__saved_mask);     

    ++n;    
    return threadNode.tid;
}




/************************
main

************************/
int main()
{
    readyQueue = createList();

    createThread(f);        
    createThread(g);
    createThread(h);


    signal(SIGVTALRM,dispatch);
    
    timer.it_value.tv_sec = 2;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 2;
    timer.it_interval.tv_usec = 0; 

    setitimer(ITIMER_VIRTUAL, &timer, NULL);

    

    
    

    dispatch();
    
    return 0;
}


