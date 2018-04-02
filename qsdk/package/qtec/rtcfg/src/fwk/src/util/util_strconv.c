#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>     /* for isDigit, really should be in oal_strconv.c */
#include <sys/stat.h>  /* this really should be in oal_strconv.c */
#include <arpa/inet.h> /* for inet_aton */
#include <sys/time.h> /* for inet_aton */

#include "fwk.h"
#include "util_strconv_oal.h"


UBOOL8 util_isValidVpiVci(SINT32 vpi, SINT32 vci)
{
   if (vpi >= VPI_MIN && vpi <= VPI_MAX && vci >= VCI_MIN && vci <= VCI_MAX)
   {
      return TRUE;
   }
   
   vosLog_error("invalid vpi/vci %d/%d", vpi, vci);
   return FALSE;
}

VOS_RET_E util_atmVpiVciStrToNum(const char *vpiVciStr, SINT32 *vpi, SINT32 *vci)
{
   char *slash;
   char vpiStr[BUFLEN_256];
   char vciStr[BUFLEN_256];
   char *prefix;
   
   *vpi = *vci = -1;   
   if (vpiVciStr == NULL)
   {
      vosLog_error("vpiVciStr is NULL");
      return VOS_RET_INVALID_ARGUMENTS;
   }      

   strncpy(vpiStr, vpiVciStr, sizeof(vpiStr));

   if (strstr(vpiStr, DSL_LINK_DESTINATION_PREFIX_SVC))
   {
      vosLog_error("DesitinationAddress string %s with %s is not supported yet.", vpiStr, DSL_LINK_DESTINATION_PREFIX_SVC);
      return VOS_RET_INVALID_PARAM_VALUE;
   }

   if ((prefix = strstr(vpiStr, DSL_LINK_DESTINATION_PREFIX_PVC)) == NULL)
   {
      vosLog_error("Invalid DesitinationAddress string %s", vpiStr);
      return VOS_RET_INVALID_PARAM_VALUE;
   }
 
   /* skip the prefix */
#if 0
   prefix += sizeof(DSL_LINK_DESTINATION_PREFIX_PVC);
#endif
   prefix += strlen(DSL_LINK_DESTINATION_PREFIX_PVC);
   /* skip the blank if there is one */
   if (*prefix == ' ')
   {
      prefix += 1;
   }

   slash = (char *) strchr(prefix, '/');
   if (slash == NULL)
   {
      vosLog_error("vpiVciStr %s is invalid", vpiVciStr);
      return VOS_RET_INVALID_ARGUMENTS;
   }
   strncpy(vciStr, (slash + 1), sizeof(vciStr));
   *slash = '\0';       
   *vpi = atoi(prefix);
   *vci = atoi(vciStr);
   if (util_isValidVpiVci(*vpi, *vci) == FALSE)
   {
      return VOS_RET_INVALID_PARAM_VALUE;
   }     

   return VOS_RET_SUCCESS;
   
}


VOS_RET_E util_atmVpiVciNumToStr(const SINT32 vpi, const SINT32 vci, char *vpiVciStr, UINT32 vpiVciStrLen)
{
   if (vpiVciStr == NULL)
   {
      vosLog_error("vpiVciStr is NULL");
      return VOS_RET_INVALID_ARGUMENTS;
   }         
   if (util_isValidVpiVci(vpi, vci) == FALSE)
   {
      return VOS_RET_INVALID_PARAM_VALUE;
   }     

   UTIL_SNPRINTF(vpiVciStr, vpiVciStrLen, "%s %d/%d", DSL_LINK_DESTINATION_PREFIX_PVC, vpi, vci);

   return VOS_RET_SUCCESS;
   
}


VOS_RET_E util_macStrToNum(const char *macStr, UINT8 *macNum) 
{
   char *token = NULL;
   char *last = NULL;
   char *buf;
   SINT32 i;
   
   if (macNum == NULL || macStr == NULL) 
   {
      vosLog_error("Invalid macNum/macStr %p/%p", macNum, macStr);
      return VOS_RET_INVALID_ARGUMENTS;
   }    
   
   if (util_isValidMacAddress(macStr) == FALSE)
   {
      return VOS_RET_INVALID_PARAM_VALUE;
   }   
   
   if ((buf = (char *) VOS_MALLOC_FLAGS(MAC_STR_LEN+1, ALLOC_ZEROIZE)) == NULL)
   {
      vosLog_error("alloc of %d bytes failed", MAC_STR_LEN+1);
      return VOS_RET_RESOURCE_EXCEEDED;
   }

   /* need to copy since strtok_r updates string */
   UTIL_STRNCPY(buf, macStr, MAC_STR_LEN + 1);

   /* Mac address has the following format
    * xx:xx:xx:xx:xx:xx where x is hex number 
    */
   token = strtok_r(buf, ":", &last);
   macNum[0] = (UINT8) strtol(token, (char **)NULL, 16);
   for (i = 1; i < MAC_ADDR_LEN; i++) 
   {
      token = strtok_r(NULL, ":", &last);
      macNum[i] = (UINT8) strtol(token, (char **)NULL, 16);
   }

   VOS_FREE(buf);

   return VOS_RET_SUCCESS;
   
}

VOS_RET_E util_macNumToStr(const UINT8 *macNum, char *macStr, UINT32 macStrLen) 
{
   if (macNum == NULL || macStr == NULL) 
   {
      vosLog_error("Invalid macNum/macStr %p/%p", macNum, macStr);
      return VOS_RET_INVALID_ARGUMENTS;
   }  

   UTIL_SNPRINTF(macStr, macStrLen, "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x",
           (UINT8) macNum[0], (UINT8) macNum[1], (UINT8) macNum[2],
           (UINT8) macNum[3], (UINT8) macNum[4], (UINT8) macNum[5]);

   return VOS_RET_SUCCESS;
}


VOS_RET_E util_strtol(const char *str, char **endptr, SINT32 base, SINT32 *val)
{
   return(oal_strtol(str, endptr, base, val));
}


VOS_RET_E util_strtoul(const char *str, char **endptr, SINT32 base, UINT32 *val)
{
   return(oal_strtoul(str, endptr, base, val));
}


