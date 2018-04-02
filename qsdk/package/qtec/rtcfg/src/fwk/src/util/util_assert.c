#include "fwk.h"


void utilAst_assertFunc(const char *filename, UINT32 lineNumber, const char *exprString, SINT32 expr)
{

   if (expr == 0)
   {
      vosLog_error("assertion \"%s\" failed at %s:%d", exprString, filename, lineNumber);

#ifndef NDEBUG
      /* Send SIGABRT only if NDEBUG is not defined */
      prctl_signalProcess(getpid(), SIGABRT);
#endif
   }

}




