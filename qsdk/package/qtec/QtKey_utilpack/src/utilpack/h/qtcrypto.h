/*------------------------------------------------------*/
/* Crpto interface define                               */
/*                                                      */
/* qtcrypto.h                                           */
/*                                                      */
/* Copyright (C) QTEC Inc.                              */
/* All rights reserved                                  */
/*                                                      */
/* Author                                               */
/*    zhubin (zhubin@qtec.cn)                           */
/*                                                      */
/* History                                              */
/*    2017/02/15  Create                                */
/*                                                      */
/*------------------------------------------------------*/
#ifndef __QTEC_CRYPTO_H__
#define __QTEC_CRYPTO_H__

#include "QtReferenceControl.h"

class QT_OS_EXPORT IQtCrypto: public IQtReferenceControl
{
public:
	enum CryptoType
	{
		Key_Exchange_Mode_RSA		= 0x100,
		Key_Exchange_Mode_PKCS7,
		Data_Encrypt_Mode_AES		= 0x200
	};
#ifndef QT_MMP	//mmp don't need it.
	/***********************************************************************************
	*	Description:	This function needs to be called before using the library on the
	*					server side. The purpose of the 
	*					function is to load the CA certificate and key into the memory from 
	*					the CA certificate and key files. A password must be supplied to 
	*					decrypt the private key.
	*
	*
	*	input:			Inputs are the CA certificate and key files and
	*					the password used to encrypt the CA private key while it was generated.
	*
	*	output:			The CA certificate and key are loaded into memory
	*	
	*	return:			0 for success and non zero for failure.
	*
	***********************************************************************************/

	virtual QtResult Init(const char* cafile, 
					const char* cakeyfile,
					const char* passwd) = 0;

	/***********************************************************************************
	*	Description:	This function needs to be called before using the library on the
	*					client side for both PKCS#7 and RSA key exchange. When using raw
	*					RSA as the key exchange mechanism pass the cacet and client cert 
	*					parameters as NULL. The purpose of this function is to setup 
	*					necessary parameters and load the CA certificate 
	*					into memory in the audioplus security library. audioSecCleanUp must
	*					be called when terminating the client.
	*
	*	input:			
	*					cacert:			The PEM encoded CA certificate.
	*					clientCert:		The PEM encoded client certificate.
	*					clientPrivkey:	The PEM encoded client private key.
	*					pw:				The password that is used to encrypt the client's
	*									private key. In audioplus, this is the client login pw.
	*
	*	output:			The CA certificate, client certificate and private key are loaded into 
	*					memory.
	*	
	*	return:			0 for success and non zero for failure.
	*
	***********************************************************************************/
	virtual QtResult Init(const char* cacert,
					const char* clientcert,
					const char* clientPriveky,
					const char* passwd) = 0;

	/***********************************************************************************
	*	Description:	This function is paired with the Init function. The 
	*					application (client and server) should call this function to release
	*					resource.
	*	input:			None
	*	output:			None
	*	return:			None 
	*
	***********************************************************************************/	
	virtual void Destroy() = 0;

	/***********************************************************************************
	*	Description:	This function is setting the key exchange mode and data encrypt mode. 
	*					application (client and server) can call this function more than once
	*					after it called Init. the Default key exchange mode is Key_Exchange_Mode_PKCS7,
	*					and the default data encrypt mode is Data_Encrypt_Mode_AES
	*	input:			
			nType:  key exchange or data encrypt or other type.
	*	output:			None
	*	return:			0 means ok, otherwise failed. 
	*
	***********************************************************************************/	
	virtual QtResult SetMode(CryptoType nType) = 0;

	/***********************************************************************************
	*	Description:	This function generates a PKCS#10 certificate request and a RSA
	*					key pair. The "privKey" private key is encrypted with a key derived 
	*					from the supplied "passwd" password. The input username is used
	*					as an ID to bind with the public key in the certificate request. 
	*					
	*	input:			username and passwd. 
	*			
	*	output:			PKCS#10 Certificate Signing Request (CSR) in req and encrypted
	*					RSA private key in privKey. The following are examples of req
	*					and privKey.
	*
	*	return:	0 for success and non zero for failure.
	*
	***********************************************************************************/
	virtual QtResult CreateReqPrivatePair(const char* username,
							const char* passwd,
							char*& req,
							char*& privKey) = 0;
	
