#include "QtKeyDistributeService.h"

void PrintBuffer(const char *pBuff1, int nLen)
{
	char *pBuff2 = new char[nLen*2+1];
	
	char *pTransform = "0123456789abcdef";
	for(int i=0; i<nLen; i++)
	{
		BYTE first4 = (pBuff1[i]&0xf0) >> 4;
		BYTE last4 = (pBuff1[i]&0x0f);
		pBuff2[i*2] = pTransform[first4];
		pBuff2[i*2+1] = pTransform[last4];
	}

	pBuff2[nLen*2] = 0;

	printf("%s\n", pBuff2);
	QT_INFO_TRACE(pBuff2);
	
	delete[] pBuff2;
	
}

void Change(char s[],char bits[]) 
{
    int i,n = 0;
    for(i = 0; s[i]; i += 2) {
        if(s[i] >= 'A' && s[i] <= 'F')
            bits[n] = s[i] - 'A' + 10;
        else bits[n] = s[i] - '0';
        if(s[i + 1] >= 'A' && s[i + 1] <= 'F')
            bits[n] = (bits[n] << 4) | (s[i + 1] - 'A' + 10);
        else bits[n] = (bits[n] << 4) | (s[i + 1] - '0');
        ++n;
    }
    return;
}

int main()
{
	CQtKeyDistribute distribute;
	char bits[32];
	Change("65001200052000001212121212121212",bits);

	PRINT("begin deal synreq\n");
	PrintBuffer(bits, 16);
	distribute.DealClientSynReq((const char*)bits, 32);

	PRINT("begin deal push synreq\n");
	PrintBuffer(bits, 16);
	distribute.DealClientPushReq((const char*)bits, 32);
	
	return 0;
}

