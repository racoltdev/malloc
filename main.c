#include <malloc.h>
#include <stdio.h>

int main() {

	int* arr = 0;
	fprintf(stderr, "Hi!\n");
	arr = (int*)malloc(sizeof(int) * 2);
	fprintf(stderr, "alloc at %p\n", arr);
	free(arr);
	int* arr2 = (int*)calloc(2, sizeof(int));
	free(arr2);
}