VOS_RET_E util_strtol64(const char *str, char **endptr, SINT32 base, SINT64 *val)
{
   return(oal_strtol64(str, endptr, base, val));
}


VOS_RET_E util_strtoul64(const char *str, char **endptr, SINT32 base, UINT64 *val)
{
   return(oal_strtoul64(str, endptr, base, val));
}


void util_strToLower(char *string)
{
    char *ptr = string;
    for (ptr = string; *ptr; ptr++)
    {
        *ptr = tolower(*ptr);
    }
}


char *util_strDupLower(char *string)
{
    char *ptr = NULL;

    ptr = VOS_STRDUP(string);
    util_strToLower(ptr);

    return ptr;
}


void util_strTrimL(char *string)
{
    int i;
    char *ptr;

    if (NULL == string)
    {
        return;
    }

    for (ptr = string; *ptr && isspace(*ptr); ptr++) ;

    if (ptr > string)
    {
        for (i = 0; (string[i] = *ptr++); i++) ;
    }
}


void util_strTrimR(char *string)
{
    char *ptr;

    if (NULL == string)
    {
        return;
    }

    for (ptr = string + strlen(string) - 1; ptr >= string && isspace(*ptr); ptr--) ;

    ptr[1] = '\0';
}


void util_strTrim(char *string)
{
    util_strTrimL(string);
    util_strTrimR(string);
}


VOS_RET_E util_parseUrl(const char *url, UrlProto *proto, char **addr, UINT16 *port, char **path)
{
   int n = 0;
   char *p = NULL;
   char protocol[BUFLEN_16];
   char host[BUFLEN_1024];
   char uri[BUFLEN_1024];

   if (url == NULL)
   {
      vosLog_debug("url is NULL");
      return VOS_RET_INVALID_ARGUMENTS;
   }

  *port = 0;
   protocol[0] = host[0]  = uri[0] = '\0';

   /* proto */
   p = (char *) url;
   if ((p = strchr(url, ':')) == NULL) 
   {
      return VOS_RET_INVALID_ARGUMENTS;
   }
   n = p - url;
   strncpy(protocol, url, n);
   protocol[n] = '\0';

   if (!strcmp(protocol, "http"))
   {
      *proto = URL_PROTO_HTTP;
   }
   else if (!strcmp(protocol, "https"))
   {
      *proto = URL_PROTO_HTTPS;
   }
   else if (!strcmp(protocol, "ftp"))
   {
      *proto = URL_PROTO_FTP;
   }
   else if (!strcmp(protocol, "tftp"))
   {
      *proto = URL_PROTO_TFTP;
   }
   else
   {
      vosLog_error("unrecognized proto in URL %s", url);
      return VOS_RET_INVALID_ARGUMENTS;
   }

   /* skip "://" */
   if (*p++ != ':') return VOS_RET_INVALID_ARGUMENTS;
   if (*p++ != '/') return VOS_RET_INVALID_ARGUMENTS;
   if (*p++ != '/') return VOS_RET_INVALID_ARGUMENTS;

   /* host */
   {
      char *pHost = host;
      char endChar1 = ':';  // by default, host field ends if a colon is seen
      char endChar2 = '/';  // by default, host field ends if a / is seen

      if (1)
      {
         if (*p && *p == '[')
         {
          /*
           * Square brackets are used to surround IPv6 addresses in : notation.
           * So if a [ is detected, then keep scanning until the end bracket
           * is seen.
           */
          endChar1 = ']';
          endChar2 = ']';
          p++;  // advance past the [
         }
      }

      while (*p && *p != endChar1 && *p != endChar2)
      {
         *pHost++ = *p++;
      }
      *pHost = '\0';

      if (1)
      {
      
         if (endChar1 == ']')
         {
             // if endChar is ], then it must be found
             if (*p != endChar1)
             {
                return VOS_RET_INVALID_ARGUMENTS;
             }
             else
             {
                p++;  // advance past the ]
             }
         }
      }
   }
   if (strlen(host) != 0)
   {
      *addr = VOS_STRDUP(host);
   }
   else
   {
      vosLog_error("unrecognized host in URL %s", url);
      return VOS_RET_INVALID_ARGUMENTS;
   }

   /* end */
   if (*p == '\0') 
   {
      *path = VOS_STRDUP("/");
       return VOS_RET_SUCCESS;
   }

   /* port */
   if (*p == ':') 
   {
      char buf[BUFLEN_16];
      char *pBuf = buf;

      p++;
      while (isdigit(*p)) 
      {
         *pBuf++ = *p++;
      }
      *pBuf = '\0';
      if (strlen(buf) == 0)
      {
         VOS_MEM_FREE_BUF_AND_NULL_PTR(*addr);
         vosLog_error("unrecognized port in URL %s", url);
         return VOS_RET_INVALID_ARGUMENTS;
      }
      *port = atoi(buf);
   }
  
   /* path */
   if (*p == '/') 
   {
      char *pUri = uri;

      while ((*pUri++ = *p++));
      *path = VOS_STRDUP(uri);  
   }
   else
   {
      *path = VOS_STRDUP("/");
   }

   return VOS_RET_SUCCESS;
}

VOS_RET_E util_getBaseDir(char *pathBuf, UINT32 pathBufLen)
{
#ifdef DESKTOP_LINUX
   UINT32 rc;
   char pwd[BUFLEN_1024]={0};
   UINT32 pwdLen = sizeof(pwd);
   char *str;
   char *envDir;
   struct stat statbuf;

   getcwd(pwd, pwdLen);
   if (strlen(pwd) == pwdLen - 1)
   {
      return VOS_RET_INTERNAL_ERROR;
   }

   vosLog_debug("pwd:%s", pwd);
   str = strstr(pwd, "usr/S304/bin");
   if (str == NULL)
   {
      str = strstr(pwd, "unittests");
   }

   if (str != NULL)
   {
      /*
       * OK, we are running from under userspace.
       * null terminate the string right before userspace and that
       * should give us the basedir.
       */
      str--;
      *str = 0;

      rc = snprintf(pathBuf, pathBufLen, "%s", pwd);
      vosLog_debug("pathBuf:%s", pathBuf);
   }
   else
   {
      /* try to figure out location of CommEngine from env var */
      if ((envDir = getenv("VOS_BASE_DIR")) != NULL)
      {
         snprintf(pwd, sizeof(pwd), "%s/unittests", envDir);
         if ((rc = stat(pwd, &statbuf)) == 0)
         {
            /* env var is good, use it. */
            rc = snprintf(pathBuf, pathBufLen, "%s", envDir);
         }
         else
         {
            /* VOS_BASE_DIR is set, but points to bad location */
            return VOS_RET_INVALID_ARGUMENTS;
         }
      }
      else
      {
         /* not running from under CommEngine and also no VOS_BASE_DIR */
         return VOS_RET_INVALID_ARGUMENTS;
      }
   }

   if (rc >= pathBufLen)
   {
      return VOS_RET_RESOURCE_EXCEEDED;
   }
#else
   pathBuf[0] = '\0';
#endif /* DESKTOP_LINUX */

   return VOS_RET_SUCCESS;
}

