This library functions similarly to mtrace(3), without using malloc hooks. Instead it is loaded in with LD_PRELOAD and calls libc malloc functions using dlsym(). mptrace_malloc is made to collect trace information for multithreaded programs and reduce disk usage where possible. </br>
The output format is notably different in a few ways:
- The first line of the trace is not '= Start', it's just the start of the data
- Lines do not start with '@'
- Full file paths are not printed, just the base name.
- PIDs are printed with each line

### License Information:
  Parts of this library are modifications of the GNU C Library 2.42,</br>
  namely malloc's mtrace.c and mtrace-impl.c files. As such, the </br>
  mptrace_malloc library is licensed under LGPL 2.1. See the [LICENSE](LICENSE)</br>
  file distributed with this library to view the license.</br>

  This library is free software; you can redistribute it and/or</br>
  modify it under the terms of the GNU Lesser General Public</br>
  License as published by the Free Software Foundation; either</br>
  version 2.1 of the License, or (at your option) any later version.</br>
