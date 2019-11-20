/*
This is basically pseudo
*/

void dispatch()
{
	struct TCB thread = pop(readyQueue);
	curr = thread;
	siglongjmp(jbuf[thread.tid],1);
}