VOS_RET_E util_parseDNS(const char *inDnsServers, char *outDnsPrimary, UINT32 outDnsPrimaryLen, char *outDnsSecondary, UINT32 outDnsSecondaryLen, UBOOL8 isIPv4)
{
   VOS_RET_E ret = VOS_RET_SUCCESS;
   char *tmpBuf;
   char *separator;
   char *separator1;
   UINT32 len;

   if (inDnsServers == NULL)
   {
      return VOS_RET_INVALID_ARGUMENTS;
   }      
   

   vosLog_debug("entered: DDNSservers=>%s<=, isIPv4<%d>", inDnsServers, isIPv4);

   if ( isIPv4 )
   {
      if (outDnsPrimary)
      {
         UTIL_STRNCPY(outDnsPrimary, "0.0.0.0", outDnsPrimaryLen);
      }
   
      if (outDnsSecondary)
      {
         UTIL_STRNCPY(outDnsSecondary, "0.0.0.0", outDnsSecondaryLen);
      }
   }

   len = strlen(inDnsServers);

   if ((tmpBuf = VOS_MALLOC_FLAGS(len+1, 0)) == NULL)
   {
      vosLog_error("alloc of %d bytes failed", len);
      ret = VOS_RET_INTERNAL_ERROR;
   }
   else
   {
      SINT32 af = isIPv4?AF_INET:AF_INET6;

      UTIL_SNPRINTF(tmpBuf, len + 1, "%s", inDnsServers);
      separator = strstr(tmpBuf, ",");
      if (separator != NULL)
      {
         /* break the string into two strings */
         *separator = 0;
         separator++;
         while ((isspace(*separator)) && (*separator != 0))
         {
            /* skip white space after comma */
            separator++;
         }
         /* There might be 3rd DNS server, truncate it. */
         separator1 = strstr(separator, ",");
         if (separator1 != NULL)
          *separator1 = 0;

         if (outDnsSecondary != NULL)
         {
            if ( util_isValidIpAddress(af, separator))
            {
               UTIL_STRNCPY(outDnsSecondary, separator, outDnsSecondaryLen);
            }
            vosLog_debug("dnsSecondary=%s", outDnsSecondary);
         }
      }

      if (outDnsPrimary != NULL)
      {
         if (util_isValidIpAddress(af, tmpBuf))
         {
            UTIL_STRNCPY(outDnsPrimary, tmpBuf, outDnsPrimaryLen);
         }
         vosLog_debug("dnsPrimary=%s", outDnsPrimary);
      }

      VOS_FREE(tmpBuf);
   }

   return ret;
   
}


#if 0    /* zhangdingli */
SINT32 util_syslogModeToNum(const char *modeStr)
{
   SINT32 mode=1;

   /*
    * These values are hard coded in httpd/html/logconfig.html.
    * Any changes to these values must also be reflected in that file.
    */
   if (!strcmp(modeStr, MDMVS_LOCAL_BUFFER))
   {
      mode = 1;
   }
   else if (!strcmp(modeStr, MDMVS_REMOTE))
   {
      mode = 2;
   }
   else if (!strcmp(modeStr, MDMVS_LOCAL_BUFFER_AND_REMOTE))
   {
      mode = 3;
   }
   else 
   {
      vosLog_error("unsupported mode string %s, default to mode=%d", modeStr, mode);
   }

   /*
    * The data model also specifies LOCAL_FILE and LOCAL_FILE_AND_REMOTE,
    * but its not clear if syslogd actually supports local file mode.
    */

   return mode;
}


char * util_numToSyslogModeString(SINT32 mode)
{
   char *modeString = MDMVS_LOCAL_BUFFER;

   /*
    * These values are hard coded in httpd/html/logconfig.html.
    * Any changes to these values must also be reflected in that file.
    */
   switch(mode)
   {
   case 1:
      modeString = MDMVS_LOCAL_BUFFER;
      break;

   case 2:
      modeString = MDMVS_REMOTE;
      break;

   case 3:
      modeString = MDMVS_LOCAL_BUFFER_AND_REMOTE;
      break;

   default:
      vosLog_error("unsupported mode %d, default to %s", mode, modeString);
      break;
   }

   /*
    * The data model also specifies LOCAL_FILE and LOCAL_FILE_AND_REMOTE,
    * but its not clear if syslogd actually supports local file mode.
    */

   return modeString;
}


UBOOL8 util_isValidSyslogMode(const char * modeStr)
{
   UINT32 mode;

   if (util_strtoul(modeStr, NULL, 10, &mode) != VOS_RET_SUCCESS) 
   {
      return FALSE;
   }

   return ((mode >= 1) && (mode <= 3));
}


SINT32 util_syslogLevelToNum(const char *levelStr)
{
   SINT32 level=3; /* default all levels to error */

   /*
    * These values are from /usr/include/sys/sysvos_log.h.
    */
   if (!strcmp(levelStr, MDMVS_EMERGENCY))
   {
      level = 0;
   }
   else if (!strcmp(levelStr, MDMVS_ALERT))
   {
      level = 1;
   }
   else if (!strcmp(levelStr, MDMVS_CRITICAL))
   {
      level = 2;
   }
   else if (!strcmp(levelStr, MDMVS_ERROR))
   {
      level = 3;
   }
   else if (!strcmp(levelStr, MDMVS_WARNING))
   {
      level = 4;
   }
   else if (!strcmp(levelStr, MDMVS_NOTICE))
   {
      level = 5;
   }
   else if (!strcmp(levelStr, MDMVS_INFORMATIONAL))
   {
      level = 6;
   }
   else if (!strcmp(levelStr, MDMVS_DEBUG))
   {
      level = 7;
   }
   else 
   {
      vosLog_error("unsupported level string %s, default to level=%d", levelStr, level);
   }

   return level;
}


