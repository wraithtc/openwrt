#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include "cJSON.h"
#include <librtcfg.h>
#include <time.h>
#include <openssl/md5.h>

#define MAX_TOKENID_LEN 129

#define RSA_PRIVATE_KEY_FILE "/etc/rsa_private.key"
#define RSA_PUBLIC_KEY_FILE "/etc/rsa_public.key"

const char * base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// 测试用的函数
#define DEBUG 1
#ifdef DEBUG 
#define DEBUG_PRINTF(format,...)   printf(format, ##__VA_ARGS__); fflush(stdout);
#else
#define DEBUG_PRINTF(format,...)
#endif


int generatortokenid(char *tokenid)
{
    srand((int)time(0));
    int i =rand()%1000;
    //计算MD5值
    MD5_CTX ctx;
    unsigned char md[16];
    char buf[33]={'\0'};
    char tmp[3]={'\0'};
    MD5_Init(&ctx);
    MD5_Update(&ctx,&i,sizeof(i));
    MD5_Final(md,&ctx);

    for(i=0;i<16;i++){
        sprintf(tmp,"%02x",md[i]);
        strcat(buf,tmp);
    }
  
    snprintf(tokenid,MAX_TOKENID_LEN,"%s",buf);
  
    return 0;
}

char * base64_encode( const unsigned char * bindata, char * base64, int binlength )
{
    int i, j;
    unsigned char current;

    for ( i = 0, j = 0 ; i < binlength ; i += 3 )
    {
        current = (bindata[i] >> 2) ;
        current &= (unsigned char)0x3F;
        base64[j++] = base64char[(int)current];

        current = ( (unsigned char)(bindata[i] << 4 ) ) & ( (unsigned char)0x30 ) ;
        if ( i + 1 >= binlength )
        {
            base64[j++] = base64char[(int)current];
            base64[j++] = '=';
            base64[j++] = '=';
            break;
        }
        current |= ( (unsigned char)(bindata[i+1] >> 4) ) & ( (unsigned char) 0x0F );
        base64[j++] = base64char[(int)current];

        current = ( (unsigned char)(bindata[i+1] << 2) ) & ( (unsigned char)0x3C ) ;
        if ( i + 2 >= binlength )
        {
            base64[j++] = base64char[(int)current];
            base64[j++] = '=';
            break;
        }
        current |= ( (unsigned char)(bindata[i+2] >> 6) ) & ( (unsigned char) 0x03 );
        base64[j++] = base64char[(int)current];

        current = ( (unsigned char)bindata[i+2] ) & ( (unsigned char)0x3F ) ;
        base64[j++] = base64char[(int)current];
    }
    base64[j] = '\0';
    return base64;
}


int base64_decode( const char * base64, unsigned char * bindata )
{
    int i, j;
    unsigned char k;
    unsigned char temp[4];
    for ( i = 0, j = 0; base64[i] != '\0' ; i += 4 )
    {
        memset( temp, 0xFF, sizeof(temp) );
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i] )
                temp[0]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i+1] )
                temp[1]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i+2] )
                temp[2]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i+3] )
                temp[3]= k;
        }

        bindata[j++] = ((unsigned char)(((unsigned char)(temp[0] << 2))&0xFC)) |
                ((unsigned char)((unsigned char)(temp[1]>>4)&0x03));
        if ( base64[i+2] == '=' )
            break;

        bindata[j++] = ((unsigned char)(((unsigned char)(temp[1] << 4))&0xF0)) |
                ((unsigned char)((unsigned char)(temp[2]>>2)&0x0F));
        if ( base64[i+3] == '=' )
            break;

        bindata[j++] = ((unsigned char)(((unsigned char)(temp[2] << 6))&0xF0)) |
                ((unsigned char)(temp[3]&0x3F));
    }
    return j;
}

/************************************************************************
 * RSA密钥生成函数
 *
 * file: test_rsa_genkey.c
 * gcc -Wall -O2 -o test_rsa_genkey test_rsa_genkey.c -lcrypto
 *
 * author: tonglulin@gmail.com by www.qmailer.net
 ************************************************************************/
