#include <stdlib.h>
#include <errno.h>
#include "util_strconv_oal.h"

VOS_RET_E oal_strtol(const char *str, char **endptr, SINT32 base, SINT32 *val)
{
   VOS_RET_E ret=VOS_RET_SUCCESS;
   char *localEndPtr=NULL;

   errno = 0;  /* set to 0 so we can detect ERANGE */

   *val = strtol(str, &localEndPtr, base);

   if ((errno != 0) || (*localEndPtr != '\0'))
   {
      *val = 0;
      ret = VOS_RET_INVALID_ARGUMENTS;
   }

   if (endptr != NULL)
   {
      *endptr = localEndPtr;
   }

   return ret;
}


VOS_RET_E oal_strtoul(const char *str, char **endptr, SINT32 base, UINT32 *val)
{
   VOS_RET_E ret=VOS_RET_SUCCESS;
   char *localEndPtr=NULL;

   /*
    * Linux strtoul allows a minus sign in front of the number.
    * This seems wrong to me.  Specifically check for this and reject
    * such strings.
    */

   if (NULL == str)
   {
      vosLog_error("str is NULL and is invaild");
      return VOS_RET_INVALID_PARAM_VALUE;
   }
   
   while (isspace(*str))
   {
      str++;
   }
   if (*str == '-')
   {
      if (endptr)
      {
         *endptr = (char *) str;
      }
      *val = 0;
      return VOS_RET_INVALID_ARGUMENTS;
   }

   errno = 0;  /* set to 0 so we can detect ERANGE */

   *val = strtoul(str, &localEndPtr, base);

   if ((errno != 0) || (*localEndPtr != '\0'))
   {
      *val = 0;
      ret = VOS_RET_INVALID_ARGUMENTS;
   }

   if (endptr != NULL)
   {
      *endptr = localEndPtr;
   }

   return ret;
}


VOS_RET_E oal_strtol64(const char *str, char **endptr, SINT32 base, SINT64 *val)
{
   VOS_RET_E ret=VOS_RET_SUCCESS;
   char *localEndPtr=NULL;

   errno = 0;  /* set to 0 so we can detect ERANGE */

   *val = strtoll(str, &localEndPtr, base);

   if ((errno != 0) || (*localEndPtr != '\0'))
   {
      *val = 0;
      ret = VOS_RET_INVALID_ARGUMENTS;
   }

   if (endptr != NULL)
   {
      *endptr = localEndPtr;
   }

   return ret;
}


VOS_RET_E oal_strtoul64(const char *str, char **endptr, SINT32 base, UINT64 *val)
{
   VOS_RET_E ret=VOS_RET_SUCCESS;
   char *localEndPtr=NULL;

   /*
    * Linux strtoul allows a minus sign in front of the number.
    * This seems wrong to me.  Specifically check for this and reject
    * such strings.
    */
   while (isspace(*str))
   {
      str++;
   }
   if (*str == '-')
   {
      if (endptr)
      {
         *endptr = (char *) str;
      }
      *val = 0;
      return VOS_RET_INVALID_ARGUMENTS;
   }

   errno = 0;  /* set to 0 so we can detect ERANGE */

   *val = strtoull(str, &localEndPtr, base);

   if ((errno != 0) || (*localEndPtr != '\0'))
   {
      *val = 0;
      ret = VOS_RET_INVALID_ARGUMENTS;
   }

   if (endptr != NULL)
   {
      *endptr = localEndPtr;
   }

   return ret;
}