char * util_numToSyslogLevelString(SINT32 level)
{
   char *levelString = MDMVS_ERROR;

   /*
    * These values come from /usr/include/sys/sysvos_log.h.
    */
   switch(level)
   {
   case 0:
      levelString = MDMVS_EMERGENCY;
      break;

   case 1:
      levelString = MDMVS_ALERT;
      break;

   case 2:
      levelString = MDMVS_CRITICAL;
      break;

   case 3:
      levelString = MDMVS_ERROR;
      break;

   case 4:
      levelString = MDMVS_WARNING;
      break;

   case 5:
      levelString = MDMVS_NOTICE;
      break;

   case 6:
      levelString = MDMVS_INFORMATIONAL;
      break;

   case 7:
      levelString = MDMVS_DEBUG;
      break;

   default:
      vosLog_error("unsupported level %d, default to %s", level, levelString);
      break;
   }

   return levelString;
}


UBOOL8 util_isValidSyslogLevel(const char *levelStr)
{
   UINT32 level;

   if (util_strtoul(levelStr, NULL, 10, &level) != VOS_RET_SUCCESS) 
   {
      return FALSE;
   }

   return (level <= 7);
}


UBOOL8 util_isValidSyslogLevelString(const char *levelStr)
{
   if ((!strcmp(levelStr, MDMVS_EMERGENCY)) ||
       (!strcmp(levelStr, MDMVS_ALERT)) ||
       (!strcmp(levelStr, MDMVS_CRITICAL)) ||
       (!strcmp(levelStr, MDMVS_ERROR)) ||
       (!strcmp(levelStr, MDMVS_WARNING)) ||
       (!strcmp(levelStr, MDMVS_NOTICE)) ||
       (!strcmp(levelStr, MDMVS_INFORMATIONAL)) ||
       (!strcmp(levelStr, MDMVS_DEBUG)))
   {
      return TRUE;
   }
   else
   {
      return FALSE;
   }
}


SINT32 util_pppAuthToNum(const char *authStr)
{
   SINT32 authNum = PPP_AUTH_METHOD_AUTO;  /* default is auto  */

   if (!strcmp(authStr, MDMVS_AUTO_AUTH))
   {
      authNum = PPP_AUTH_METHOD_AUTO;
   }
   else if (!strcmp(authStr, MDMVS_PAP))
   {
      authNum = PPP_AUTH_METHOD_PAP;
   }
   else if (!strcmp(authStr, MDMVS_CHAP))
   {
       authNum = PPP_AUTH_METHOD_CHAP;
   }
   else if (!strcmp(authStr, MDMVS_MS_CHAP))
   {
         authNum = PPP_AUTH_METHOD_MSCHAP;
   }
   else 
   {
      vosLog_error("unsupported auth string %s, default to auto=%d", authStr, authNum);
   }

   return authNum;
   
}


char * util_numToPppAuthString(SINT32 authNum)
{
   char *authStr = MDMVS_AUTO_AUTH;   /* default to auto */

   switch(authNum)
   {
   case PPP_AUTH_METHOD_AUTO:
      authStr = MDMVS_AUTO_AUTH;
      break;

   case PPP_AUTH_METHOD_PAP:
      authStr = MDMVS_PAP;
      break;

   case PPP_AUTH_METHOD_CHAP:
      authStr = MDMVS_CHAP;
      break;

   case PPP_AUTH_METHOD_MSCHAP:
      authStr = MDMVS_MS_CHAP; 
      break;

   default:
      vosLog_error("unsupported authNum %d, default to %s", authNum, authStr);
      break;
   }

   return authStr;
   
}


#endif    /* zhangdingli */


#define MDMVS_AUTO_AUTH  "AUTO_AUTH"
#define MDMVS_PAP  "PAP"
#define MDMVS_CHAP  "CHAP"
#define MDMVS_MS_CHAP  "MS-CHAP"


SINT32 util_pppAuthToNum(const char *authStr)
{
   SINT32 authNum = PPP_AUTH_METHOD_AUTO;  /* default is auto  */

   if (!strcmp(authStr, MDMVS_AUTO_AUTH))
   {
      authNum = PPP_AUTH_METHOD_AUTO;
   }
   else if (!strcmp(authStr, MDMVS_PAP))
   {
      authNum = PPP_AUTH_METHOD_PAP;
   }
   else if (!strcmp(authStr, MDMVS_CHAP))
   {
       authNum = PPP_AUTH_METHOD_CHAP;
   }
   else if (!strcmp(authStr, MDMVS_MS_CHAP))
   {
         authNum = PPP_AUTH_METHOD_MSCHAP;
   }
   else 
   {
      vosLog_error("unsupported auth string %s, default to auto=%d", authStr, authNum);
   }
   vosLog_debug("authNum = %d",authNum);
   return authNum;
   
}


UBOOL8 util_isValidIpAddress(SINT32 af, const char* address)
{
   if ( IS_EMPTY_STRING(address) ) return FALSE;

   if (1 && af == AF_INET6)
   {
      struct in6_addr in6Addr;
      UINT32 plen;
      char   addr[UTIL_IPADDR_LENGTH];

      if (util_parsePrefixAddress(address, addr, sizeof(addr), &plen) != VOS_RET_SUCCESS)
      {
         vosLog_debug("Invalid ipv6 address=%s", address);
         return FALSE;
      }

      if (inet_pton(AF_INET6, addr, &in6Addr) <= 0)
      {
         vosLog_debug("Invalid ipv6 address=%s", address);
         return FALSE;
      }

      return TRUE;
   }
   else
   {
      if (af == AF_INET)
      {
         return util_isValidIpv4Address(address);
      }
      else
      {
         return FALSE;
      }
   }
}  /* End of util_isValidIpAddress() */

