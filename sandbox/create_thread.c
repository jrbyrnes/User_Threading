/*
Jeffrey Byrnes
myThread implements a
user-space thread library
*/

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>



#define STACK_SIZE 4096
#define MAX_THREADS 20
#define SECOND 1000000


/*
"Black Box" code
*/

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





/*
structures
*/

struct TCB
{
	char bp[STACK_SIZE];
	address_t sp, pc;
	int reg[4], state, tid;
};

/*
globals
*/

int numThreads = 0;
struct TCB threadArr[MAX_THREADS];
sigjmp_buf jbuf[MAX_THREADS];
struct TCB runThread;





/*
functions
*/

void switchThreads()
{
    
    int ret_val = sigsetjmp(jbuf[runThread.tid],1);
    printf("SWITCH: ret_val=%d\n", ret_val); 
    if (ret_val == 1) {
        return;
    }
    

    siglongjmp(jbuf[1 - runThread.tid],1);
}


int createThread(void (*fPtr)(void))
{

	int n = numThreads;
	struct TCB threadNode;
	
	//initialize node elements
	threadNode.sp = (address_t) threadNode.bp + (address_t) STACK_SIZE - sizeof(address_t);
	threadNode.pc = (address_t) fPtr;
	threadNode.tid = numThreads;
	//((void *) --threadNode.sp) = &switchThreads;

	threadArr[numThreads++] = threadNode;
	printf("created thread\n");

	//create the jmp environment
	sigsetjmp(jbuf[n],1);
	runThread = threadNode;
	(jbuf[numThreads]->__jmpbuf)[JB_SP] = translate_address(threadNode.sp);
	(jbuf[numThreads]->__jmpbuf)[JB_PC] = translate_address(threadNode.pc);
	sigemptyset(&jbuf[n]->__saved_mask);  

	return n;

}

void f( void )
{
    printf("b\n");
    int a=1;
    //fprintf(stderr,"%d\n",a); //XXX
    printf("c\n");
    int i=0;
    while(1) {
        ++i;
        printf("in f (%d)\n",i);
        if (i % 3 == 0) {
            printf("f: switching\n");
            switchThreads();
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
            switchThreads();
        }
        usleep(SECOND);
    }
}



int main(void)
{
	int x = createThread(f);
	int y = createThread(g);

	siglongjmp(jbuf[x],1);
	printf("Hello from main\n");
}




