#include <stdio.h>

int arr[5];

void fillArr(void)
{
	for (int i = 0; i < 5; i++)
	{
		arr[i] = 5 - i;
	}
}

int main(void)
{
	fillArr();

	for (int i = 0; i < 5; i++)
	{
		printf("%d\n", arr[i]);
	}
}