UBOOL8 util_isValidIpv4Address(const char* input)
{
   UBOOL8 ret = TRUE;
   char *token = NULL;
   char *last = NULL;
   char buf[BUFLEN_16];
   UINT32 i, num;

   if (input == NULL || strlen(input) < 7 || strlen(input) > 15)
   {
      return FALSE;
   }

   /* need to copy since strtok_r updates string */
   UTIL_STRNCPY(buf, input, sizeof(buf));

   /* IP address has the following format
      xxx.xxx.xxx.xxx where x is decimal number */
   token = strtok_r(buf, ".", &last);
   if ((util_strtoul(token, NULL, 10, &num) != VOS_RET_SUCCESS) ||
       (num > 255))
   {
      ret = FALSE;
   }
   else
   {
      for ( i = 0; i < 3; i++ )
      {
         token = strtok_r(NULL, ".", &last);
         
         if ((util_strtoul(token, NULL, 10, &num) != VOS_RET_SUCCESS) || (num > 255))
         {
            ret = FALSE;
            break;
         }
      }
   }

   return ret;
}



UBOOL8 util_isValidMacAddress(const char* input)
{
   UBOOL8 ret =  TRUE;
   char *token = NULL;
   char *last = NULL;
   char buf[BUFLEN_32];
   UINT32 i, num;

   if (input == NULL || strlen(input) != MAC_STR_LEN)
   {
      return FALSE;
   }

   /* need to copy since strtok_r updates string */
   UTIL_STRNCPY(buf, input, sizeof(buf));

   /* Mac address has the following format
       xx:xx:xx:xx:xx:xx where x is hex number */
   token = strtok_r(buf, ":", &last);
   if ((strlen(token) != 2) ||
       (util_strtoul(token, NULL, 16, &num) != VOS_RET_SUCCESS))
   {
      ret = FALSE;
   }
   else
   {
      for ( i = 0; i < 5; i++ )
      {
         token = strtok_r(NULL, ":", &last);
         if ((strlen(token) != 2) ||
             (util_strtoul(token, NULL, 16, &num) != VOS_RET_SUCCESS))
         {
            ret = FALSE;
            break;
         }
      }
   }

   return ret;
}


UBOOL8 util_isValidPortNumber(const char * portNumberStr)
{
   UINT32 portNum;

   if (util_strtoul(portNumberStr, NULL, 10, &portNum) != VOS_RET_SUCCESS) 
   {
      return FALSE;
   }

   return (portNum < (64 * 1024));
}


int safe_strncmpIgnoreCase(const char *s1, const char *s2, SINT32 n)
{
    if (!s1 && !s2)
        return 0;
    if (!s1)
        return -1;
    if (!s2)
        return 1;
    if(n <= 0)
        return 0;

    while(*s1 && *s2 && n >0)
    {
        int diff = tolower(*s1) - tolower(*s2);
        if(diff)
            return diff;
        s1++;
        s2++;
        n--;
    }
    
    if(n == 0)
    {
        return 0;
    }
    else
    {
        return *s1 ? 1: *s2? -1:0;
    }

}

int safe_strcmpIgnoreCase(const char *s1,const char *s2)
{
    if (!s1 && !s2)
        return 0;
    if (!s1)
        return -1;
    if (!s2)
        return 1;

    while (*s1 && *s2) {
        int diff = tolower(*s1) - tolower(*s2);
        if(diff)
            return diff;
        s1++;
        s2++;
    }

    return *s1 ? 1 : *s2 ? -1 : 0;
}

SINT32 util_strcmp(const char *s1, const char *s2) 
{
   const char emptyStr = '\0';

   if (s1 == NULL)
   {
      s1 = &emptyStr;
   }
   if (s2 == NULL)
   {
      s2 = &emptyStr;
   }

   return strcmp(s1, s2);
}


SINT32 util_strcasecmp(const char *s1, const char *s2) 
{
   const char emptyStr = '\0';

   if (s1 == NULL)
   {
      s1 = &emptyStr;
   }
   if (s2 == NULL)
   {
      s2 = &emptyStr;
   }

   return strcasecmp(s1, s2);
}


SINT32 util_strncmp(const char *s1, const char *s2, SINT32 n) 
{
   const char emptyStr = '\0';

   if (s1 == NULL)
   {
      s1 = &emptyStr;
   }
   if (s2 == NULL)
   {
      s2 = &emptyStr;
   }

   return strncmp(s1, s2, n);
}


SINT32 util_strncasecmp(const char *s1, const char *s2, SINT32 n) 
{
   const char emptyStr = '\0';

   if (s1 == NULL)
   {
      s1 = &emptyStr;
   }
   if (s2 == NULL)
   {
      s2 = &emptyStr;
   }

   return strncasecmp(s1, s2, n);
}


char *util_strstr(const char *s1, const char *s2) 
{
   const char emptyStr = '\0';

   if (s1 == NULL)
   {
      s1 = &emptyStr;
   }
   if (s2 == NULL)
   {
      s2 = &emptyStr;
   }

   return strstr(s1, s2);
}

char *util_strncpy(const char *func, UINT32 line, char *dest, const char *src, SINT32 dlen)
{
   if ((src == NULL) || (dest == NULL) || dlen <= 0)
   {
      vosLog_notice("%s:%u:null pointer reference src =%p ,dest =%p, dlen = %d", func, line, src, dest, dlen);
      return dest;
   }

   if (strlen(src) + 1 >= dlen)
   {
      vosLog_notice("%s:%u:truncating:src string length > dest buffer", func, line);
      strncpy(dest,src,dlen-1);
      dest[dlen-1] ='\0';
   }
   else
   {
      strncpy(dest,src, dlen);
   }

   return dest;
} 

char *util_strncat(const char *func, UINT32 line, char *dest, const char *src, SINT32 dlen)
{
   SINT32 n = 0;

   if ((src == NULL) || (dest == NULL) || dlen <= 0)
   {
      vosLog_error("%s:%u:null pointer reference src =%p ,dest =%p, dlen = %d", func, line, src, dest, dlen);
      return dest;
   }

   n = dlen - strlen(dest);
   if (n <= 0)
   {
      vosLog_error("%s:%u:dlen < util_strlen(dest)", func, line);
      return dest;
   }

   if (strlen(src) + 1 > n)
   {
      vosLog_notice("%s:%u:truncating:src string length > dest buffer", func, line);
      strncat(dest, src, n - 1);
      dest[dlen - 1] ='\0';
   }
   else
   {
      strncat(dest, src, n);
   }

   return dest;
}

