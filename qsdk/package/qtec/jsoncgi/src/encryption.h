#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <stdio.h>
#include <stdlib.h>
#pragma pack(1)

#define KEYLEN        16
#define KEYIDLEN      16
#define BUFLEN_64     64
#define BUFLEN_1024   1024
#define BUFLEN_2048   2048
#define BUFLEN_4096   4096
#define BUFLEN_8192   8192
#define USERIDLEN     33
#define DEVICEIDLEN   33
#define MIXRAWKEYNUM  400
#define MAXSYNCKEYNUM 50

typedef struct _RemoteInfo
{
	long remoteUserId;
	char remoteDevicedId[DEVICEIDLEN];
}RemoteInfo;

typedef struct _SrcMessage
{
	unsigned char keyNumber;
	char  keyLength;
	char  specified;
	char  shareNumber;
	char  localUserId[USERIDLEN];
	char  localDeviceId[DEVICEIDLEN];
}SrcMessage;

typedef struct _DestMessage
{
	char  keyNumber;
	char  keyLength;
	char  pushUserId[USERIDLEN];
	char  pushDeviceId[DEVICEIDLEN];
	unsigned char  *keyId;
	unsigned char  *key;
}DestMessage;

#pragma pack()
#endif