int create_key()
{
    FILE *private_file=NULL;
    FILE *public_file=NULL;

    private_file = fopen("/etc/rsa_private.key","w+");

    if(private_file == NULL)
    {
        DEBUG_PRINTF("==[%s]=====",RSA_PRIVATE_KEY_FILE,"open fail",__func__);
        return -1;
    }
    
    public_file=fopen("/etc/rsa_public.key","w+");
    if(public_file == NULL)
    {
        DEBUG_PRINTF("===[%s]====",RSA_PUBLIC_KEY_FILE,"open fail", __func__);
        return -1;
    }
    
    /* 产生RSA密钥 */
    RSA *rsa = RSA_generate_key(1024, 65537, NULL, NULL);
 
    DEBUG_PRINTF("[%s]: BIGNUM: %s\n", __func__, BN_bn2hex(rsa->n));
 
    /* 提取私钥 */
    DEBUG_PRINTF("[%s]: PRIKEY:\n", __func__);
    PEM_write_RSAPrivateKey(stdout, rsa, NULL, NULL, 0, NULL, NULL);
    PEM_write_RSAPrivateKey(private_file, rsa, NULL, NULL, 0, NULL, NULL);
    
    /* 提取公钥 */
    char* n_b = (unsigned char*)calloc(RSA_size(rsa),sizeof(char));
    char* e_b = (unsigned char*)calloc(RSA_size(rsa),sizeof(char));
 
    int n_size = BN_bn2bin(rsa->n, n_b);
    int b_size = BN_bn2bin(rsa->e, e_b);
 
    RSA *pubrsa = RSA_new();
    pubrsa->n = BN_bin2bn(n_b, n_size, NULL);
    pubrsa->e = BN_bin2bn(e_b, b_size, NULL);
 
    printf("[%s]: PUBKEY: \n",__func__);
    //PEM_write_RSAPublicKey(stdout, pubrsa);
    //PEM_write_RSAPublicKey(public_file, pubrsa);
    PEM_write_RSA_PUBKEY(stdout,pubrsa);
    PEM_write_RSA_PUBKEY(public_file,pubrsa);
    
    RSA_free(rsa);
    RSA_free(pubrsa);

    fclose(private_file);
    fclose(public_file);
    return 0;
}


void proc_rsacfg_getKey(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("===[%s]=========\n",__func__);
    int ret=0;
    RSA *rsa = NULL;
    FILE *fp=NULL;
  
    ret = create_key();
    if(ret !=0)
    {
        DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__);
        cJSON_AddItemToObject(jsonOut, "code", cJSON_CreateNumber(1));	
	    cJSON_AddItemToObject(jsonOut, "msg", cJSON_CreateString("create rsa key fail"));
        return;
    }
    /*
     *format
     *-----BEGIN PUBLIC KEY-----
        MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCoaO3ODknWwcxlRfiFhzbLmBkX
        fxfHZiG8j9qQ5YAQwCduFdu4gS+EtZ+FyJWq4nQX5GfeNe7IoSeX5BYhCdh+Ls7i
        Zb1tHEAS+SaYa/sY8JJO0dCDocKvbjJ+onk32tudYjClFjtJ31nESk8aWc1B8OLZ
        Ryvr4ZFcll5Jf+gvSwIDAQAB
      -----END PUBLIC KEY-----
     *
     *
     */

  
    fp = fopen(RSA_PUBLIC_KEY_FILE,"r");
    char result[2048]={0};
    char buf[256]={0};
    fgets(buf,256,fp);
    memset(buf,0,256);
    int i=0;
    while((fgets(buf,256,fp)!=NULL) &&(i<4))
    {
        DEBUG_PRINTF("===buf:%s====",buf);
        buf[strlen(buf)-1]='\0';
        strcat(result,buf);
        memset(buf,0,256);
        i++;
    }

    DEBUG_PRINTF("===result:%s====\n",result);
    

    if(fp == NULL)
    {
        DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__);
        cJSON_AddItemToObject(jsonOut, "code", cJSON_CreateNumber(1));	
	    cJSON_AddItemToObject(jsonOut, "msg", cJSON_CreateString("can not read public key file"));
        return;
    }

    cJSON_AddItemToObject(jsonOut,"rand_key",cJSON_CreateString(result));
    cJSON_AddItemToObject(jsonOut, "code", cJSON_CreateNumber(0));	
	cJSON_AddItemToObject(jsonOut, "msg", cJSON_CreateString("success"));
    
   #if 0
    //读取RSA key 
   // rsa = PEM_read_RSAPublicKey(fp,&rsa,NULL,NULL);
    rsa= PEM_read_RSA_PUBKEY(fp,&rsa,NULL,NULL);
    if( rsa == NULL)
    {
        DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__);
        cJSON_AddItemToObject(jsonOut, "code", cJSON_CreateNumber(1));	
	    cJSON_AddItemToObject(jsonOut, "msg", cJSON_CreateString("public key is null"));
    }
    else
    {   
        cJSON *obj = NULL;
        obj = cJSON_CreateObject();
        cJSON_AddItemToObject(jsonOut, "data", obj);
        DEBUG_PRINTF("===[%s]:  [%d]===\n",__func__,__LINE__);
        RSA_print_fp(stdout,rsa,0);
        fflush(stdout);
        DEBUG_PRINTF("%s\n", BN_bn2hex(rsa->n));  
        DEBUG_PRINTF("%s\n", BN_bn2hex(rsa->e));

        //尝试base64 加码rsa
     
        char base64[2048]={0};
        base64_encode(rsa, base64, sizeof(RSA) );
        printf("====base64: %s  ====\n", base64);
 
        cJSON_AddItemToObject(obj,"rand_key1",cJSON_CreateString(BN_bn2hex(rsa->n)));
        cJSON_AddItemToObject(obj,"rand_key2",cJSON_CreateString(BN_bn2hex(rsa->e)));
        cJSON_AddItemToObject(jsonOut, "code", cJSON_CreateNumber(0));	
	    cJSON_AddItemToObject(jsonOut, "msg", cJSON_CreateString("success"));
    }