SINT32 util_strlen(const char *src)
{
   const char emptyStr = '\0';
   
   if(src == NULL)
   {
      src = &emptyStr;
   }	

   return strlen(src);
} 


SINT32 util_snprintf(const char *func, UINT32 line, char *str, UINT32 size, const char *format, ...)
{
    SINT32 ret = 0;
    va_list paraList;

    if (NULL == str || NULL == format)
    {
        vosLog_error("%s:%u:null pointer reference str =%p, format =%p", func, line, str, format);
        return 0;
    }

    va_start(paraList, format);
    ret = vsnprintf(str, size, format, paraList);
    va_end(paraList);

    return ret;
}


UBOOL8 util_isSubOptionPresent(const char *fullOptionString, const char *subOption)
{
   const char *startChar, *currChar;
   UINT32 len=0;
   UBOOL8 found=FALSE;

   vosLog_debug("look for subOption %s in fullOptionString=%s", subOption, fullOptionString);

   if (fullOptionString == NULL || subOption == NULL)
   {
      return FALSE;
   }

   startChar = fullOptionString;
   currChar = startChar;

   while (!found && *currChar != '\0')
   {
      /* get to the end of the current subOption */
      while (*currChar != ' ' && *currChar != ',' && *currChar != '\0')
      {
         currChar++;
         len++;
      }

      /* compare the current subOption with the subOption that was specified */
      if ((len == strlen(subOption)) &&
          (0 == strncmp(subOption, startChar, len)))
      {
         found = TRUE;
      }

      /* advance to the start of the next subOption */
      if (*currChar != '\0')
      {
         while (*currChar == ' ' || *currChar == ',')
         {
            currChar++;
         }

         len = 0;
         startChar = currChar;
      }
   }

   vosLog_debug("found=%d", found);
   return found;
}


#if 0    /* zhangdingli */
void util_getWanProtocolName(UINT8 protocol, char *name, UINT32 nameLen) 
{
    if ( name == NULL ) 
      return;

    name[0] = '\0';
       
    switch ( protocol ) 
    {
        case CMC_WAN_TYPE_PPPOE:
            UTIL_STRNCPY(name, "PPPoE", nameLen);
            break;
        case CMC_WAN_TYPE_PPPOA:
            UTIL_STRNCPY(name, "PPPoA", nameLen);
            break;
        case CMC_WAN_TYPE_DYNAMIC_IPOE:
        case CMC_WAN_TYPE_STATIC_IPOE:
            UTIL_STRNCPY(name, "IPoE", nameLen);
            break;
        case CMC_WAN_TYPE_IPOA:
            UTIL_STRNCPY(name, "IPoA", nameLen);
            break;
        case CMC_WAN_TYPE_BRIDGE:
            UTIL_STRNCPY(name, "Bridge", nameLen);
            break;
        case CMC_WAN_TYPE_DYNAMIC_ETHERNET_IP:
            UTIL_STRNCPY(name, "IPoW", nameLen);
            break;
        default:
            UTIL_STRNCPY(name, "Not Applicable", nameLen);
            break;
    }
}


char *util_getAggregateStringFromDhcpVendorIds(const char *vendorIds)
{
   char *aggregateString;
   const char *vendorId;
   UINT32 i, count=0;

   if (vendorIds == NULL)
   {
      return NULL;
   }

   aggregateString = VOS_MALLOC_FLAGS(MAX_PORTMAPPING_DHCP_VENDOR_IDS * (DHCP_VENDOR_ID_LEN + 1), ALLOC_ZEROIZE);
   if (aggregateString == NULL)
   {
      vosLog_error("allocation of aggregate string failed");
      return NULL;
   }

   for (i=0; i < MAX_PORTMAPPING_DHCP_VENDOR_IDS; i++)
   {
      vendorId = &(vendorIds[i * (DHCP_VENDOR_ID_LEN + 1)]);
      if (*vendorId != '\0')
      {
         if (count > 0)
         {
            strcat(aggregateString, ",");
         }
         /* strncat writes at most DHCP_VENDOR_ID_LEN+1 bytes, which includes the trailing NULL */
         strncat(aggregateString, vendorId, DHCP_VENDOR_ID_LEN);
        
         count++;
      }
   }

   return aggregateString;
}


char *util_getDhcpVendorIdsFromAggregateString(const char *aggregateString)
{
   char *vendorIds, *vendorId, *ptr, *savePtr=NULL;
   char *copy;
   UINT32 count=0;

   if (aggregateString == NULL)
   {
      return NULL;
   }

   vendorIds = VOS_MALLOC_FLAGS(MAX_PORTMAPPING_DHCP_VENDOR_IDS * (DHCP_VENDOR_ID_LEN + 1), ALLOC_ZEROIZE);
   if (vendorIds == NULL)
   {
      vosLog_error("allocation of vendorIds buffer failed");
      return NULL;
   }

   copy = VOS_STRDUP(aggregateString);
   ptr = strtok_r(copy, ",", &savePtr);
   while ((ptr != NULL) && (count < MAX_PORTMAPPING_DHCP_VENDOR_IDS))
   {
      vendorId = &(vendorIds[count * (DHCP_VENDOR_ID_LEN + 1)]);
      /*
       * copy at most DHCP_VENDOR_ID_LEN bytes.  Since each chunk in the linear
       * buffer is DHCP_VENDOR_ID_LEN+1 bytes long and initialized to 0,
       * we are guaranteed that each vendor id is null terminated.
       */
      strncpy(vendorId, ptr, DHCP_VENDOR_ID_LEN);
      count++;

      ptr = strtok_r(NULL, ",", &savePtr);
   }

   VOS_FREE(copy);
   
   return vendorIds;
}


ConnectionModeType util_connectionModeStrToNum(const char *connModeStr)
{
   ConnectionModeType connMode = CMC_CONNECTION_MODE_DEFAULT;
   if (connModeStr == NULL)
   {
      vosLog_error("connModeStr is NULL");
      return connMode;
   }

   if (util_strcmp(connModeStr, MDMVS_VLANMUXMODE) == 0)
   {
      connMode = CMC_CONNECTION_MODE_VLANMUX;
   }
   return connMode;

}
#endif    /* zhangdingli */


