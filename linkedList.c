#include <stdio.h>
#include <stdlib.h>

struct listNode
{
	int x;
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



void createHead(int y, struct linkedList* list)
{
	list->head->x = y;
	list->head->next = list->tail;
	++list->size;
}

struct listNode * add(int y, struct linkedList* list)
{
	/*
	must initialize tail->next in order to use struct returned
	*/

	list->tail->x = y;
	list->tail->next = (struct listNode *)malloc(sizeof(struct listNode));

	return list->tail->next;

}

void addToEnd(int y, struct linkedList* list)
{
	if (!list->head->x)
	{
		createHead(y,list);
	}

	else
	{
		list->tail = add(y, list);
		++list->size;
	}

}

int pop(struct linkedList* list)
{
	/*
	extract necessary info from head then
	free to prevent memory leak
	*/

	struct listNode *temp = list->head;
	list->head = list->head->next;
	int y = temp->x;
	free(temp);

	--list->size;

	return y;
}

void traverse(struct linkedList* list)
{
	struct listNode *temp = list->head;
	while (temp->x)
	{
		printf("%d\n", temp->x);
		temp = temp->next;
	}


}

int popN(int n, struct linkedList* list)
{
	int i = 0;
	int ret;
	if (n-- > list->size)
	{
		printf("List only has %d elements, %d requested", list->size, n);
	}

	struct listNode *temp = list->head;
	struct listNode *prev = list->head;
	
	while (temp->x)
	{
		if (i == n)
		{
			printf("requested element found with element # %d \n", temp->x);
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



int main(void)
{
	struct linkedList myList; 
	myList = createList();

	addToEnd(1, &myList);
	addToEnd(5, &myList);
	addToEnd(3, &myList);

	printf("size %d\n", myList.size);

	
	traverse(&myList);


	//popN(2,&myList);
	//printf("size %d\n", myList.size);


}