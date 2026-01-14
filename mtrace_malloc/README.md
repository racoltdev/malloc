This library functions identically to mtrace(3), without using malloc hooks. Instead it is loaded in with LD_PRELOAD and calls libc malloc functions using dlsym().

### License Information:
  Parts of this library are modifications of the GNU C Library 2.42,</br>
  namely malloc's mtrace.c and mtrace-impl.c files. As such, the </br>
  mtrace_malloc library is licensed under LGPL 2.1. See the License</br>
  file distributed with this library to view the license.</br>
  
  This library is free software; you can redistribute it and/or</br>
  modify it under the terms of the GNU Lesser General Public</br>
  License as published by the Free Software Foundation; either</br>
  version 2.1 of the License, or (at your option) any later version.</br>
