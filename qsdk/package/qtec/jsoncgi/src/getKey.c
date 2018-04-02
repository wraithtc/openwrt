#include "basic.h"

//重新生成私钥和公钥，且 获取公钥
void proc_rsacfg_getKey(cJSON *jsonValue,cJSON *jsonOut)
{
    DEBUG_PRINTF("===[%s]=========\n",__func__);
    int ret=0;
    RSA *rsa = NULL;
    FILE *fp=NULL;
    cJSON* obj = cJSON_CreateObject();

    FILE *lock_fp=NULL;
    lock_fp=fopen("/tmp/.rsa_key_lock","r");
    while(lock_fp!=NULL)
    {
        fclose(lock_fp);
        lock_fp=NULL;
        sleep(1);
        lock_fp=fopen("/tmp/.rsa_key_lock","r");
    }
    fp = fopen(RSA_PUBLIC_KEY_FILE,"r");
    if(fp == NULL)
    {
        system("touch /tmp/.rsa_key_lock");
        ret = create_key();
        system("rm -rf /tmp/.rsa_key_lock");
        if(ret !=0)
        {
            DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__);
            //cJSON_AddItemToObject(jsonOut, "code", cJSON_CreateNumber(1));	
	        //cJSON_AddItemToObject(jsonOut, "msg", cJSON_CreateString("create rsa key fail"));
            global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
            return;
        }
       
        fp=fopen(RSA_PUBLIC_KEY_FILE,"r");
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

  
    
    char result[2048]={0};
    char buf[256]={0};
    fgets(buf,256,fp);
    memset(buf,0,256);
    int i=0;
    while((fgets(buf,256,fp)!=NULL) &&(i<4))
    {
        //DEBUG_PRINTF("===buf:%s====",buf);
        buf[strlen(buf)-1]='\0';
        strcat(result,buf);
        memset(buf,0,256);
        i++;
    }

   // DEBUG_PRINTF("===result:%s====\n",result);
    

    if(fp == NULL)
    {
        DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__);
        global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
        return;
    }

    cJSON_AddItemToObject(jsonOut, "data", obj);
    cJSON_AddItemToObject(obj,"rand_key",cJSON_CreateString(result));
    fclose(fp);
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