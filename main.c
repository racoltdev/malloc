//#include <dummy_malloc.h>
#include <malloc.h>
#include <stdio.h>

int main() {
	int* arr = 0;
	arr = (int*)malloc(sizeof(int) * 1);
	printf("alloc at %p\n", arr);
	free(arr);
}
