/********************************************************
*	Filename:	QtkeyEncrypt.h	                    	*
*	Author	:	lirui(lir@qtec.cn)                    	*
*                                                      	*
* 	History                                         	*
*		2017/07/15	Create								*
********************************************************/
#ifndef QT_KEYENCRYPT_H
#define QT_KEYENCRYPT_H

#include "QtManagent_defines.h"

#define ENCRYPT_KEY_LEN		16
#define ENCRYPT_INPUT_LEN	2048
#define ENCRYPT_OUTPUT_LEN	2048

enum Algorithm{
	SM4_ECB_DEC = 0,
	SM4_ECB_ENC,
	SM4_CBC_ENC,
	SM4_CBC_DEC,
};
	
class CQtKeyEncrypt
{
public:
	CQtKeyEncrypt();
	~CQtKeyEncrypt();

public:
	virtual int EncryptOrDecrypt(BYTE *dest, int &destLen, BYTE *source, int sourceLen, BYTE encryAlgType, BYTE *key);

public:
	virtual int SM4ECBEncrypt(BYTE *dest, int &destLen, BYTE *source, int sourceLen, BYTE *key);
 	virtual int SM4ECBDecrypt(BYTE *dest, int &destLen, BYTE *source, int sourceLen, BYTE *key);
 	virtual int SM4CBCEncrypt(BYTE *dest, int &destLen, BYTE *source, int sourceLen, BYTE *key);
	virtual int SM4CBCDecrypt(BYTE *dest, int &destLen, BYTE *source, int sourceLen, BYTE *key);

public:
	void StrToHex(BYTE *pbDest, BYTE *pbSrc, int nLen);
};

#endif
