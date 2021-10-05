#include <stdio.h>

#define SIZE 10
int main()
{
	int v[SIZE];
	for (int i = 0; i < SIZE; i++)
		printf("i: %d, v[i]=%d\n", i, v[i]);
	return 0;
}
