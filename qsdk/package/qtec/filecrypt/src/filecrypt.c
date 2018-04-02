/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
  Copyright (C) 2011       Sebastian Pipping <sebastian@pipping.org>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  gcc -Wall `pkg-config fuse --cflags --libs` -lulockmgr fusexmp_fh.c -o fusexmp_fh
*/

#define FUSE_USE_VERSION 26

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE

#include <fuse.h>
#include <ulockmgr.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif
#include <sys/file.h> /* flock(2) */
#include <openssl/aes.h>
#include <time.h>
#include "sec_api.h"


#define AES_BITS 128
#define MSG_LEN 128
#define MAX_NAME_LEN 256
#define DEBUG_FILE "/tmp/fs_debug"
#define REAL_PATH_PREFIX    "/mnt/sda1"

#define MAX(a,b) (a)>(b)?(a):(b)

char main_key[16] = {0};

void write_log(const char *fmt, ...)
{
    int ret = 0;
    char buf[4096] = {0};
    char *cmd = NULL;
    char *allocBuf = NULL;
    va_list paraList;
    FILE *fp;

    if (access(DEBUG_FILE, F_OK) != 0)
        return;
    
    va_start(paraList, fmt);
    vsnprintf(buf, sizeof(buf), fmt, paraList);
    va_end(paraList);

    fp = fopen("/mnt/sda1/fs_log", "a");
    fprintf(fp,"%s", buf);
    fclose(fp);
}

void getRealPath(const char *path, char *realPath)
{
    if (strlen(path)+strlen(REAL_PATH_PREFIX) >= MAX_NAME_LEN)
    {
        return;
    }
    strcat(realPath, REAL_PATH_PREFIX);
    strcat(realPath, path);
}

int aes_encrypt(char* in, char* key, char* out, int olen)
{
    AES_KEY aes;
    int i=0;
    unsigned char iv[AES_BLOCK_SIZE];//加密的初始化向量
    //int len=strlen(in);//这里的长度是char*in的长度，但是如果in中间包含'\0'字符的话
    int len = olen;
    //那么就只会加密前面'\0'前面的一段，所以，这个len可以作为参数传进来，记录in的长度

    //至于解密也是一个道理，光以'\0'来判断字符串长度，确有不妥，后面都是一个道理。
    if(!in || !key || !out) return 0;
    
    for(i=0; i<AES_BLOCK_SIZE; ++i)//iv一般设置为全0,可以设置其他，但是加密解密要一样就行
    	iv[i]=0;
    
    if(AES_set_encrypt_key((unsigned char*)key, 128, &aes) < 0)
    {
        return 0;
    }
    i = 0;
    while (len >= 16)
    {
        AES_ecb_encrypt((unsigned char*)in, (unsigned char*)out, &aes, AES_ENCRYPT);
        i++;
        len -= 16;
        in += 16;
        out += 16;
    }
    if (len > 0)
    {
        memcpy(out, in, len);
    }
    //AES_ecb_encrypt((unsigned char*)in, (unsigned char*)out, len, &aes, iv, AES_ENCRYPT);
    return 1;
}
int aes_decrypt(char* in, char* key, char* out, int olen)
{
    AES_KEY aes;
    int i=0;
    unsigned char iv[AES_BLOCK_SIZE];//加密的初始化向量
    //int len=strlen(in);//这里的长度是char*in的长度，但是如果in中间包含'\0'字符的话
    int len = olen;
    //那么就只会加密前面'\0'前面的一段，所以，这个len可以作为参数传进来，记录in的长度

    //至于解密也是一个道理，光以'\0'来判断字符串长度，确有不妥，后面都是一个道理。
    
    if(!in || !key || !out) return 0;

    for(i=0; i<AES_BLOCK_SIZE; ++i)//iv一般设置为全0,可以设置其他，但是加密解密要一样就行
    	iv[i]=0;
    
    if(AES_set_decrypt_key((unsigned char*)key, 128, &aes) < 0)
    {
        return 0;
    }

    while (len >= 16)
    {
        AES_ecb_encrypt((unsigned char*)in, (unsigned char*)out, &aes, AES_DECRYPT);
        i++;
        len -= 16;
        in += 16;
        out += 16;
    }

    if (len > 0)
    {
        memcpy(out, in, len);
    }
    //AES_cbc_encrypt((unsigned char*)in, (unsigned char*)out, len, &aes, iv, AES_DECRYPT);
    return 1;
}

