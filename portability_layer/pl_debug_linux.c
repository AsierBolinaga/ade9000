
#include "pl_debug.h"
#ifdef PL_DEBUG

#ifdef PL_LINUX

int pl_debug_printf_with_var_list_linux(const char *fmt_s, va_list _ap)
{
	int dbgResult = 0;
	
	dbgResult = printf(fmt_s, _ap);
	
	return dbgResult;

}

#endif /* PL_LINUX */
#endif /* PL_DEBUG */