#endif
}


char *my_encrypt(char *str,char *path_key, int *p_len){
    DEBUG_PRINTF("===[%s] wjj2===str:%s====\n",__func__,str);
    char *p_en;
    RSA *p_rsa;
    FILE *file;
    int flen,rsa_len;
    if((file=fopen(path_key,"r"))==NULL){
        perror("open key file error");
        return NULL;    
    }   
 //   if((p_rsa=PEM_read_RSAPublicKey(file,NULL,NULL,NULL))==NULL){
    if((p_rsa=PEM_read_RSA_PUBKEY(file,NULL,NULL,NULL))==NULL){
        ERR_print_errors_fp(stdout);
        fflush(stdout);
        return NULL;
    }   
    flen=strlen(str);
    rsa_len=RSA_size(p_rsa);
    p_en=(unsigned char *)malloc(rsa_len+1);
    
    memset(p_en,0,rsa_len+1);
 
    int n=RSA_public_encrypt(flen,(unsigned char *)str,(unsigned char*)p_en,p_rsa,RSA_PKCS1_PADDING);
    if( n<0){
        DEBUG_PRINTF("===[%s]====[%d] can't encrypt===\n",__func__,__LINE__);
        return NULL;
    }
    RSA_free(p_rsa);
    fclose(file);

    *p_len = n;
    printf("====n:%d ====\n",n);
    char base64[2048]={0};
    base64_encode(p_en, base64, n);
    printf("====base64: %s  ====\n", base64);
    return p_en;
}



char *my_decrypt(char *str,char *path_key){
    char *p_de;
    RSA *p_rsa;
    FILE *file;
    int rsa_len;
    
    if((file=fopen(path_key,"r"))==NULL){
        perror("open key file error");
        return NULL;
    }
    if((p_rsa=PEM_read_RSAPrivateKey(file,NULL,NULL,NULL))==NULL){
        ERR_print_errors_fp(stdout);
        return NULL;
    }
    rsa_len=RSA_size(p_rsa);
    p_de=(unsigned char *)malloc(rsa_len+1);
    memset(p_de,0,rsa_len+1);
//   if(RSA_private_decrypt(rsa_len,(unsigned char *)str,(unsigned char*)p_de,p_rsa,RSA_NO_PADDING)<0){
    if(RSA_private_decrypt(rsa_len,(unsigned char *)str,(unsigned char*)p_de,p_rsa,RSA_PKCS1_PADDING)<0){
        printf("===[%s] decrypt fail====\n",__func__);
        return NULL;
    }
    RSA_free(p_rsa);
    fclose(file);
    return p_de;
}

void proc_rsacfg_encrypt(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("===[%s]===\n",__func__);
    char *str=NULL;
    str=cJSON_GetObjectItem(jsonValue, "password")?cJSON_GetObjectItem(jsonValue, "password")->valuestring:"";
    unsigned char *result=NULL;
    int result_len=0;
    result = my_encrypt(str, RSA_PUBLIC_KEY_FILE,&result_len);
    if(result == NULL)
    {
        cJSON_AddItemToObject(jsonOut, "code", cJSON_CreateNumber(1));	
	    cJSON_AddItemToObject(jsonOut, "msg", cJSON_CreateString("encrypt fail"));
    }
    else
    {
        char base64[2048]={0};
        base64_encode(result, base64, result_len);
        DEBUG_PRINTF("====base64: %s  ====\n", base64);
        
        cJSON_AddItemToObject(jsonOut, "result", cJSON_CreateString(base64));
        cJSON_AddItemToObject(jsonOut, "code", cJSON_CreateNumber(0));	
	    cJSON_AddItemToObject(jsonOut, "msg", cJSON_CreateString("success"));
    }
}

