
#include "QtBase.h"

#if defined QT_WIN32 || defined QT_PORT_CLIENT
const char * QT_strcaserstr(const char *big, const char *little)
{
    const char *p;
    size_t ll;

    if( ((const char *)0 == big) || ((const char *)0 == little) ) return (char *)0;
    if( ((char)0 == *big) || ((char)0 == *little) ) return NULL;

    ll = strlen(little);
    p = &big[ strlen(big) - ll ];
    if( p < big ) return NULL;

    for( ; p >= big; p-- )
        /* obvious improvement available here */
            if( 0 == strncasecmp(p, little, ll) )
                return (char *)p;

    return NULL;
}

#if 0 //will put the codes into next SP (SP8?)

#ifdef QT_MMP
#include "cmssl.h"
#else
#include <openssl/evp.h>
#endif

void QT_Base64Decode(const char *bufInput, CQtString &sbDest)
{
    int nbytesdecoded;
    const char *bufin = bufInput;
    int nprbytes;

    /* trip leading whitespace first. */
    while(*bufInput==' ' || *bufInput == '\t') bufInput++;

    /* count how many characters are in the input buffer. */
    bufin = bufInput;
    while(*(bufin++)!= '\0'){}

	// count the valid character in the buffer
    nprbytes = bufin - bufInput - 1;
    
	//count the buffer that decoded buffer needed
	nbytesdecoded = ((nprbytes+ 3 ) / 4) * 3;

	sbDest.resize(0);
	sbDest.resize(nbytesdecoded);
    unsigned char *bufout = reinterpret_cast<unsigned char *>(const_cast<char*>(sbDest.c_str()));
    bufin = bufInput;

	EVP_ENCODE_CTX	ctx;
	int iResult, iTmpLen;
#ifdef QT_MMP
	CQTSSL::QT_EVP_DecodeInit(&ctx);
	CQTSSL::QT_EVP_DecodeUpdate(&ctx, (unsigned char *)bufout, &iResult, (unsigned char *)bufin, nprbytes);
	CQTSSL::QT_EVP_DecodeFinal(&ctx, (unsigned char *)&bufout[iResult], &iTmpLen);
#else
	EVP_DecodeInit(&ctx);
	EVP_DecodeUpdate(&ctx, (unsigned char *)bufout, &iResult, (unsigned char *)bufin, nprbytes);
	EVP_DecodeFinal(&ctx, (unsigned char *)&bufout[iResult], &iTmpLen);
#endif
	QT_ASSERTE(iResult + iTmpLen <= sbDest.length());
	sbDest.resize(iResult + iTmpLen);
}

void QT_Base64Encode(const unsigned char *bufin, unsigned long nbytes, CQtString &sbDest)
{
   int iResult;


   unsigned long nLen = nbytes + ((nbytes + 3) / 3) + 4;
   sbDest.resize(0);
   sbDest.resize(nLen + nLen / 64);

   unsigned char *szOut = reinterpret_cast<unsigned char *>(const_cast<char*>(sbDest.c_str()));
   EVP_ENCODE_CTX	ctx;
#ifdef QT_MMP
   CQTSSL::QT_EVP_EncodeInit(&ctx);
   CQTSSL::QT_EVP_EncodeUpdate(&ctx, szOut, &iResult, (unsigned char *)bufin, nbytes);
   CQTSSL::QT_EVP_EncodeFinal(&ctx, (unsigned char *)&szOut[iResult], &iResult);
#else
   EVP_EncodeInit(&ctx);
   EVP_EncodeUpdate(&ctx, szOut, &iResult, (unsigned char *)bufin, nbytes);
   EVP_EncodeFinal(&ctx, (unsigned char *)&szOut[iResult], &iResult);
#endif

   QT_ASSERTE((iResult + sizeof(iResult)) <= sbDest.length());
   if (iResult + sizeof(iResult) < sbDest.length())
	   sbDest.resize(iResult + sizeof(iResult));
}

#else

/************************************************************
 *    uuencode/decode functions
 ************************************************************/
//
//  Taken from NCSA HTTP and wwwlib.
//
//  NOTE: These conform to RFC1113, which is slightly different then the Unix
//        uuencode and uudecode!
//
const int pr2six[256]={
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,62,64,64,64,63,
    52,53,54,55,56,57,58,59,60,61,64,64,64,64,64,64,64,0,1,2,3,4,5,6,7,8,9,
    10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,64,64,64,64,64,64,26,27,
    28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64
};

char six2pr[64] = {
    'A','B','C','D','E','F','G','H','I','J','K','L','M',
    'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
    'a','b','c','d','e','f','g','h','i','j','k','l','m',
    'n','o','p','q','r','s','t','u','v','w','x','y','z',
    '0','1','2','3','4','5','6','7','8','9','+','/'
};


