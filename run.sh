gcc -g3 -o main main.c
lib=dummy_malloc/libdummy_malloc.so
if [ $# -gt 0 ]; then
	lib=$1
fi

echo "= Start" > m.trace
LD_PRELOAD=$PWD/$lib MALLOC_TRACE=./m.trace ./main