void proc_rsacfg_decrypt(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("====[%s]=====\n",__func__);
    char *password=NULL;
    password =cJSON_GetObjectItem(jsonValue, "password")?cJSON_GetObjectItem(jsonValue, "password")->valuestring:"";

    char *username=NULL;
    username =cJSON_GetObjectItem(jsonValue, "username")?cJSON_GetObjectItem(jsonValue, "username")->valuestring:"";
    
    char *target=NULL;
    target = cJSON_GetObjectItem(jsonValue, "target")?cJSON_GetObjectItem(jsonValue, "target")->valuestring:"";
    DEBUG_PRINTF("===[%s]===str: %s ===\n",__func__,password);
    unsigned char bindata[2048]={0};
    base64_decode(password, bindata);
    
    char *result=NULL;
    result=my_decrypt(bindata, RSA_PRIVATE_KEY_FILE);
    if(result == NULL)
    {
        cJSON_AddItemToObject(jsonOut, "code", cJSON_CreateNumber(1));	
	    cJSON_AddItemToObject(jsonOut, "msg", cJSON_CreateString("decry fail"));
    }
    else
    {
        if(strncmp(target,"check",strlen(target)) ==0 )
        {
            int ret=0;
            ret=CheckGuiPassword(result);
            if(ret !=0)
            {
                cJSON_AddItemToObject(jsonOut, "code", cJSON_CreateNumber(1));	
	            cJSON_AddItemToObject(jsonOut, "msg", cJSON_CreateString("password not match"));
            }
            else
            {
                char tokenid[MAX_TOKENID_LEN]={0};
                generatortokenid(tokenid);
                setTokenId(tokenid);
                cJSON_AddItemToObject(jsonOut, "tokenid", cJSON_CreateString(tokenid));
                cJSON_AddItemToObject(jsonOut, "code", cJSON_CreateNumber(0));	
	            cJSON_AddItemToObject(jsonOut, "msg", cJSON_CreateString("success"));       
            }

        }
        else if(strncmp(target,"set",strlen(target))==0)
        {
            //在set之前需要检测老密码是否匹配
            int ret=0;
            char oldPassworld[16]={0};
            ret=GetGuiPassword(oldPassworld);
            //如果当前系统密码为空或不可取时，不检测老密码参数，这种情况应该发生在第一次配置
            if( (ret!=0) || (strlen(oldPassworld)==0) )
            {
                DEBUG_PRINTF("===[%s]====cant get current password from system, so ignore password match===\n",__func__);
            }
            else
            {
                char *oldpassword=NULL;
                oldpassword =cJSON_GetObjectItem(jsonValue, "oldpassword")?cJSON_GetObjectItem(jsonValue, "oldpassword")->valuestring:"";
                unsigned char old_bindata[2048]={0};
                base64_decode(oldpassword, old_bindata);
                char *old_result=NULL;
                old_result=my_decrypt(old_bindata, RSA_PRIVATE_KEY_FILE);
                if(old_result==NULL)
                {
                    cJSON_AddItemToObject(jsonOut, "code", cJSON_CreateNumber(1));	
	                cJSON_AddItemToObject(jsonOut, "msg", cJSON_CreateString("old password decry fail"));
                    return;
                }
                ret=CheckGuiPassword(old_result);
                if(ret !=0)
                {
                    cJSON_AddItemToObject(jsonOut, "code", cJSON_CreateNumber(1));	
	                cJSON_AddItemToObject(jsonOut, "msg", cJSON_CreateString("old password not match"));
                    return;
                }
            }
            ret=SetGuiPassword(result);
            if(ret !=0)
            {
                cJSON_AddItemToObject(jsonOut, "code", cJSON_CreateNumber(1));  
	            cJSON_AddItemToObject(jsonOut, "msg", cJSON_CreateString("setpassword fail"));
            }
            else
            {   
                char tokenid[200]={0};
                generatortokenid(tokenid);
                setTokenId(tokenid);
                cJSON_AddItemToObject(jsonOut, "tokenid", cJSON_CreateString(tokenid));
                cJSON_AddItemToObject(jsonOut, "code", cJSON_CreateNumber(0));	
	            cJSON_AddItemToObject(jsonOut, "msg", cJSON_CreateString("success"));
            }
        }
        else
        {
                cJSON_AddItemToObject(jsonOut, "code", cJSON_CreateNumber(1));  
	            cJSON_AddItemToObject(jsonOut, "msg", cJSON_CreateString("target not support"));
        }
    }
}