static int xmp_getattr(const char *path, struct stat *stbuf)
{
	int res;
    char rPath[MAX_NAME_LEN] = {0};
    int fd;
    char len[16];
    int i;
	(void) path;
    getRealPath(path, rPath);
    write_log("path:[%s],<%s><%d>\n",rPath, __FUNCTION__,__LINE__);
    if (strcmp(path, "/") && strcmp(path, "/desktop.ini") && 0 == access(rPath, F_OK))
    {
        errno = 0;
        res = lstat(rPath, stbuf);
        if (S_ISREG(stbuf->st_mode))
        {
           
            fd = open(rPath, O_RDONLY);
            if (fd == -1)
        		return -errno;
            
            if (stbuf->st_size >= 16)
            {
                res = pread(fd, len, 16, 32);
                if (res == -1){
                    close(fd);
            		return -errno;
                }
                write_log("len[16]:");
                for (i = 0 ; i < 16; i++)
                {
                    write_log("%d ", len[i]);
                }
                write_log("<%s><%d>\n",__FUNCTION__,__LINE__);
            }

            res = pread(fd, len, 16, 16);
            if (res == -1){
                close(fd);
        		return -errno;
            }
            close(fd);
            write_log("len[16]:");
            for (i = 0 ; i < 16; i++)
            {
                write_log("%d ", len[i]);
            }
            write_log("<%s><%d>\n",__FUNCTION__,__LINE__);
            
            if (len[0] == 1)
                stbuf->st_size -= 32;
            else
                stbuf->st_size -= 32;//stbuf->st_size -= (48 - len[0]+1);
           
        }
    }
    else
    {
        errno = 0;
        res = lstat(rPath, stbuf);
        stbuf->st_size -= 32;
    }
    
	if (res == -1)
		return -errno;
    write_log("<%s>end\n", __FUNCTION__);
	return 0;
}

static int xmp_fgetattr(const char *path, struct stat *stbuf,
			struct fuse_file_info *fi)
{
	int res;

