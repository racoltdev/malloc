This library functions identically to mtrace(3), without using malloc hooks. Instead it is loaded in with LD_PRELOAD and calls libc malloc functions using dlsym().
