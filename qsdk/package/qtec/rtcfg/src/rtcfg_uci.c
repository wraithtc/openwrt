#include "rtcfg_uci.h"
#include "string.h"

/**
 * rtcfgUciGet: get the value of one option or the type of one section
 * @cmd: the pathname of option or section forexample: network.wan.gateway
 * @value: the value of option or the type of section
 * 
 */
int rtcfgUciGet(const char *cmd, char *value)
{
    struct uci_ptr rtcfg_ptr;
    struct uci_element *e=NULL;
    struct uci_context *rtcfg_ctx=NULL;
    
    char *tmp=strdup(cmd);
    int result = UCI_OK;
    int dummy;
    
    if(cmd==NULL)
    {
        printf("%s: ERROR ptr_name can't be null\n",__func__);
        return 1;
    }
    
    rtcfg_ctx=uci_alloc_context();
    
    if(uci_lookup_ptr(rtcfg_ctx, &rtcfg_ptr, tmp, true) != UCI_OK)
    {
    
        printf(" %s: ERROR cmd:%s \n",__func__,cmd);
        result= 1;
        goto out;
    }
	
    if(rtcfg_ptr.value)
    {
        printf(" %s: ERROR cmd:%s \n", __func__,cmd); 
        result= 1;
        goto out;
    }
    
    e=rtcfg_ptr.last;
    
    if(!(rtcfg_ptr.flags & UCI_LOOKUP_COMPLETE))
    {
        rtcfg_ctx->err = UCI_ERR_NOTFOUND;
        printf("%s: ERROR cmd : %s can't find value\n", __func__,cmd);
        result =1;
        goto out;
    }
    
    struct uci_element *new_e=NULL;
    bool sep=false;
    switch(e->type){
        case UCI_TYPE_SECTION:
            sprintf(value,"%s",rtcfg_ptr.s->type);
            break;
        case UCI_TYPE_OPTION:
            //uci_show_value(ptr.o)
            switch(rtcfg_ptr.o->type){
                case UCI_TYPE_STRING:
                    sprintf(value,"%s",rtcfg_ptr.o->v.string);
                    break;
                case UCI_TYPE_LIST:
                    
                    uci_foreach_element(&rtcfg_ptr.o->v.list, new_e) {
                        char tmp_char[16]={0};
                        sprintf(tmp_char,"%s%s", (sep ? " ":""),new_e->name);
                        strcat(value,tmp_char);
                        sep=true;
                    }
                    break;
                default:
                    sprintf(value,"<unknown>");
                    break;
            }
            break;
        default:
            printf("%s: cmd:%s is not section and not option\n",__func__,cmd);
            break;

    }
out:
    uci_free_context(rtcfg_ctx);
    rtcfg_ctx=NULL;
    free(tmp);
    return result;
}

/*
 * rtcfgUciSet: set an element's value
 * @cmd: the cmd contain the pathname of an element and the value forexample: network.wan.gateway=10.0.0.1
 */

int rtcfgUciSet(const char *cmd)
{
    struct uci_ptr rtcfg_ptr;
    struct uci_element *e=NULL;
    struct uci_context *rtcfg_ctx=NULL;
    
    char *tmp=strdup(cmd);
    int result = UCI_OK;
    int dummy;
    
    if(cmd==NULL)
    {
        printf("%s: ERROR ptr_name can't be null\n",__func__);
        return 1;
    }
    
    rtcfg_ctx=uci_alloc_context();
    
    if(uci_lookup_ptr(rtcfg_ctx, &rtcfg_ptr, tmp, true) != UCI_OK)
    {
    
        printf(" %s: ERROR cmd:%s \n",__func__,cmd);
        result= 1;
        goto out;
    }
    
    printf("%s: debug: rtcfg_ptr.value: %s ===\n",__func__,rtcfg_ptr.value);
    result=uci_set(rtcfg_ctx,&rtcfg_ptr);
    
    /*save chages, but don't commit them yet */
    if(UCI_OK == result)
        result = uci_save(rtcfg_ctx, rtcfg_ptr.p);

    if(result != UCI_OK)
    {
        printf("%s:Error can't save cmd: %s\n", __func__, cmd);
        result=1;
    }

out:
    uci_free_context(rtcfg_ctx);
    rtcfg_ctx=NULL;
    free(tmp);
    return result;
}

	
/*
 * rtcfgUciAddList: append a string to an element list
 * @cmd: contain the pathname of element and the value
 */
int rtcfgUciAddList(const char *cmd)
{
    struct uci_ptr rtcfg_ptr;
    struct uci_element *e=NULL;
    struct uci_context *rtcfg_ctx=NULL;
    
    char *tmp=strdup(cmd);
    int result = UCI_OK;
    int dummy;
    
    if(cmd==NULL)
    {
        printf("%s: ERROR ptr_name can't be null\n",__func__);
        return 1;
    }
    
    rtcfg_ctx=uci_alloc_context();
    
    if(uci_lookup_ptr(rtcfg_ctx, &rtcfg_ptr, tmp, true) != UCI_OK)
    {
    
        printf(" %s: ERROR cmd:%s \n",__func__,cmd);
        result= 1;
        goto out;
    }
    
    printf("%s: debug: ptr.value: %s ===\n",__func__,rtcfg_ptr.value);
    result=uci_add_list(rtcfg_ctx,&rtcfg_ptr);
    
    /*save chages, but don't commit them yet */
    if(UCI_OK == result)
        result = uci_save(rtcfg_ctx, rtcfg_ptr.p);

    if(result != UCI_OK)
    {
        printf("%s:Error can't save cmd: %s\n", __func__, cmd);
        result=1;
    }

out:
    uci_free_context(rtcfg_ctx);
    rtcfg_ctx=NULL;
    free(tmp);
    return result;
}
	

