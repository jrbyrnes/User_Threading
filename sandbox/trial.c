/*
Jeffrey Byrnes
myThread implements a
user-space thread library
*/

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>



#define STACK_SIZE 4096
#define MAX_THREADS 20


typedef unsigned long address_t;



/*
structures
*/

struct TCB
{
	void **sp;
	void *pc, *bp;
	int reg[4], tid, state;
};

/*
globals
*/

int numThreads = 0;
struct TCB threadArr[MAX_THREADS];
sigjmp_buf jbuf[MAX_THREADS];




/*
functions
*/

void switchThreads(void)
{
	siglongjmp(jbuf[1],1);
}

void createStack(struct TCB *threadNode, int size)
{
	threadNode->bp = (void *) malloc(STACK_SIZE);
	threadNode->sp = (void **) (threadNode->bp + size);

}

void trialFunction(void)
{
	printf("Hello world -- from first thread!\n");
	siglongjmp(jbuf[1],1);
}



/*
"Black Box" code
*/

#ifdef __x86_64__
// code for 64 bit Intel arch


#define JB_SP 6
#define JB_PC 7

//A translation required when using an address of a variable
//Use this as a black box in your code.
void * translate_address(void * addr)
{
    void * ret;
    asm volatile("xor    %%fs:0x30,%0\n"
		"rol    $0x11,%0\n"
                 : "=g" (ret)
                 : "0" (addr));
    return ret;
}

#else
// code for 32 bit Intel arch

#define JB_SP 4
#define JB_PC 5 

//A translation required when using an address of a variable
//Use this as a black box in your code.
void * translate_address(void * addr)
{
    void * ret;
    asm volatile("xor    %%gs:0x18,%0\n"
		"rol    $0x9,%0\n"
                 : "=g" (ret)
                 : "0" (addr));
    return ret;
}

#endif




int createThread(void (*fPtr)(void))
{

	int n = numThreads;
	struct TCB threadNode;
	
	//initialize node elements
	createStack(&threadNode, STACK_SIZE);
	threadNode.pc = fPtr;
	threadNode.tid = numThreads;
	*(--threadNode.sp) = &switchThreads;

	threadArr[numThreads++] = threadNode;
	printf("created thread\n");

	//create the jmp environment
	sigsetjmp(jbuf[n],1);
	(jbuf[numThreads]->__jmpbuf)[JB_SP] = translate_address(threadNode.sp);
	(jbuf[numThreads]->__jmpbuf)[JB_PC] = translate_address(threadNode.pc);
	sigemptyset(&jbuf[0]->__saved_mask);  

	return n;

}



int main(void)
{
	int x = createThread(trialFunction);
	int y = sigsetjmp(jbuf[1],1);
	if (y == 0)
		siglongjmp(jbuf[x],1);
	printf("Hello from main\n");
}