	/***********************************************************************************
	*	Description:	This function is for server sid to sign a CSR and output a X.509 
	*					certificate. The application/caller must supply a universal unique 
	*					serial number for each certificate.
	*					
	*									
	*	input:			CSR certReq generated by CreateReqPrivatePair and a serial number.
	*
	*	output:			An X.509 certificate saved in cert. The following is an example
	*					of an X.509 certificate, encoded in PEM format.
	*					
	*	return:			0 for success and non zero for failure.
	*
	***********************************************************************************/

	virtual QtResult CreateCert(const char* req,
							DWORD dwSerial,
							char*& cert) = 0;

	/***********************************************************************************
	*
	*	Description:	This function is to decrypt the encrypted private key with the
	*					supplied password passwd. If the password does not match what
	*					is used to encrypt the private key then it returns FALSE.
	*
	*	input:			PEM encoded private key and user password.
	*
	*	output:			none
	*
	*	return:			TRUE for success and FALSE for failure.
	*
	***********************************************************************************/
	
	virtual BOOL VefityPrivateKey(const char* strPriKey,
									const char* strPasswd) = 0;

	/***********************************************************************************
	*	Description:	This function is to generate a session key for end-to-end data 
	*					encryption.
	*				
	*	input:			keyLen which specifies the session key length. unit is byte
	*	
	*	output:			A session key stored in key. 
	*
	*	return:			0 for success and non zero for failure.
	*
	***********************************************************************************/
	
	virtual QtResult CreateSessionKey(int iKeyLen,
								LPBYTE& lpKey) = 0;

	/***********************************************************************************
	*	Description:	This function is to encrypt a session key. The encoding of the encrypted session
	*					key can be in the form of either binary or PKCS#7 digital envelope
	*					depending on how the caller/app set the key exchange mode with 
	*					SetMode API. The default mode is PKCS#7 as it is more secure.
	*	
	*	input:			Session key "key", key length keyLen, and certificate cert.
	*
	*	output:			Encrypted session key saved in lpEKey and Encrypt Key length is in iEKeyLength.
	*		
	*	return:			
			0 means success, otherwise it failed.
	***********************************************************************************/

	virtual QtResult EncryptSessionKey(const LPBYTE lpKey,
									int iKeyLen,
									const char* strRecvCert,
									LPBYTE& lpEKey,
									int& iEKeyLength) = 0;


	/***********************************************************************************
	*	Description:	This function is to decrypt an encrypted session key with a RSA
	*					private key. In the PKCS#7 mode, it also verifies the signature of 
	*					the message and the certificate that signed the message. So, the
	*					CA certificate must be available in this case. The receiver needs 
	*					to call this function in order to recover the cleartext session key 
	*					during the key exchange process.
	*					
	*	input:			Encrypted session key lpEKey, key length iEKeyLength
	*					
	*
	*	output:			Cleartext session key saved in lpKey and key length in iKeyLength.
	*
	*	return:			0 means success, otherwise failed
	*
	***********************************************************************************/
	virtual QtResult DecryptSessionKey(const LPBYTE lpEKey,
									int iEKeyLength,
									LPBYTE& lpKey,
									int& iKeyLength) = 0;

	/***********************************************************************************
	*	Description:	This function is to encrypt or decrypt data.			
	*	input:			lpInData, iInDataLength, lpKey, iKeyLength, bEncrypt
	*
	*	output:			lpOutData and iOutDataLength.
	*	return:			0:success, otherwise failed.
	*
	*	NOTICE:			The length of the cipher text will be multiple of 16,
	*					so the cipher text may be longer than the plain text.
	*
	***********************************************************************************/	
	virtual QtResult CryptoData(const LPBYTE lpInData,
							int	iInDataLength,
							const LPBYTE lpKey,
							int iKeyLenght,
							BOOL bEncrypt,
							LPBYTE& lpOutData,
							int& iOutDataLength,
							BOOL bStream) = 0;
	
