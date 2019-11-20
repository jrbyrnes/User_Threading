#include <stdio.h>

int main(void)
{
	int arr[10];

	for (int i = 0; i < 10; i++)
	{
		arr[i] = 0;
	}

	arr[-1] = -1;

	for (int i = 0; i < 10; i++)
	{
		printf("%d\n",arr[i]);
	}
}