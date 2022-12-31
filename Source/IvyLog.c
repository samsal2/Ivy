#include "IvyLog.h"

#include <stdarg.h>
#include <stdio.h>

void ivyLogImplementation(char const *functionName, char const *format, ...) {
  va_list args;
  va_start(args, format);
  printf("\033[33m%s\033[0m\n  ", functionName);
  vprintf(format, args);
  va_end(args);
}
