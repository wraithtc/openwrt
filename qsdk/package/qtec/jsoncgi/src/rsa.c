
#include "basic.h"


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

    private_file = fopen(RSA_PRIVATE_KEY_FILE,"w+");

    if(private_file == NULL)
    {
        DEBUG_PRINTF("==[%s]=====",RSA_PRIVATE_KEY_FILE,"open fail",__func__);
        return -1;
    }
    
    public_file=fopen(RSA_PUBLIC_KEY_FILE,"w+");
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

    return p_en;
}



char* my_decrypt(int in_len, char *str,char *path_key){
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
    DEBUG_PRINTF("===[%s]=== rsa_len: %d====\n",__func__,rsa_len);
//   if(RSA_private_decrypt(rsa_len,(unsigned char *)str,(unsigned char*)p_de,p_rsa,RSA_NO_PADDING)<0){

    if( RSA_private_decrypt(in_len,(unsigned char *)str,(unsigned char*)p_de,p_rsa,RSA_PKCS1_PADDING)<0){
        printf("===[%s] RSA_PKCS1_PADDING decrypt fail====\n",__func__);
        return NULL;
    }
 
 
    RSA_free(p_rsa);
    fclose(file);
    return p_de;
}








