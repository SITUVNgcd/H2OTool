#include <stdio.h>
#include <stdarg.h>

void dprintf(const char *format, ...) {
#ifdef DEBUG
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	fflush(stdout);
	va_end(args);
#endif
}
