#include "threading.h"
#include "dispatcher.h"


void f();
void g();
void h();

int main(void)
{
	int scheduleType, nThreads;
	int jumpVal;

    do
    {
        printf("Please enter 0 for Round Robin, 1 for Lottery w Random weights, 2 for User-Defined weights: \n");
        scanf("%d", &scheduleType);
    } while (scheduleType != 0 && scheduleType != 1 && scheduleType != 2);

	/*    
    do
    {
        printf("Do you want locked thread demo (0 for no, 1 for yes): \n");
        scanf("%d", &lockThread);
    } while (lockThread != 1 && lockThread != 0);
	*/

    do
    {
        printf("Please enter number of threads (1 - 100  --including locked thread): \n");
        scanf("%d", &nThreads);
    } while (nThreads < 1 || nThreads > 100);

    
    typedef void (*easyFuncs)(void);

    easyFuncs myFuncs[3]; 

    myFuncs[0] = f;
    myFuncs[1] = g;
    myFuncs[2] = h;


    dispatcher mainDispatch {nThreads, 0, scheduleType, myFuncs, 3}; //addThread(fun)
	mainDispatch.runThreads();    
}

void f() 
{

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
            
            //dispatch(0); --how to in function threadswitch?
        }

        int j;
        while(j<99995555) j++;
    }
}

void g()
{
    //printf("My ID is: %d\n", GetMyID()); 
    int i=0;
   

   	/* 
    ++nRuns;
    /*if (xyz == 10)
    {
        printf("putting thread %d to sleep --\n", curr.tid);
        sleepThread(2); //for test
    }
    
    if (nRuns == 35)
       cleanup();
    */

    while(1)
    {
        ++i;
        if (i % 99999555 == 0) 
        {
            printf("g: switching\n");
            

            //dispatch(0); --how to in function threadswitch?
        }

        int j;
        while(j<99995555) 
            j++;
    }


}

void h()
{
    //printf("My ID is: %d\n", getMyID()); 
    int i=0;

    while(1)
    {
        ++i;
        if (i % 99999555 == 0) 
        {
           printf("h: switching\n");
           
           //dispatch(0); --how to in function threadswitch?
        }
    
        int j;
        while(j<99995555) j++;
    }
}
    