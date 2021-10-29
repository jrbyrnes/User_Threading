#ifndef NODE_H
#define NODE_H

#include "thread.h"

class node {
	public:
    	struct thread *value; 
    	node *next;
    	node ();
        node (thread* source);
    	node (node* node2) : value(node2->value), next(new node(*node2->next)) {} //copy constructor

    	node& operator= (const node& node2) {
    		value = node2.value;
    		next = node2.next; 
    		return *this;
    	}


    	node getNext() {return *next;} 
};

node::node() {
	value = 0;
	next = nullptr;
}

node::node(thread *source) {
    value = source; //copy assignment
    next = nullptr;
}


#endif
