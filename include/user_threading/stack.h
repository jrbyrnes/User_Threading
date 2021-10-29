#ifndef stack_h
#define stack_h

#include <iostream>
#include "node.h"

class stack {
            int size;
            node* head;
        
        public:
            stack ();
            int getSize();
            node* pop();
            void push(node*);
            //void push(int);


};

stack::stack() {
    size = 0;
    head = new node;
}

node* stack::pop() {
    if (size == 0) //exception
    {
        std::cout << "Error: attempt to pop from empty stack";
    }

    else
    {
        node* temp = head;
        head = head->next;
        --size;
        return temp;
    }
}

void stack::push(node* pushedNode) {
    ++size;
    node temp = node(pushedNode);

    temp.next = new node;
    *(temp.next) = *head; //copy assignment
    head->value = temp.value; 
    head->next = new node;
    *(head->next) = *(temp.next); //copy assignment -- does not seem to get invoked

    delete temp.next; //would be called implicitly
}

/*
void stack::push(int val)
{
    node newNode;
    newNode.value = val;
    newNode.next = new node;


    push(&newNode);

    delete newNode.next; //would be called impllcity

}*/

int stack::getSize()
{
    return size;
}

#endif