	/***********************************************************************************
	*	Description:	save data into file			
	*	input:			data and data length in lpData and iDataLength, and filename in
	*					strFileName.
	*
	*	output:			None.
	*	return:			0:success, otherwise failed.
	*
	***********************************************************************************/	
	static QtResult SaveToFile(const LPBYTE lpData,
					int iDataLength,
					const char* strFileName);

	/***********************************************************************************
	*	Description:	read data from file			
	*	input:			filename in strFileName.
	*
	*	output:			Data and Data length in lpData and iDataLength.
	*	return:			0:success, otherwise failed.
	*
	***********************************************************************************/	
	static QtResult ReadFromFile(const char* strFileName,
					LPBYTE& lpData,
					int& iDataLength);

#endif

	/***********************************************************************************
	*	Description:	MD5 hashing function implement			
	*	input:			in data in lpInData and length as dwInLen
	*
	*	output:			lpOutData, length = 16.
	*	return:			0 means success, otherwise failed.
	*
	***********************************************************************************/	
	static QtResult MD5(const LPBYTE lpInData, 
					DWORD dwInLen,
					LPBYTE& lpOutData);
	
	/***********************************************************************************
	*	Description:	base64 encode/decode			
	*	input:			in data in lpInData and length as dwInLen, 
	*					bEncode: encode or decode.
	*	output:			lpOutData, dwOutLen.
	*	return:			0 means success, otherwise failed.
	*
	***********************************************************************************/	
	static QtResult Base64(const LPBYTE lpInData,
							DWORD dwInLen,
							BOOL bEncode,
							LPBYTE& lpOutData,
							DWORD& dwOutLen);
	
	/***********************************************************************************
	*	Description:	This function is to encrypt or decrypt data.			
	*	input:			lpInData, iInDataLength, lpKey, iKeyLength, bEncrypt
	*
	*	output:			lpOutData and iOutDataLength.
	*	return:			0:success, otherwise failed.
	*
	***********************************************************************************/	
	static QtResult CryptoDataEx(const LPBYTE lpInData,
							int	iInDataLength,
							const LPBYTE lpKey,
							int iKeyLenght,
							BOOL bEncrypt,
							LPBYTE& lpOutData,
							int& iOutDataLength,
							BOOL bStream,
							CryptoType = Data_Encrypt_Mode_AES);

	/***********************************************************************************
	*	Description:	destroy the resource which get by this module			
	*	input:			p, the resource point which u want to destroy.
	*
	*	output:			None.
	*	return:			none.
	*
	***********************************************************************************/	
	static void Free(void* p);
};

#ifndef QT_MMP
extern "C"
{
	QT_OS_EXPORT IQtCrypto* CreateCrypto();
}
#endif


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class QT_OS_EXPORT IExtCommUtils
{
public:
	static unsigned char* SHA1(const unsigned char *d, unsigned long n, unsigned char *md);
};


class QT_OS_EXPORT IExtAesCrypto
{
public:
	static QtResult CryptoDataEx(const LPBYTE lpInData,
							     int iInDataLength,
							     const LPBYTE lpKey,
							     int iKeyLenght,
							     BOOL bEncrypt,
							     LPBYTE& lpOutData,
							     int& iOutDataLength,
							     BOOL bStream);

private:
	// block
	static QtResult CryptoDataBlock(const LPBYTE lpInData,
							        int	iInDataLength,
							        const LPBYTE lpKey,
							        int iKeyLength,
							        BOOL bEncrypt,
							        LPBYTE& lpOutData,
							        int& iOutDataLength);

	// stream
	static QtResult CryptoDataStream(const LPBYTE lpInData,
							         int iInDataLength,
							         const LPBYTE lpKey,
							         int iKeyLength,
							         BOOL bEncrypt,
							         LPBYTE& lpOutData,
							         int& iOutDataLength);

};
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

#endif//~__QTEC_CRYPTO_H__