void decode_message(char *msg)
{
	cJSON *json, *jsonValue, *jsonOut;
	char *target, *buffer, *method;
	int  f0, f1, f2;
	
	json = cJSON_Parse(msg);
	if (!json)
	{
		DEBUG_PRINTF("[%s]: json prase error!",__func__);
		return;		
	}
	
	f0 = dup(STDOUT_FILENO);
	f1 = open("/dev/console", O_RDWR);
	f2 = dup2(f1, STDOUT_FILENO);
	close(f1);
	

	target = cJSON_GetObjectItem(json, "requestUrl")?cJSON_GetObjectItem(json, "requestUrl")->valuestring:"";
    method = cJSON_GetObjectItem(json, "method")?cJSON_GetObjectItem(json, "method")->valuestring:"";
    DEBUG_PRINTF("===target: %s===\n", target);
    DEBUG_PRINTF("===method: %s=== \n",method);
  
	jsonValue = cJSON_GetObjectItem(json, "data");
	
	jsonOut = cJSON_CreateObject();

	if (strncmp(target,"rsacfg",strlen(target))==0 )
	{
        printf("====[%s]====%d===\n",__func__,__LINE__);
        if( (strncmp(method,"get",strlen(method)) == 0) || (strncmp(method,"GET",strlen(method))==0 ) )
        {
            printf("====[%s]====%d===\n",__func__,__LINE__); fflush(stdout);
            proc_rsacfg_getKey(jsonValue,jsonOut);
        }
        else if( (strncmp(method,"test",strlen(method)) == 0) || (strncmp(method,"TEST",strlen(method))==0) )
        {
            DEBUG_PRINTF("===[%s]===[%d]====\n",__func__,__LINE__); fflush(stdout);
            proc_rsacfg_encrypt(jsonValue,jsonOut);
            
        }
        else if( (strncmp(method,"post",strlen(method))==0) || (strncmp(method,"POST",strlen(method))==0) )
        {
            proc_rsacfg_decrypt(jsonValue, jsonOut);
        }
        else
        {
            printf("====[%s]====%d===\n",__func__,__LINE__);
            cJSON_AddItemToObject(jsonOut, "code", cJSON_CreateNumber(1));	
	        cJSON_AddItemToObject(jsonOut, "msg", cJSON_CreateString("rsacfg don't support this method"));     
        }
	}
	

	fflush(stdout);
	dup2(f0, f2);
	buffer = cJSON_Print(jsonOut);
	printf("%s\n",buffer);
	free(buffer);

	cJSON_Delete(jsonOut);
	cJSON_Delete(json);
	
	return;
}

#if 1
int main()
{

	int length;
	char *method;
	char *inputstring;

	printf("Content-Type: application/json\n\n");
   // printf("Access-Control-Allow-Origin: * \n\n");
    fflush(stdout);
	method = getenv("REQUEST_METHOD"); 
  
	if(method == NULL)
	{
		return 1;   
	}
	
	//post method,read from stdin
	if(!strcmp(method, "POST")) 
	{

		length = atoi(getenv("CONTENT_LENGTH")); 
		if(length != 0)
		{
			inputstring = malloc(sizeof(char)*length + 1); 
			if (0 != fread(inputstring, sizeof(char), length, stdin)) 
			{
				decode_message(inputstring);
			}
	    }
	}

	//get method
	else if(!strcmp(method, "GET"))
	{

		inputstring = getenv("QUERY_STRING");   
		length = strlen(inputstring);

	}

	return 0;
}
#endif 

#if 0
void main()
{
   // my_encrypt("12345678", RSA_PUBLIC_KEY_FILE);
   // my_decrypt("MTIzNDU2Nzg=", RSA_PRIVATE_KEY_FILE);
   unsigned char bindata[2048]={0};
   base64_decode("MTIzNDU2Nzg=", bindata);
   printf("===bindata: %s =====\n",bindata);
   char base64[256]={0};
   base64_encode("12345678", base64, 8);
   printf("===base64:%s====\n",base64);
}
#endif 
