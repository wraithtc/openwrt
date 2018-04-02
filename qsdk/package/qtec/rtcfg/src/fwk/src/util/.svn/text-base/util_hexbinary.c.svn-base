#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fwk.h"


VOS_RET_E util_binaryBufToHexString(const UINT8 *binaryBuf, UINT32 binaryBufLen, char **hexStr)
{
   UINT32 i, j;
   char buf[5] = {0};

   if (hexStr == NULL)
   {
      vosLog_error("hexStr buffer is NULL");
      return VOS_RET_INVALID_ARGUMENTS;
   }

   if (binaryBuf == NULL)
   {
      vosLog_error("binaryBuf buffer is NULL");
      return VOS_RET_INVALID_ARGUMENTS;
   }

   *hexStr = VOS_MALLOC_FLAGS((binaryBufLen*2)+1, ALLOC_ZEROIZE);
   if (*hexStr == NULL)
   {
      return VOS_RET_RESOURCE_EXCEEDED;
   }

   for (i=0, j=0; i < binaryBufLen; i++, j+=2)
   {
       UTIL_SNPRINTF(buf, sizeof(buf), "%04x", binaryBuf[i]);
       (*hexStr)[j] = buf[2];
       (*hexStr)[j + 1] = buf[3];
   }

   return VOS_RET_SUCCESS;
}


VOS_RET_E util_hexStringToBinaryBuf(const char *hexStr, UINT8 **binaryBuf, UINT32 *binaryBufLen)
{
   UINT32 len;
   UINT32 val;
   UINT32 i, j;
   char tmpbuf[3];
   VOS_RET_E ret;

   len = strlen(hexStr);
   if (len % 2 != 0)
   {
      vosLog_error("hexStr must be an even number of characters");
      return VOS_RET_INVALID_ARGUMENTS;
   }

   *binaryBuf = VOS_MALLOC_FLAGS(len/2, 0);
   if (*binaryBuf == NULL)
   {
      return VOS_RET_RESOURCE_EXCEEDED;
   }

   for (i=0, j=0; j < len; i++, j+=2)
   {
      tmpbuf[0] = hexStr[j];
      tmpbuf[1] = hexStr[j+1];
      tmpbuf[2] = 0;

      ret = util_strtoul(tmpbuf, NULL, 16, &val);
      if (ret != VOS_RET_SUCCESS)
      {
         VOS_FREE(*binaryBuf);
         *binaryBuf = NULL;
         return ret;
      }
      else
      {
         (*binaryBuf)[i] = (UINT8) val;
      }
   }

   /* if we get here, we were successful, set length */
   *binaryBufLen = len / 2;

   return ret;
}