VOS_RET_E util_standardizeIp6Addr(const char *address, char *stdAddr, UINT32 stdAddrLen)
{
   struct in6_addr in6Addr;
   UINT32 plen;
   char   addr[BUFLEN_40];

   if (address == NULL || stdAddr == NULL)
   {
      return VOS_RET_INVALID_ARGUMENTS;
   }

   if (util_parsePrefixAddress(address, addr, sizeof(addr), &plen) != VOS_RET_SUCCESS)
   {
      vosLog_error("Invalid ipv6 address=%s", address);
      return VOS_RET_INVALID_ARGUMENTS;
   }

   if (inet_pton(AF_INET6, addr, &in6Addr) <= 0)
   {
      vosLog_error("Invalid ipv6 address=%s", address);
      return VOS_RET_INVALID_ARGUMENTS;
   }

   inet_ntop(AF_INET6, &in6Addr, stdAddr, BUFLEN_40);

   if (strchr(address, '/') != NULL)
   {
      char prefix[BUFLEN_8];

      UTIL_SNPRINTF(prefix, sizeof(prefix), "/%d", plen);
      UTIL_STRNCAT(stdAddr, prefix, stdAddrLen);
   }

   return VOS_RET_SUCCESS;

}  /* End of util_standardizeIp6Addr() */

UBOOL8 util_isGUAorULA(const char *address)
{
   struct in6_addr in6Addr;
   UINT32 plen;
   char   addr[BUFLEN_40];

   if (util_parsePrefixAddress(address, addr, sizeof(addr), &plen) != VOS_RET_SUCCESS)
   {
      vosLog_error("Invalid ipv6 address=%s", address);
      return FALSE;
   }

   if (inet_pton(AF_INET6, addr, &in6Addr) <= 0)
   {
      vosLog_error("Invalid ipv6 address=%s", address);
      return FALSE;
   }

   /* currently IANA assigned global unicast address prefix is 001..... */
   if ( ((in6Addr.s6_addr[0] & 0xe0) == 0x20) || 
        ((in6Addr.s6_addr[0] & 0xfe) == 0xfc) )
   {
      return TRUE;
   }

   return FALSE;


}  /* End of util_isGUAorULA() */


VOS_RET_E util_replaceEui64(const char *address1, char *address2)
{
   struct in6_addr   in6Addr1, in6Addr2;

   if (inet_pton(AF_INET6, address1, &in6Addr1) <= 0)
   {
      vosLog_error("Invalid address=%s", address1);
      return VOS_RET_INVALID_ARGUMENTS;
   }
   if (inet_pton(AF_INET6, address2, &in6Addr2) <= 0)
   {
      vosLog_error("Invalid address=%s", address2);
      return VOS_RET_INVALID_ARGUMENTS;
   }

   in6Addr2.s6_addr32[2] = in6Addr1.s6_addr32[2];
   in6Addr2.s6_addr32[3] = in6Addr1.s6_addr32[3];

   if (inet_ntop(AF_INET6, &in6Addr2, address2, BUFLEN_40) == NULL)
   {
      vosLog_error("inet_ntop returns NULL");
      return VOS_RET_INTERNAL_ERROR;
   }

   return VOS_RET_SUCCESS;
      
}  /* End of util_replaceEui64() */


VOS_RET_E util_getAddrPrefix(const char *address, UINT32 plen, char *prefix)
{
   struct in6_addr   in6Addr;
   UINT16 i, k, mask;

   if (plen > 128)
   {
      vosLog_error("Invalid plen=%d", plen);
      return VOS_RET_INVALID_ARGUMENTS;
   }
   else if (plen == 128)
   {

      UTIL_STRNCPY(prefix, address, INET6_ADDRSTRLEN);
      return VOS_RET_SUCCESS; 
   }

   if (inet_pton(AF_INET6, address, &in6Addr) <= 0)
   {
      vosLog_error("Invalid address=%s", address);
      return VOS_RET_INVALID_ARGUMENTS;
   }

   k = plen / 16;
   mask = 0;
   if (plen % 16)
   {
      mask = ~(UINT16)(((1 << (16 - (plen % 16))) - 1) & 0xFFFF);
   }

   in6Addr.s6_addr16[k] &= mask;
   
   for (i = k+1; i < 8; i++)
   {
      in6Addr.s6_addr16[i] = 0;
   } 
   
   if (inet_ntop(AF_INET6, &in6Addr, prefix, INET6_ADDRSTRLEN) == NULL)
   {
      vosLog_error("inet_ntop returns NULL");
      return VOS_RET_INTERNAL_ERROR;
   }

   return VOS_RET_SUCCESS; 
   
}  /* End of util_getAddrPrefix() */


VOS_RET_E util_parsePrefixAddress(const char *prefixAddr, char *address, UINT32 addressLen, UINT32 *plen)
{
   VOS_RET_E ret = VOS_RET_SUCCESS;
   char *tmpBuf;
   char *separator;
   UINT32 len;

   if (prefixAddr == NULL || address == NULL || plen == NULL)
   {
      return VOS_RET_INVALID_ARGUMENTS;
   }      
   
   vosLog_debug("prefixAddr=%s", prefixAddr);

   *address = '\0';
   *plen    = 128;

   len = strlen(prefixAddr);

   if ((tmpBuf = VOS_MALLOC_FLAGS(len+1, 0)) == NULL)
   {
      vosLog_error("alloc of %d bytes failed", len);
      ret = VOS_RET_INTERNAL_ERROR;
   }
   else
   {
      UTIL_SNPRINTF(tmpBuf, len + 1, "%s", prefixAddr);
      separator = strchr(tmpBuf, '/');
      if (separator != NULL)
      {
         /* break the string into two strings */
         *separator = 0;
         separator++;
         while ((isspace(*separator)) && (*separator != 0))
         {
            /* skip white space after comma */
            separator++;
         }

         *plen = atoi(separator);
         vosLog_debug("plen=%d", *plen);
      }

      vosLog_debug("address=%s", tmpBuf);
      if (strlen(tmpBuf) < BUFLEN_40 && *plen <= 128)
      {
         UTIL_STRNCPY(address, tmpBuf, addressLen);
      }
      else
      {
         ret = VOS_RET_INVALID_ARGUMENTS;
      }
      VOS_FREE(tmpBuf);
   }

   return ret;
   
}  /* End of util_parsePrefixAddress() */


