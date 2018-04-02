#include "QtKeyEncrypt.h"
#include "sm4.h"
#include <cstring>

CQtKeyEncrypt::CQtKeyEncrypt()
{	
	
}


CQtKeyEncrypt::~CQtKeyEncrypt()
{

}

void CQtKeyEncrypt::StrToHex(BYTE *pbDest, BYTE *pbSrc, int nLen)
{
	char h1,h2;
	BYTE s1,s2;
	int i;
	for (i=0; i<nLen; i++)
	{
		h1 = pbSrc[2*i];
		h2 = pbSrc[2*i+1];
		s1 = toupper(h1) - 0x30;
		if (s1 > 9) 
		s1 -= 7;
		s2 = toupper(h2) - 0x30;
		if (s2 > 9) 
		{
			s2 -= 7;
		}
		pbDest[i] = s1*16 + s2;
	}
}

int CQtKeyEncrypt::EncryptOrDecrypt( 
		BYTE *dest, 
		int &destLen, 
		BYTE *source, 
		int sourceLen, 
		BYTE encryAlgType, 
		BYTE *key)
{
	if( dest == NULL || source == NULL)
	{
		PRINT("CQtKeyEncrypt::Encrypt dest or source is NULL!\n");
		return QT_ERROR_FAILURE;
	}

	/*if(sourceLen != ENCRYPT_INPUT_LEN)
	{
		PRINT("CQtKeyEncrypt::Encrypt source is error!\n");
		return QT_ERROR_FAILURE;
	}*/

	switch(encryAlgType){
		case SM4_ECB_ENC:
			SM4ECBEncrypt(dest, destLen, source, sourceLen, key);
			break;
		case SM4_ECB_DEC:
			SM4ECBDecrypt(dest, destLen, source, sourceLen, key);
			break;
		case SM4_CBC_ENC:
			SM4CBCEncrypt(dest, destLen, source, sourceLen, key);
			break;
		case SM4_CBC_DEC:
			SM4CBCDecrypt(dest, destLen, source, sourceLen, key);
			break;
		default:
			PRINT("CQtKeyEncrypt::Encrypt encryAlgType is not match!\n");
			return QT_ERROR_FAILURE;
	}

	return QT_OK;
}

int CQtKeyEncrypt::SM4ECBEncrypt( 
    BYTE *dest, 
    int &destLen, 
    BYTE *source, 
    int sourceLen, 
    BYTE *key)
{
  sm4_context ctx;
  
  sm4_setkey_enc(&ctx,key);
  sm4_crypt_ecb(&ctx, SM4_ECB_ENC, sourceLen,source,dest);

  return QT_OK;
}

int CQtKeyEncrypt::SM4ECBDecrypt( 
    BYTE *dest, 
    int &destLen, 
    BYTE *source, 
    int sourceLen, 
    BYTE *key)
{
  sm4_context ctx;
  
  sm4_setkey_dec(&ctx,key);
  sm4_crypt_ecb(&ctx,SM4_ECB_DEC,sourceLen,source,dest);
  
  return QT_OK;
}

int CQtKeyEncrypt::SM4CBCEncrypt( 
		BYTE *dest, 
		int &destLen, 
		BYTE *source, 
		int sourceLen, 
		BYTE *key)
{

	return QT_OK;
}

int CQtKeyEncrypt::SM4CBCDecrypt( 
		BYTE *dest, 
		int &destLen, 
		BYTE *source, 
		int sourceLen, 
		BYTE *key)
{

	return QT_OK;
}

