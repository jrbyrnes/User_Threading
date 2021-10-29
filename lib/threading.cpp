#include "user_threading/threading.h"
#include "user_threading/dispatcher.h"


void f();
void g();
void h();

int main(void) {
	int scheduleType, nThreads;
	int jumpVal;

    do {
        printf("Please enter 0 for Round Robin, 1 for Lottery w Random weights, 2 for User-Defined weights: \n");
        scanf("%d", &scheduleType);
    } while (scheduleType != 0 && scheduleType != 1 && scheduleType != 2);

    do {
        printf("Please enter number of threads (1 - 100  --including locked thread): \n");
        scanf("%d", &nThreads);
    } while (nThreads < 1 || nThreads > 100);

    
    typedef void (*easyFuncs)(void);

    easyFuncs myFuncs[3]; 

    myFuncs[0] = f;
    myFuncs[1] = g;
    myFuncs[2] = h;


    dispatcher mainDispatch {nThreads, 0, scheduleType, myFuncs, 3};
	mainDispatch.runThreads();    
}

void f()  {
    int i=0;

    while(1)  {
        ++i;
        if (i % 99999555 == 0)  {
            printf("f: switching\n");
        }

        int j;
        while(j<99995555) j++;
    }
}

void g() {
    int i=0;
   
    while(1) {
        ++i;
        if (i % 99999555 == 0)  {
            printf("g: switching\n");
        }

        int j;
        while(j<99995555) 
            j++;
    }


}

void h() {
    int i=0;

    while(1) {
        ++i;
        if (i % 99999555 == 0)  {
           printf("h: switching\n"); 
        }
    
        int j;
        while(j<99995555) j++;
    }
}
    