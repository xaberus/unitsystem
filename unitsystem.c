
#include "unitsystem.h"

#ifdef TEST
#include <stdarg.h>
#include <stdio.h>

void test_logger(void * data, char format[], ...) {
	va_list ap;
	char buff[4096];
	UNUSED_PARAM(data);

	va_start(ap, format);
	vsnprintf(buff, 4096, format, ap);
	va_end(ap);

	bt_log("%s", buff);
}


err_log_t * err_logger = test_logger;
void * err_logger_data = NULL;
#endif

