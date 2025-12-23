#include <malloc.h>
#include <stdio.h>

int main() {

	int* arr = 0;
	fprintf(stderr, "Hi!\n");
	arr = (int*)malloc(sizeof(int) * 2);
	fprintf(stderr, "alloc at %p\n", arr);
}
