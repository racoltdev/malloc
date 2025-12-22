gcc -o main main.c
LD_PRELOAD=$PWD/dummmy_malloc/libdummy_malloc.so ./main
