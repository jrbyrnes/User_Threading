#include "singleton.h"
#include <stdio.h>

int main()
{
	singleton thisObj{4};
	thisObj.runThread();
}