#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <iostream>
#include "node.h"

class linkedList {
        int size;
        node *head;
        node *tail;
        int totalWeight;

    public:
        linkedList ();
        int getSize();
        int getTotalWeight();
        struct node* getHead();
        void addNode(node*);
        void addNode(int);
        void addNodeToEnd(node *);
        void traverse(node*);
        struct node* pop();
        struct node* popN(int);
};


linkedList::linkedList() {
    size = 0;
    head = new node;
    //tail = new node;

}

int linkedList::getSize() {
    return size;
}

int linkedList::getTotalWeight() {
	return totalWeight;
}

struct node* linkedList::getHead() {
    return head;
}

void linkedList::addNode(node* newNode) {
    if (!head->value) {
        head->value = newNode->value;
        head->next = new node;
        tail = head->next;
    }

    else {
        //std::cout << newNode->value;
        tail->value = newNode->value;
        tail->next = new node;
        tail = tail->next;
    }

    totalWeight += newNode->value->weight;
    ++size;
}

/*
void LinkedList::addNode(int val)
{
    node newNode;
    newNode.value = val;

    addNode(&newNode);
}*/

void linkedList::traverse(node* listPosition) {
    while (true) {
        listPosition->value->printStats();

        if (!listPosition->next->value)
            break;
        listPosition = listPosition->next;
    }
}


struct node* linkedList::pop() {
    node* temp = head;
    
    if (head->next->value) {
        head = head->next;
    }

    else {
        head = new node;
    }
    
    --size;
    totalWeight -= temp->value->weight; 

    return temp;
}

struct node* linkedList::popN(int lotteryPick) {
	int runningWeight = 0;
	if (lotteryPick <= head->value->weight) {
        return pop();
    }

    else {
    
        struct node returnNode;
        if (lotteryPick > totalWeight) {
            printf("Random value %d, total weight %d", lotteryPick, totalWeight);
        }
    
        struct node *temp = head; //looks like a bug
        struct node *prev = head;
        
    
        while (lotteryPick > (runningWeight + temp->value->weight)) {
            prev = temp;
            if (!temp->next->value->sp) {
                break;
            }
            temp = temp->next;
            runningWeight += temp->value->weight;
        }
    
    
        struct node *ret = temp;
    
        //node structuring & accounting
        prev->next = temp->next;
        --size;
        totalWeight -= ret->value->weight;
    
        return ret;
    }
}

#endif