void QT_Base64Decode(const char *bufInput, CQtString &sbDest)
{
    int nbytesdecoded;
    const char *bufin = bufInput;
    unsigned char *bufout;
    int nprbytes;

    /* Strip leading whitespace. */

    while(*bufInput==' ' || *bufInput == '\t') bufInput++;

    /* Figure out how many characters are in the input buffer.
     * If this would decode into more bytes than would fit into
     * the output buffer, adjust the number of input bytes downwards.
     */
    bufin = bufInput;
    while(pr2six[(int)*(bufin++)] <= 63){}
    nprbytes = bufin - bufInput - 1;
    nbytesdecoded = ((nprbytes+3)/4) * 3;

	sbDest.resize(0);
	sbDest.resize(nbytesdecoded);
    bufout = reinterpret_cast<unsigned char *>(const_cast<char*>(sbDest.c_str()));
    bufin = bufInput;

    while (nprbytes > 0) {
        *(bufout++) =
            (unsigned char) (pr2six[(int)*bufin] << 2 | pr2six[(int)bufin[1]] >> 4);
        *(bufout++) =
            (unsigned char) (pr2six[(int)bufin[1]] << 4 | pr2six[(int)bufin[2]] >> 2);
        *(bufout++) =
            (unsigned char) (pr2six[(int)bufin[2]] << 6 | pr2six[(int)bufin[3]]);
        bufin += 4;
        nprbytes -= 4;
    }

    if(nprbytes & 03) {
        if(pr2six[(int)bufin[-2]] > 63)
            nbytesdecoded -= 2;
        else
            nbytesdecoded -= 1;
		sbDest.resize(nbytesdecoded);
    }
}

void QT_Base64Encode(const unsigned char *bufin, unsigned long nbytes, CQtString &sbDest)
{
   unsigned char *outptr;
   unsigned int i;

   //
   //  Resize the buffer to 133% of the incoming data
   //
   sbDest.resize(0);
   sbDest.resize(nbytes + ((nbytes + 3) / 3) + 4);
   outptr = reinterpret_cast<unsigned char *>(const_cast<char*>(sbDest.c_str()));

   for (i=0; i<nbytes; i += 3) {
      *(outptr++) = six2pr[*bufin >> 2];            /* c1 */
      *(outptr++) = six2pr[((*bufin << 4) & 060) | ((bufin[1] >> 4) & 017)]; /*c2*/
      *(outptr++) = six2pr[((bufin[1] << 2) & 074) | ((bufin[2] >> 6) & 03)];/*c3*/
      *(outptr++) = six2pr[bufin[2] & 077];         /* c4 */

      bufin += 3;
   }

   /* If nbytes was not a multiple of 3, then we have encoded too
    * many characters.  Adjust appropriately.
    */
   if(i == nbytes+1) {
      /* There were only 2 bytes in that last group */
      outptr[-1] = '=';
   } else if(i == nbytes+2) {
      /* There was only 1 byte in that last group */
      outptr[-1] = '=';
      outptr[-2] = '=';
   }

   *(outptr++) = '\0';

   size_t len = outptr - 1 -
	   reinterpret_cast<unsigned char *>(const_cast<char*>(sbDest.c_str()));
   QT_ASSERTE(len <= sbDest.length());
   if (len < sbDest.length())
	   sbDest.resize(len);
}
#endif //!0
#endif //QT_WIN32 || QT_PORT_CLIENT

#ifdef QT_WIN32

#include <tchar.h>
HANDLE g_hInstDll;
HANDLE GetTPDllHandle()
{
	return g_hInstDll;
}

CQtString GetModuelPath()
{
	CQtString strPath;
	TCHAR szFull[_MAX_PATH];
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	if (0 == ::GetModuleFileName((HINSTANCE)g_hInstDll, szFull, sizeof(szFull)/sizeof(TCHAR)))
	{
		QT_ERROR_TRACE("::GetModuelPath failed, errno = " << ::GetLastError());
		return strPath;
	}
	_tsplitpath(szFull, szDrive, szDir, NULL, NULL);
	_tcscpy(szFull, szDrive);
	_tcscat(szFull, szDir);
	strPath = CQtString(szFull);
	return strPath;	
}

#endif

/*! the definition could be found in QtDebug.h */
namespace wbx{
#ifdef _TRACE_OBJECT
	IObject::Object_G_Map *IObject::m_pObjectMap;/*! all IObject will be registered here when constructed and unregistered when destroyed */
#endif
	void ObjectTracer()
	{
	}
}

