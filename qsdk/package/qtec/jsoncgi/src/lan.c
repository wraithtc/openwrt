#include "basic.h"

//这个文件处理lan相关事宜
int proc_landhcp_get(cJSON *json_value, cJSON *jsonOut)
{
    DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__); 
    int ret=0;
    struct lanConfig result={0};
    cJSON *obj = NULL;
	obj = cJSON_CreateObject();
    ret = lanConfigGet(&result);
    
    if(ret !=0)
    {
        printf("Fail to get dhcp cfg\n");
        return -1;
    }

    cJSON_AddItemToObject(jsonOut, "data", obj);
    cJSON_AddItemToObject(obj, "ipaddress", cJSON_CreateString(result.ipaddress));
	cJSON_AddItemToObject(obj, "netmask", cJSON_CreateString(result.netmask));
    cJSON_AddItemToObject(obj, "poolstart", cJSON_CreateString(result.dhcpPoolStart));
    cJSON_AddItemToObject(obj, "poollimit", cJSON_CreateString(result.dhcpPoolLimit));
    cJSON_AddItemToObject(obj, "dhcp_enable", cJSON_CreateNumber(result.dhcpEnable));
    return;
}

int proc_landhcp_set(cJSON *json_value, cJSON *jsonOut)
{
    DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__);
    int ret=0;
    struct lanConfig lannode={0};
    QT_GUEST_WIFI_CFG guestWifiCfg = {0};
    
    UTIL_STRNCPY(lannode.ipaddress, (cJSON_GetObjectItem(json_value, "ipaddress")?cJSON_GetObjectItem(json_value, "ipaddress")->valuestring:""), sizeof(lannode.ipaddress));
	UTIL_STRNCPY(lannode.netmask, (cJSON_GetObjectItem(json_value, "netmask")?cJSON_GetObjectItem(json_value, "netmask")->valuestring:""), sizeof(lannode.netmask));
	
    UTIL_STRNCPY(lannode.dhcpPoolStart, cJSON_GetObjectItem(json_value, "poolstart")?cJSON_GetObjectItem(json_value, "poolstart")->valuestring:"", sizeof(lannode.dhcpPoolStart));
    UTIL_STRNCPY(lannode.dhcpPoolLimit, cJSON_GetObjectItem(json_value, "poollimit")?cJSON_GetObjectItem(json_value, "poollimit")->valuestring:"", sizeof(lannode.dhcpPoolLimit));
    lannode.dhcpEnable = cJSON_GetObjectItem(json_value, "dhcp_enable")?cJSON_GetObjectItem(json_value, "dhcp_enable")->valueint:0;
    ret += lanConfigSet(&lannode);
    ret += QtGetGuestWifi(&guestWifiCfg);
    ret += QtSetGuestWifi(&guestWifiCfg);
    if(ret !=0)
    { 
	    printf("lan dhcp config set fail\n");
        return ret;
    }
    ProcLanCfgSetMsgReq();
    return ret;
}

void proc_staticlease_add(cJSON *json_value, cJSON *jsonOut)
{
    DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__); 
    int ret=0;
    struct staticLeaseConfig node={0};
    strcpy(node.hostname, (cJSON_GetObjectItem(json_value, "hostname")?cJSON_GetObjectItem(json_value, "hostname")->valuestring:""));
	strcpy(node.mac, (cJSON_GetObjectItem(json_value, "mac")?cJSON_GetObjectItem(json_value, "mac")->valuestring:""));
    strcpy(node.ipaddress, (cJSON_GetObjectItem(json_value, "ipaddress")?cJSON_GetObjectItem(json_value, "ipaddress")->valuestring:""));
	ret = addStaticLeaseEntry(&node);
   
    if(ret !=0)
    {
        cJSON_AddItemToObject(jsonOut, "code", cJSON_CreateNumber(1));  
	    cJSON_AddItemToObject(jsonOut, "msg", cJSON_CreateString("static lease add fail"));
        
    }
    else
    {
        cJSON_AddItemToObject(jsonOut, "code", cJSON_CreateNumber(0));	
	    cJSON_AddItemToObject(jsonOut, "msg", cJSON_CreateString("success"));
       
    }
    return;
    
}

void proc_staticlease_del(cJSON *json_value, cJSON *jsonOut)
{
    DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__); 
    int ret=0;
    int index;
    index = cJSON_GetObjectItem(json_value, "delindex")?cJSON_GetObjectItem(json_value, "delindex")->valueint:0;
	ret = delStaticLeaseEntry(index);
    
    if(ret !=0)
    {
        cJSON_AddItemToObject(jsonOut, "code", cJSON_CreateNumber(1));  
	    cJSON_AddItemToObject(jsonOut, "msg", cJSON_CreateString("static lease del fail"));
        
    }
    else
    {
        cJSON_AddItemToObject(jsonOut, "code", cJSON_CreateNumber(0));	
	    cJSON_AddItemToObject(jsonOut, "msg", cJSON_CreateString("success"));
       
    }
    return;
}

void proc_staticlease_getall(cJSON *json_value, cJSON *jsonOut)
{
    DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__);  
    cJSON *array = NULL;
	cJSON *obj = NULL;
    int index=0;
    int array_num=0;
    struct staticLeaseConfig *pstStaticLeaseGet = getStaticLeaseArray(&array_num);
    
    cJSON_AddItemToObject(jsonOut,"list",array=cJSON_CreateArray());
    
    
	for(index = 0; index < array_num; index++)
	{
        
        obj = cJSON_CreateObject();
	    cJSON_AddItemToObject(obj,"hostname",cJSON_CreateString(pstStaticLeaseGet[index].hostname));
		cJSON_AddItemToObject(obj,"mac",cJSON_CreateString(pstStaticLeaseGet[index].mac));
		cJSON_AddItemToObject(obj,"ipaddress",cJSON_CreateString(pstStaticLeaseGet[index].ipaddress));
        cJSON_AddItemToArray(array,obj);
		 
	}
	
    if(pstStaticLeaseGet !=NULL)
	    free(pstStaticLeaseGet);
	
    cJSON_AddItemToObject(jsonOut, "code", cJSON_CreateNumber(0));	
	cJSON_AddItemToObject(jsonOut, "msg", cJSON_CreateString("success"));
}
 