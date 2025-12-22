This produces a library to replace malloc with dummy behavior. It must be linked with LD_PRELOAD. Any call to malloc will simply output "Malloc replaced" and exit the program.
