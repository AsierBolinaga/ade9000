
#include "absl_debug.h"
#ifdef ABSL_DEBUG

#ifdef ABSL_LINUX

int absl_debug_printf_with_var_list_linux(const char *fmt_s, va_list _ap)
{
	int dbgResult = 0;
	
	dbgResult = printf(fmt_s, _ap);
	
	return dbgResult;

}

#endif /* ABSL_LINUX */
#endif /* ABSL_DEBUG */