UBOOL8 util_ipStrToOctets(const char *input, char *output)
{
   UBOOL8 ret = TRUE;
   char *token = NULL;
   char *last = NULL;
   char buf[BUFLEN_16];
   UINT32 i, num;

   if (input == NULL || strlen(input) < 7 || strlen(input) > 15)
   {
      return FALSE;
   }

   /* need to copy since strtok_r updates string */
   UTIL_STRNCPY(buf, input, sizeof(buf));

   /* IP address has the following format
      xxx.xxx.xxx.xxx where x is decimal number */
   token = strtok_r(buf, ".", &last);
   if ((util_strtoul(token, NULL, 10, &num) != VOS_RET_SUCCESS) ||
       (num > 255))
   {
      ret = FALSE;
   }
   else
   {
      output[0] = num;

      for ( i = 0; i < 3; i++ )
      {
         token = strtok_r(NULL, ".", &last);

         if ((util_strtoul(token, NULL, 10, &num) != VOS_RET_SUCCESS) ||
             (num > 255))
         {
            ret = FALSE;
            break;
         }
         else
         {
            output[i+1] = num;
         }
      }
   }
   return ret;
}

#define MAX_ADJUSTMENT 10
#define MAXFDS	128

static int get_random_fd(void)
{
    int fd;

    while (1) 
    {
        fd = ((int) random()) % MAXFDS;
        if (fd > 2)
        {
            return fd;
        }
    }
}

static void get_random_bytes(void *buf, int nbytes)
{
    int i, n = nbytes, fd = get_random_fd();
    int lose_counter = 0;
    unsigned char *cp = (unsigned char *) buf;

    if (fd >= 0) 
    {
        while (n > 0) 
        {
            i = read(fd, cp, n);
            if (i <= 0) 
            {
                if (lose_counter++ > 16)
                {
                    break;
                }
                continue;
            }
            n -= i;
            cp += i;
            lose_counter = 0;
        }
    }

    /*
     * We do this all the time, but this is the only source of
     * randomness if /dev/random/urandom is out to lunch.
     */
    for (cp = buf, i = 0; i < nbytes; i++)
    {
        *cp++ ^= (rand() >> 7) & 0xFF;
    }
    return;
}

static int get_clock(UINT32 *clock_high, UINT32 *clock_low, UINT16 *ret_clock_seq)
{
    static int              adjustment = 0;
    static struct timeval   last = {0, 0};
    static UINT16           clock_seq;
    struct timeval          tv;
    unsigned long long      clock_reg;

try_again:
    gettimeofday(&tv, 0);
    if ((last.tv_sec == 0) && (last.tv_usec == 0)) 
    {
        get_random_bytes(&clock_seq, sizeof(clock_seq));
        clock_seq &= 0x3FFF;
        last = tv;
        last.tv_sec--;
    }
    if ((tv.tv_sec < last.tv_sec) ||
        ((tv.tv_sec == last.tv_sec) &&
         (tv.tv_usec < last.tv_usec))) 
    {
        clock_seq = (clock_seq+1) & 0x3FFF;
        adjustment = 0;
        last = tv;
    } 
    else if ((tv.tv_sec == last.tv_sec) &&
             (tv.tv_usec == last.tv_usec)) 
    {
        if (adjustment >= MAX_ADJUSTMENT)
        {
            goto try_again;
        }
        adjustment++;
    } 
    else 
    {
        adjustment = 0;
        last = tv;
    }
  
    clock_reg = tv.tv_usec*10 + adjustment;
    clock_reg += ((unsigned long long) tv.tv_sec)*10000000;
    clock_reg += (((unsigned long long) 0x01B21DD2) << 32) + 0x13814000;

    *clock_high = clock_reg >> 32;
    *clock_low = clock_reg;
    *ret_clock_seq = clock_seq;
    return 0;
}

void uuid_pack(struct _uuid_t *uu, unsigned char *ptr)
{
   UINT32  tmp;
   unsigned char  *out = ptr;

   tmp = uu->time_low;
   out[3] = (unsigned char) tmp;
   tmp >>= 8;
   out[2] = (unsigned char) tmp;
   tmp >>= 8;
   out[1] = (unsigned char) tmp;
   tmp >>= 8;
   out[0] = (unsigned char) tmp;
   
   tmp = uu->time_mid;
   out[5] = (unsigned char) tmp;
   tmp >>= 8;
   out[4] = (unsigned char) tmp;
   
   tmp = uu->time_hi_and_version;
   out[7] = (unsigned char) tmp;
   tmp >>= 8;
   out[6] = (unsigned char) tmp;
   
   tmp = uu->clock_seq;
   out[9] = (unsigned char) tmp;
   tmp >>= 8;
   out[8] = (unsigned char) tmp;
   
   memcpy(out+10, uu->node, 6);
}

static void uuid_generate_time(unsigned char *out, unsigned char *macAddress)
{
   struct _uuid_t uu;
   UINT32  clock_mid;
   
   get_clock(&clock_mid, &uu.time_low, &uu.clock_seq);
   uu.clock_seq |= 0x8000;
   uu.time_mid = (UINT16) clock_mid;
   uu.time_hi_and_version = ((clock_mid >> 16) & 0x0FFF) | 0x1000;
   memcpy(uu.node, macAddress, 6);
   uuid_pack(&uu, out);
}

void util_generateUuidStr(char *str, int len, unsigned char *macAddress)
{
   unsigned char d[16];
   uuid_generate_time(d,macAddress);
   snprintf(str, len, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            (UINT8)d[0], (UINT8)d[1], (UINT8)d[2], (UINT8)d[3], (UINT8)d[4], (UINT8)d[5], (UINT8)d[6], (UINT8)d[7], 
            (UINT8)d[8], (UINT8)d[9], (UINT8)d[10], (UINT8)d[11], (UINT8)d[12], (UINT8)d[13], (UINT8)d[14], (UINT8)d[15]);
}

