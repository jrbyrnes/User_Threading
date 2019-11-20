/*
need to provide nicer API
createLinkedList 
	returns linkedList struct
		has head
		has tail
addToEnd(linkedList struct)
	functionality defined below
popFromFront(linkedList struct)
	functionality defined below
*/



#include <stdio.h>
#include <stdlib.h>

struct listNode
{
	int x;
	struct listNode *next;
};

struct listNode* head;
struct listNode* tail;



void createHead(int y)
{
	head->x = y;
	head->next = tail;
}

struct listNode * addToEnd(int y)
{
	/*
	must initialize tail->next in order to use struct returned
	*/
	
	tail->x = y;
	tail->next = (struct listNode *)malloc(sizeof(struct listNode));

	return tail->next;

}

int pop()
{
	/*
	extract necessary info from head then
	free to prevent memory leak
	*/

	struct listNode *temp = head;
	head = head->next;
	int y = temp->x;
	free(temp);

	return y;
}

void traverse()
{
	struct listNode *temp = head;
	while (temp->x)
	{
		printf("%d\n", temp->x);
		temp = temp->next;
	}

	free(temp);
}



int main(void)
{
	head = (struct listNode *)malloc(sizeof(struct listNode));
	tail = (struct listNode *)malloc(sizeof(struct listNode));

	createHead(1);
	tail = addToEnd(2);
	tail = addToEnd(3);

	traverse();

	int x = pop();
	printf("%d popped off list \n",x);
	printf("new list\n");
	traverse();

}
