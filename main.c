#include <malloc.h>
#include <stdio.h>

int main() {
	int* arr = 0;
	arr = (int*)malloc(sizeof(int) * 2);
	printf("alloc at %p\n", arr);
	free(arr);
}
