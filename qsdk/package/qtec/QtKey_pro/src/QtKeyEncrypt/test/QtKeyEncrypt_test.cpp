#include "QtKeyEncrypt.h"


int main()
{
	int nRet = 0;
	int i;
	CQtKeyEncrypt *qkencrypt = new CQtKeyEncrypt();
	unsigned char key[16] = {0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,0xfe,0xdc,0xba,0x98,0x76,0x54,0x32,0x10};
	unsigned char input[ENCRYPT_INPUT_LEN] = {0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,0xfe,0xdc,0xba,0x98,0x76,0x54,0x32,0x10};
	unsigned char output[ENCRYPT_OUTPUT_LEN];
	unsigned char output2[ENCRYPT_OUTPUT_LEN];
	int destLen;

	PRINT("enc input:\n");
	for(i=0;i<16;i++)
		PRINT("%02x ", input[i]);
	PRINT("\n");
	
	if(0 != qkencrypt->EncryptOrDecrypt(output, destLen, input, ENCRYPT_INPUT_LEN, SM4_ECB_ENC, key))
	{
		PRINT("qkencrypt.EncryptOrDecrypt error\n");
		return 0;
	}

	PRINT("enc output:\n");
	for(i=0;i<16;i++)
		PRINT("%02x ", output[i]);
	PRINT("\n");

	if(0 != qkencrypt->EncryptOrDecrypt(output2, destLen, output, ENCRYPT_INPUT_LEN, SM4_ECB_DEC, key))
	{
		PRINT("qkencrypt.EncryptOrDecrypt error\n");
		return 0;
	}
	
	PRINT("dec output:\n");
	for(i=0;i<16;i++)
		PRINT("%02x ", output2[i]);
	PRINT("\n");
	
	return 0;
}

