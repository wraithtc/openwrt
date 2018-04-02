#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>

#include "fwk.h"
#include "util_fil_oal.h"

UBOOL8 oalFil_isFilePresent(const char *filename)
{
   struct stat statbuf;
   SINT32 rc;

   rc = stat(filename, &statbuf);

   if (rc == 0)
   {
      return TRUE;
   }
   else
   {
      return FALSE;
   }
}


SINT32 oalFil_getSize(const char *filename)
{
   struct stat statbuf;
   SINT32 rc;

   rc = stat(filename, &statbuf);

   if (rc == 0)
   {
      return ((SINT32) statbuf.st_size);
   }
   else
   {
      return -1;
   }
}


VOS_RET_E oalFil_copyToBuffer(const char *filename, UINT8 *buf, UINT32 *bufSize)
{
   SINT32 actualFileSize;
   SINT32 fd, rc;

   actualFileSize = oalFil_getSize(filename);

   if (*bufSize < (UINT32) actualFileSize)
   {
      vosLog_error("user supplied buffer is %d, filename actual size is %d", *bufSize, actualFileSize);
      return VOS_RET_RESOURCE_EXCEEDED;
   }

   *bufSize = 0;
       
   fd = open(filename, 0);
   if (fd < 0)
   {
      vosLog_error("could not open file %s, errno=%d", filename, errno);
      return VOS_RET_INTERNAL_ERROR;
   }

   rc = read(fd, buf, actualFileSize);
   if (rc != actualFileSize)
   {
      vosLog_error("read error, got %d, expected %d", rc, actualFileSize);
      close(fd);
      return VOS_RET_INTERNAL_ERROR;
   }

   close(fd);

   /* let user know how many bytes was actually copied */
   *bufSize = (UINT32) actualFileSize;
   return VOS_RET_SUCCESS;
}


VOS_RET_E oalFil_writeToProc(const char *procFilename, const char *s)
{
   VOS_RET_E ret=VOS_RET_SUCCESS;

#ifdef DESKTOP_LINUX

   vosLog_debug("writing %s to %s", s, procFilename);

#else

   /* on the modem */
   SINT32 fd, rc;

   if ((fd = open(procFilename, O_RDWR)) < 0)
   {
      vosLog_error("could not open %s", procFilename);
      return VOS_RET_INTERNAL_ERROR;
   }

   rc = write(fd, s, strlen(s));

   if (rc < (SINT32) strlen(s))
   {
      vosLog_error("write %s to %s failed, rc=%d", s, procFilename, rc);
      ret = VOS_RET_INTERNAL_ERROR;
   }

   close(fd);

#endif  /* DESKTOP_LINUX */

   return ret;
}


VOS_RET_E oalFil_writeBufferToFile(const char *filename, const UINT8 *buf, UINT32 bufLen)
{
   VOS_RET_E ret=VOS_RET_SUCCESS;
   SINT32 fd, rc;

   if ((fd = open(filename, O_RDWR|O_CREAT|O_TRUNC, S_IRWXU)) < 0)
   {
      vosLog_error("could not open %s", filename);
      return VOS_RET_INTERNAL_ERROR;
   }

   rc = write(fd, buf, bufLen);

   if (rc < (SINT32) bufLen)
   {
      vosLog_error("write to %s failed, rc=%d", filename, rc);
      ret = VOS_RET_INTERNAL_ERROR;
   }

   close(fd);

   return ret;
}


VOS_RET_E oalFil_writeToFileEnd(const char *filename, const char *s)
{
   VOS_RET_E ret=VOS_RET_SUCCESS;

#ifdef DESKTOP_LINUX

   vosLog_debug("writing %s to %s", s, filename);

#else

   /* on the modem */
   SINT32 fd, rc;

   if ((fd = open(filename, O_RDWR)) < 0)
   {
      vosLog_error("could not open %s", filename);
      return VOS_RET_INTERNAL_ERROR;
   }

   lseek(fd, 0, SEEK_END);
   rc = write(fd, s, strlen(s));

   if (rc < (SINT32) strlen(s))
   {
      vosLog_error("write %s to %s failed, rc=%d", s, filename, rc);
      ret = VOS_RET_INTERNAL_ERROR;
   }

   close(fd);

#endif  /* DESKTOP_LINUX */

   return ret;
}


VOS_RET_E oalFil_removeDir(const char *dirname)
{
   DIR *d;
   struct dirent *dent;

   vosLog_debug("dirname=%s", dirname);
   /*
    * Remove all non-directories in this dir.
    * Recurse into any sub-dirs and remove them.
    */
   d = opendir(dirname);
   if (NULL == d)
   {
      /* dir must not exist, no need to remove */
      return VOS_RET_SUCCESS;
   }

   while (NULL != (dent = readdir(d)))
   {
      char path[BUFLEN_1024]={0};

      if (!strcmp(dent->d_name, ".") || !strcmp(dent->d_name, ".."))
         continue;

      snprintf(path, sizeof(path)-1, "%s/%s", dirname, dent->d_name);
      if (DT_DIR == dent->d_type)
      {
         oalFil_removeDir(path);
      }
      else
      {
         unlink(path);
      }
   }

   closedir(d);

   if (0 != rmdir(dirname))
   {
      vosLog_error("rmdir of %s failed!", dirname);
   }

   return VOS_RET_SUCCESS;
}


VOS_RET_E oalFil_makeDir(const char *dirname)
{

   if (0 != mkdir(dirname, S_IRWXU))
   {
      vosLog_error("mkdir on %s failed", dirname);
      return VOS_RET_INTERNAL_ERROR;
   }

   return VOS_RET_SUCCESS;
}