	(void) path;
	res = fstat(fi->fh, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_access(const char *path, int mask)
{
	int res;
    char rPath[MAX_NAME_LEN] = {0};
    
    getRealPath(path, rPath);
	res = access(rPath, mask);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readlink(const char *path, char *buf, size_t size)
{
	int res;
    char rPath[MAX_NAME_LEN] = {0};
    
    getRealPath(path, rPath);
    
	res = readlink(rPath, buf, size - 1);
	if (res == -1)
		return -errno;

	buf[res] = '\0';
	return 0;
}

struct xmp_dirp {
	DIR *dp;
	struct dirent *entry;
	off_t offset;
};

static int xmp_opendir(const char *path, struct fuse_file_info *fi)
{
	int res;
	struct xmp_dirp *d = malloc(sizeof(struct xmp_dirp));
    char rPath[MAX_NAME_LEN] = {0};
    
    getRealPath(path, rPath);
	if (d == NULL)
		return -ENOMEM;

	d->dp = opendir(rPath);
	if (d->dp == NULL) {
		res = -errno;
		free(d);
		return res;
	}
	d->offset = 0;
	d->entry = NULL;

	fi->fh = (unsigned long) d;
	return 0;
}

static inline struct xmp_dirp *get_dirp(struct fuse_file_info *fi)
{
	return (struct xmp_dirp *) (uintptr_t) fi->fh;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
	struct xmp_dirp *d = get_dirp(fi);
    
	(void) path;
	if (offset != d->offset) {
		seekdir(d->dp, offset);
		d->entry = NULL;
		d->offset = offset;
	}
	while (1) {
		struct stat st;
		off_t nextoff;

		if (!d->entry) {
			d->entry = readdir(d->dp);
			if (!d->entry)
				break;
		}

		memset(&st, 0, sizeof(st));
		st.st_ino = d->entry->d_ino;
		st.st_mode = d->entry->d_type << 12;
		nextoff = telldir(d->dp);
		if (filler(buf, d->entry->d_name, &st, nextoff))
			break;

		d->entry = NULL;
		d->offset = nextoff;
	}

	return 0;
}

static int xmp_releasedir(const char *path, struct fuse_file_info *fi)
{
	struct xmp_dirp *d = get_dirp(fi);
	(void) path;
	closedir(d->dp);
	free(d);
	return 0;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev)
{
	int res;
    
    char rPath[MAX_NAME_LEN] = {0};
    
    getRealPath(path, rPath);
	if (S_ISFIFO(mode))
		res = mkfifo(rPath, mode);
	else
		res = mknod(rPath, mode, rdev);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_mkdir(const char *path, mode_t mode)
{
	int res;
    char rPath[MAX_NAME_LEN] = {0};
    
    getRealPath(path, rPath);
    mode = 511;
	res = mkdir(rPath, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_unlink(const char *path)
{
	int res;
    char rPath[MAX_NAME_LEN] = {0};
    
    getRealPath(path, rPath);
    
	res = unlink(rPath);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rmdir(const char *path)
{
	int res;
    char rPath[MAX_NAME_LEN] = {0};
    
    getRealPath(path, rPath);
    
	res = rmdir(rPath);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_symlink(const char *from, const char *to)
{
	int res;
    
	res = symlink(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rename(const char *from, const char *to)
{
	int res;
    char frPath[MAX_NAME_LEN] = {0};
    char trPath[MAX_NAME_LEN] = {0};
    getRealPath(from, frPath);
    getRealPath(to, trPath);
	res = rename(frPath, trPath);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_link(const char *from, const char *to)
{
	int res;
    
	res = link(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chmod(const char *path, mode_t mode)
{
	int res;
    char rPath[MAX_NAME_LEN] = {0};
    
    getRealPath(path, rPath);
	res = chmod(rPath, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chown(const char *path, uid_t uid, gid_t gid)
{
	int res;
    char rPath[MAX_NAME_LEN] = {0};
    
    getRealPath(path, rPath);
    
	res = lchown(path, uid, gid);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_truncate(const char *path, off_t size)
{
	int res;
    char rPath[MAX_NAME_LEN] = {0};
    
    getRealPath(path, rPath);
    
	res = truncate(rPath, size);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_ftruncate(const char *path, off_t size,
			 struct fuse_file_info *fi)
{
	int res;

	(void) path;

	res = ftruncate(fi->fh, size);
	if (res == -1)
		return -errno;

	return 0;
}

#ifdef HAVE_UTIMENSAT
static int xmp_utimens(const char *path, const struct timespec ts[2])
{
	int res;
    char rPath[MAX_NAME_LEN] = {0};
    
    getRealPath(path, rPath);
	/* don't use utime/utimes since they follow symlinks */
	res = utimensat(0, rPath, ts, AT_SYMLINK_NOFOLLOW);
	if (res == -1)
		return -errno;

	return 0;
}
#endif

static int xmp_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	int fd, res, i;
    char rPath[MAX_NAME_LEN] = {0};
    char lasInfo[16] = {1,0x00,0x00,0x00,0x00,'#','#','#','#','#','#','#','#','#','#',0};
    char key[16] = {0};
    char encrypt_key[16] = {0};
    int block_size = 4096;

    write_log("create <%s> <%d>,[%s]_[%d]\n", path,*(int *)(&lasInfo[1]), __FUNCTION__, __LINE__);
    write_log("len[1] :");
    getRealPath(path, rPath);
    
    res = GetRandom(key, sizeof(key));
    if (res != 0){
        write_log("Fail to get random num from security chip, ret = %d\n",res, __FUNCTION__, __LINE__);
        return res;
    }

    aes_encrypt((char *)key, main_key, encrypt_key, 16);
        
	fd = open(rPath, fi->flags, mode);
	if (fd == -1)
		return -errno;

	fi->fh = fd;
    res = pwrite(fd, encrypt_key, 16, 0);
    if (res == -1)
		return -errno;
    res = pwrite(fd, lasInfo, 16, 16);
    if (res == -1)
		return -errno;
    write_log("<%s>end\n", __FUNCTION__);
	return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi)
{
	int fd;
    char rPath[MAX_NAME_LEN] = {0};
    write_log("open <%s>,[%s]_[%d]\n", path, __FUNCTION__, __LINE__);
    getRealPath(path, rPath);

	fd = open(rPath, fi->flags);
	if (fd == -1)
		return -errno;

	fi->fh = fd;
	return 0;
}
#define CRYPT_BLOCK 65536
static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	int res;
    char *bufRead;
    int fd,i, n;
    char rPath[MAX_NAME_LEN] = {0};
    char decrypt_key[16] = {0};
    char tmpkey[16] = {0};
    char len[16] = {0};
    int lastLen;
    //int *block_size;
    write_log("<%s>start\n", __FUNCTION__);
	(void) path;
    getRealPath(path, rPath);write_log("[%s]------------<%s><%d>\n",rPath, __FUNCTION__,__LINE__);
    fd = open(rPath, O_RDONLY);write_log("[%s]------------<%s><%d>\n",rPath, __FUNCTION__,__LINE__);
    if (fd == -1)
		return -errno;write_log("[%s]------------<%s><%d>\n",rPath, __FUNCTION__,__LINE__);
    /*res = pread(fd, len, 16, 16);
    if (res == -1)
		res = -errno;
    lastLen = len[0] -1;
    block_size = (int *)&len[1];*/
    res = pread(fd, tmpkey, 16, 0);write_log("[%s]------------<%s><%d>\n",rPath, __FUNCTION__,__LINE__);
    if (res == -1)
		res = -errno;write_log("[%s]------------<%s><%d>\n",rPath, __FUNCTION__,__LINE__);
    aes_decrypt(tmpkey, main_key, decrypt_key, (int)sizeof(tmpkey));write_log("[%s]------------<%s><%d>\n",rPath, __FUNCTION__,__LINE__);
    write_log("decrypt key:");
    for (i = 0 ; i < 16; i++)
    {
        write_log("%d ", decrypt_key[i]);
    }
    write_log("[%s]<%s><%d>\n",rPath, __FUNCTION__,__LINE__);
    close(fd);
    write_log("res:[%d],size:[%d], <%s><%d>\n", res,(int)size, __FUNCTION__, __LINE__);
    bufRead = (char *)calloc(1, size);
    if (!bufRead)
        return -errno;
	res = pread(fi->fh, bufRead, size, offset + 32);
    write_log("res:[%d],size:[%d], <%s><%d>\n", res,(int)size, __FUNCTION__, __LINE__);
    if (res == -1)
		res = -ENOMEM;write_log("res:[%d],size:[%d], <%s><%d>\n", res,(int)size, __FUNCTION__, __LINE__);
    if (res == 0){
        strncpy(buf, bufRead,size);
        goto out;
    }
    write_log("res:[%d],size:[%d], <%s><%d>\n", res,(int)size, __FUNCTION__, __LINE__);

    aes_decrypt(bufRead, decrypt_key, buf, res);write_log("res:[%d],size:[%d], <%s><%d>\n", res,(int)size, __FUNCTION__, __LINE__);

out:
    if (bufRead)
        free(bufRead);
    write_log("<%s>end\n", __FUNCTION__);
    return res;

}

static int xmp_read_buf(const char *path, struct fuse_bufvec **bufp,
			size_t size, off_t offset, struct fuse_file_info *fi)
{
	struct fuse_bufvec *src;

	(void) path;

	src = malloc(sizeof(struct fuse_bufvec));
	if (src == NULL)
		return -ENOMEM;

	*src = FUSE_BUFVEC_INIT(size);

	src->buf[0].flags = FUSE_BUF_IS_FD | FUSE_BUF_FD_SEEK;
	src->buf[0].fd = fi->fh;
	src->buf[0].pos = offset;

	*bufp = src;

	return 0;
}

static int xmp_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	int fd;
	int res1, res2, res;
    size_t len;
    char *bufWrite;
    char rPath[MAX_NAME_LEN] = {0};
    char decrypt_key[16] = {0};
    char tmpkey[16] = {0};
    char lastLen[16] = {1,0,'#','#','#','#','#','#','#','#','#','#','#','#','#',0};
    write_log("write <%s>,[%s]_[%d]\n", path, __FUNCTION__, __LINE__);
    getRealPath(path, rPath);
    int *block_size;
    
    (void) fi;
    fd = open(rPath, O_RDONLY);
    if (fd == -1)
		return -errno;
    res = pread(fd, tmpkey, 16, 0);
    if (res == -1)
		res = -errno;
  
    aes_decrypt(tmpkey, main_key, decrypt_key, (int)sizeof(tmpkey));
    close(fd);
    
	fd = open(rPath, O_WRONLY);
	if (fd == -1)
		return -errno;
    len = size%16?(16*(size/16 + 1)):size;
    lastLen[0] = size%16 + 1;
    block_size = (int *)&lastLen[1];
    bufWrite = (char *)calloc(1, len);
    if (!bufWrite)
        return -ENOMEM;
    
    aes_encrypt((char *)buf, decrypt_key, bufWrite, size);
    (void) path;
    res1 = pwrite(fd, lastLen, 16, offset + 16);
	if (res1 == -1)
		goto out;
    close(fd);
#if 0
    fd = open(rPath, O_RDONLY);
    if (fd == -1)
		return -errno;
    res = pread(fd, lastLen, 16, 16);
    if (res == -1)
		res = -errno;
    close(fd);

    write_log("len[16]:");
    for (fd = 0 ; fd < 16; fd++)
    {
        write_log("%d ", lastLen[fd]);
    }
    write_log("\n");
#endif    
    fd = open(rPath, O_WRONLY);
	if (fd == -1)
		goto out;

	res2 = pwrite(fd, bufWrite, len, offset + 32);
	
	if (res2 == -1)
		goto out;

    close(fd);
   
    return res2 + res1 + 16;    

out:
    close(fd);
    free(bufWrite);
    return -errno;
}

static int xmp_write_buf(const char *path, struct fuse_bufvec *buf,
		     off_t offset, struct fuse_file_info *fi)
{
	struct fuse_bufvec dst = FUSE_BUFVEC_INIT(fuse_buf_size(buf));
    int fd, i, headoffset = 0, newsize;
	int res1, res2, res, restlen;
    size_t len;
    char *bufWrite, *tmpbuf = NULL;
    char rPath[MAX_NAME_LEN] = {0};
    char decrypt_key[16] = {0};
    char tmpkey[16] = {0};
    char lastLen[16] = {1,0x00,0x10,0x00,0x00,'#','#','#','#','#','#','#','#','#','#',0};
    char restBuf[16] = {0};
    char restBufEnc[16] = {0};
    size_t size = fuse_buf_size(buf);
    //int *block_size;
    
    write_log("write <%s>,<%d>,[%s]_[%d]\n", path, (int)size, __FUNCTION__, __LINE__);
    
    getRealPath(path, rPath);
    (void) fi;
    fd = open(rPath, O_RDONLY);
    if (fd == -1)
		return -errno;
    res = pread(fd, tmpkey, 16, 0);
    if (res == -1)
		res = -errno;
    res = pread(fd, lastLen, 16, 16);
    if (res == -1)
		res = -errno;
    /*
    write_log("encrypt key[16]:");
    for (fd = 0 ; fd < 16; fd++)
    {
        write_log("%d ", tmpkey[fd]);
    }
    write_log("<%s><%d>\n",__FUNCTION__,__LINE__);*/
    
    aes_decrypt(tmpkey, main_key, decrypt_key, (int)sizeof(tmpkey));
    close(fd);
    
	fd = open(rPath, O_WRONLY);
	if (fd == -1)
		return -errno;
    restlen = lastLen[0] - 1;
    newsize = size;
    if (restlen > 0)
    {
        headoffset = 16 - restlen;
        newsize = size - headoffset;
        memcpy(restBuf, &lastLen[1], restlen);
        memcpy(restBuf + restlen, buf->buf[0].mem, headoffset);
        tmpbuf = (char *)malloc(size - headoffset);
        memcpy(tmpbuf, buf->buf[0].mem + headoffset, size - headoffset);
        memcpy(buf->buf[0].mem, tmpbuf, newsize+1);
        *((char *)buf->buf[0].mem+newsize-1) = *(tmpbuf+newsize-1);
        free(tmpbuf);
        aes_encrypt((char *)restBuf, decrypt_key, restBufEnc, 16);
        res1 = pwrite(fd, restBufEnc, 16, offset + 32 - restlen);
    	if (res1 == -1)
    		goto out;
    }
    len = (newsize)%16?(16*(newsize/16)):(newsize);
    lastLen[0] = (newsize)%16 + 1;
    if (lastLen[0] > 1)
    {/*
        if (restlen > 0)
        {
            memcpy(&lastLen[1], tmpbuf + len, lastLen[0] - 1);write_log("end:[0x%.2x] <%s><%d>\n",*(tmpbuf+newsize-1), __FUNCTION__,__LINE__);
        }
        else
        {*/
        memcpy(&lastLen[1], (char *)(buf->buf[0].mem) + len, lastLen[0] - 1);
        //}
    }
        
    /*
    block_size = (int *)(&lastLen[1]);
    if (size > *block_size)
        *block_size = size;*/
    bufWrite = (char *)calloc(1, newsize);
    if (!bufWrite)
        return -ENOMEM;
    write_log("lastlen:");
    for (i = 0 ; i < 16; i++)
    {
        write_log("0x%.2x ", lastLen[i]);
    }
    write_log("<%s><%d>\n",__FUNCTION__,__LINE__);
    aes_encrypt((char *)buf->buf[0].mem, decrypt_key, bufWrite, newsize);
    
    memcpy(buf->buf[0].mem, bufWrite, newsize);
    (void) path;
    res1 = pwrite(fd, lastLen, 16, 16);
	if (res1 == -1)
		goto out;
    
    close(fd);

	(void) path;
    write_log("restlen:%d,headoffset:%d, size:%d,<%s><%d>\n",restlen,headoffset, size, __FUNCTION__,__LINE__);
	dst.buf[0].flags = FUSE_BUF_IS_FD | FUSE_BUF_FD_SEEK;
	dst.buf[0].fd = fi->fh;
	dst.buf[0].pos = offset + 32 + headoffset;
    dst.buf[0].size = newsize;
	res1 = fuse_buf_copy(&dst, buf, FUSE_BUF_SPLICE_NONBLOCK);
    free(bufWrite);

    return size;
out:
    close(fd);

    free(bufWrite);write_log("<%s>endi\n", __FUNCTION__);
    return -errno;

}

static int xmp_statfs(const char *path, struct statvfs *stbuf)
{
	int res;
    char rPath[MAX_NAME_LEN] = {0};

    getRealPath(path, rPath);
	res = statvfs(rPath, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_flush(const char *path, struct fuse_file_info *fi)
{
	int res;

	(void) path;
	/* This is called from every close on an open file, so call the
	   close on the underlying filesystem.	But since flush may be
	   called multiple times for an open file, this must not really
	   close the file.  This is important if used on a network
	   filesystem like NFS which flush the data/metadata on close() */
	res = close(dup(fi->fh));
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_release(const char *path, struct fuse_file_info *fi)
{
	(void) path;
	close(fi->fh);

	return 0;
}

static int xmp_fsync(const char *path, int isdatasync,
		     struct fuse_file_info *fi)
{
	int res;
	(void) path;

#ifndef HAVE_FDATASYNC
	(void) isdatasync;
#else
	if (isdatasync)
		res = fdatasync(fi->fh);
	else
#endif
		res = fsync(fi->fh);
	if (res == -1)
		return -errno;

	return 0;
}

#ifdef HAVE_SETXATTR
/* xattr operations are optional and can safely be left unimplemented */
static int xmp_setxattr(const char *path, const char *name, const char *value,
			size_t size, int flags)
{
	int res = lsetxattr(path, name, value, size, flags);
	if (res == -1)
		return -errno;
	return 0;
}

static int xmp_getxattr(const char *path, const char *name, char *value,
			size_t size)
{
	int res = lgetxattr(path, name, value, size);
	if (res == -1)
		return -errno;
	return res;
}

static int xmp_listxattr(const char *path, char *list, size_t size)
{
	int res = llistxattr(path, list, size);
	if (res == -1)
		return -errno;
	return res;
}

static int xmp_removexattr(const char *path, const char *name)
{
	int res = lremovexattr(path, name);
	if (res == -1)
		return -errno;
	return 0;
}
#endif /* HAVE_SETXATTR */
#if 1
static int xmp_lock(const char *path, struct fuse_file_info *fi, int cmd,
		    struct flock *lock)
{
	(void) path;

	return ulockmgr_op(fi->fh, cmd, lock, &fi->lock_owner,
			   sizeof(fi->lock_owner));
}
#endif
static int xmp_flock(const char *path, struct fuse_file_info *fi, int op)
{
	int res;
	(void) path;

	res = flock(fi->fh, op);
	if (res == -1)
		return -errno;

	return 0;
}

static struct fuse_operations xmp_oper = {
	.getattr	= xmp_getattr,
	.fgetattr	= xmp_fgetattr,
	.access		= xmp_access,
	.readlink	= xmp_readlink,
	.opendir	= xmp_opendir,
	.readdir	= xmp_readdir,
	.releasedir	= xmp_releasedir,
	.mknod		= xmp_mknod,
	.mkdir		= xmp_mkdir,
	.symlink	= xmp_symlink,
	.unlink		= xmp_unlink,
	.rmdir		= xmp_rmdir,
	.rename		= xmp_rename,
	.link		= xmp_link,
	.chmod		= xmp_chmod,
	.chown		= xmp_chown,
	.truncate	= xmp_truncate,
	.ftruncate	= xmp_ftruncate,
#ifdef HAVE_UTIMENSAT
	.utimens	= xmp_utimens,
#endif
	.create		= xmp_create,
	.open		= xmp_open,
	.read		= xmp_read,
	//.read_buf	= xmp_read_buf,
	.write		= xmp_write,
	.write_buf	= xmp_write_buf,
	.statfs		= xmp_statfs,
	.flush		= xmp_flush,
	.release	= xmp_release,
	.fsync		= xmp_fsync,
#ifdef HAVE_SETXATTR
	.setxattr	= xmp_setxattr,
	.getxattr	= xmp_getxattr,
	.listxattr	= xmp_listxattr,
	.removexattr	= xmp_removexattr,
#endif
	.lock		= xmp_lock,
	.flock		= xmp_flock,

	.flag_nullpath_ok = 1,
#if HAVE_UTIMENSAT
	.flag_utime_omit_ok = 1,
#endif
};

int main(int argc, char *argv[])
{
    int ret,i;
    char cmd[128] = {0};
	umask(0);
    ret = LoadData(main_key, (int)sizeof(main_key));
    for (i = 0;i < 16; i++)
    {
        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "echo '0x%.2x ' >> /tmp/mainkey ", main_key[i]);
        system(cmd);
    }
    if (ret != 0){
        printf("Fail to get main key from security chip, ret = %d\n", ret);
        return ret;
    }

	return fuse_main(argc, argv, &xmp_oper, NULL);
}