/**
 *rtcfgUciDelList: remove a string from an element list
 *@cmd: contain pathname of element and the value
 */
int rtcfgUciDelList(const char *cmd)
{
    struct uci_ptr rtcfg_ptr;
    struct uci_element *e=NULL;
    struct uci_context *rtcfg_ctx=NULL;
    
    char *tmp=strdup(cmd);
    int result = UCI_OK;
    int dummy;
    
    if(cmd==NULL)
    {
        printf("%s: ERROR ptr_name can't be null\n",__func__);
        return 1;
    }
    
    rtcfg_ctx=uci_alloc_context();
    
    if(uci_lookup_ptr(rtcfg_ctx, &rtcfg_ptr, tmp, true) != UCI_OK)
    {
    
        printf(" %s: ERROR cmd:%s \n",__func__,cmd);
        result= 1;
        goto out;
    }
    
    printf("%s: debug: rtcfg_ptr.value: %s ===\n",__func__,rtcfg_ptr.value);
    result=uci_del_list(rtcfg_ctx,&rtcfg_ptr);
    
    /*save chages, but don't commit them yet */
    if(UCI_OK == result)
        result = uci_save(rtcfg_ctx, rtcfg_ptr.p);

    if(result != UCI_OK)
    {
        printf("%s:Error can't save cmd: %s\n", __func__, cmd);
        result=1;
    }

out:
    uci_free_context(rtcfg_ctx);
    rtcfg_ctx=NULL;
    free(tmp);
    return result;
}

/**
 *  rtcfgUciAdd: add a section into one config
 *  @config_name: the name of config where we add a section
 *  @section_name: the section which we add
 */

int rtcfgUciAdd(const char *config_name, const char *section_name)
{
    struct uci_package *p=NULL;
    struct uci_section *s=NULL;
    struct uci_context *rtcfg_ctx=NULL;
    
    int ret=0;

    if( (config_name == NULL) || (section_name == NULL) )
    {
        printf("===%s=== argv error !!!=====\n",__func__);
        return 1;
    }
    char *tmp_config=strdup(config_name);
    char *tmp_section=strdup(section_name);
    
    rtcfg_ctx=uci_alloc_context();
    ret=uci_load(rtcfg_ctx,tmp_config,&p);
    if(ret !=UCI_OK)
        goto out;
    
    ret = uci_add_section(rtcfg_ctx,p,section_name,&s);
    if(ret != UCI_OK)
        goto out;
    
    ret = uci_save(rtcfg_ctx,p);

out:
    if(ret !=UCI_OK)
    {
        printf("===%s=== add section %s in config %s fail==\n",__func__,section_name,config_name);
    }
    uci_free_context(rtcfg_ctx);
    rtcfg_ctx=NULL;
    free(tmp_config);
    free(tmp_section);
    return ret;
}

/**
 *  rtcfgUciDel: Delete a section or option
 *  @cmd: the pathname of element
 */
int rtcfgUciDel(const char *cmd)
{
    struct uci_ptr rtcfg_ptr;
    struct uci_element *e=NULL;
    struct uci_context *rtcfg_ctx=NULL;
    
    char *tmp=strdup(cmd);
    int result = UCI_OK;
    int dummy;
    
    if(cmd==NULL)
    {
        printf("%s: ERROR ptr_name can't be null\n",__func__);
        return 1;
    }
    
    rtcfg_ctx=uci_alloc_context();
    
    if(uci_lookup_ptr(rtcfg_ctx, &rtcfg_ptr, tmp, true) != UCI_OK)
    {
    
        printf(" %s: ERROR cmd:%s \n",__func__,cmd);
        result= 1;
        goto out;
    }
    
    printf("%s: debug: rtcfg_ptr.value: %s ===\n",__func__,rtcfg_ptr.value);
    if(rtcfg_ptr.value && !sscanf(rtcfg_ptr.value, "%d", &dummy))
    {
        result = 1;
        goto out;
    }
    result=uci_delete(rtcfg_ctx,&rtcfg_ptr);

    /*save chages, but don't commit them yet */
    if(UCI_OK == result)
        result = uci_save(rtcfg_ctx, rtcfg_ptr.p);

    if(result != UCI_OK)
    {
        printf("%s:Error can't save cmd: %s\n", __func__, cmd);
        result=1;
    }

out:
    uci_free_context(rtcfg_ctx);
    rtcfg_ctx=NULL;
    free(tmp);
    return result;
}

/**
 * rtcfgUciCommit: commit changes to package
 * @cmd : the name of package 
 */
int rtcfgUciCommit(const char *cmd)
{
    struct uci_ptr rtcfg_ptr;
    struct uci_element *e=NULL;
    struct uci_context *rtcfg_ctx=NULL;
    
    char *tmp=strdup(cmd);
    int result = UCI_OK;
    int dummy;
    
    if(cmd==NULL)
    {
        printf("%s: ERROR ptr_name can't be null\n",__func__);
        return 1;
    }
    
    rtcfg_ctx=uci_alloc_context();
    
    if(uci_lookup_ptr(rtcfg_ctx, &rtcfg_ptr, tmp, true) != UCI_OK)
    {
    
        printf(" %s: ERROR cmd:%s \n",__func__,cmd);
        result= 1;
        goto out;
    }
   
    if( uci_commit(rtcfg_ctx,&rtcfg_ptr.p,false) !=UCI_OK )
    {
        printf("%s: Error uci_commit fail===\n", __func__);
        result=1;
        goto out;
    }
    
    if(rtcfg_ptr.p)
        uci_unload(rtcfg_ctx, rtcfg_ptr.p);

out:
    uci_free_context(rtcfg_ctx);
    rtcfg_ctx=NULL;
    free(tmp);
    return result;
}






