#include "basic.h"

//Õâ¸öÎÄ¼þ´¦ÀíwanÏà¹ØµÄÊÂÒË
void proc_wan_set(cJSON *json_value, cJSON *jsonOut)
{
    DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__);
	struct wanStaticConfig wannode = {0};
	struct wanPppoeConfig  pppoeNode = {0};
	int ret=0;
    char *type=NULL;
    type = cJSON_GetObjectItem(json_value, "connectiontype")?cJSON_GetObjectItem(json_value, "connectiontype")->valuestring:"";

    if(strncmp(type,"dhcp",strlen(type))==0)
    {
		ret=wanDhcpConfigSet();
    }
    else if(strncmp(type,"pppoe",strlen(type))==0)
    {
		strcpy(pppoeNode.username, (cJSON_GetObjectItem(json_value, "username")?cJSON_GetObjectItem(json_value, "username")->valuestring:""));

#if 0
        //¶Ôpppoe ÃÜÂë½øÐÐbase64 ½âÂë²Ù×÷
        char tmp_base64_password[64]={0};
		//strcpy(pppoeNode.password, (cJSON_GetObjectItem(json_value, "password")?cJSON_GetObjectItem(json_value, "password")->valuestring:""));
	    strcpy(tmp_base64_password,(cJSON_GetObjectItem(json_value, "password")?cJSON_GetObjectItem(json_value, "password")->valuestring:""));
        DEBUG_PRINTF("[%s]====tmp_base64_password: %s====\n",__func__,tmp_base64_password);
        base64_decode(tmp_base64_password, pppoeNode.password);  
        DEBUG_PRINTF("[%s], ppppoeNode.password: %s===\n",__func__,pppoeNode.password);
#endif
        char *password=NULL;
        password =cJSON_GetObjectItem(json_value, "password")?cJSON_GetObjectItem(json_value, "password")->valuestring:"";

        DEBUG_PRINTF("===[%s]===str: %s ===\n",__func__,password);
        unsigned char bindata[2048]={0};
        int len_debug=base64_decode(password, bindata);
    
        char *result=NULL;
        result=(char *)my_decrypt(len_debug,bindata, RSA_PRIVATE_KEY_FILE); 
        DEBUG_PRINTF("[%s]====result %s===\n",__func__,result);
        if(result == NULL)
        {
            global_weberrorcode=ERR_DECRY_FAIL;
            return;
        }
        
        strcpy(pppoeNode.password,result);
		ret=wanPppoeConfigSet(&pppoeNode);
        if(result !=NULL)
        {
            free(result);
        }
    }
    else if(strncmp(type,"static",strlen(type))==0)
    {
	    strcpy(wannode.ipaddr, (cJSON_GetObjectItem(json_value, "ipaddr")?cJSON_GetObjectItem(json_value, "ipaddr")->valuestring:""));
		strcpy(wannode.netmask, (cJSON_GetObjectItem(json_value, "netmask")?cJSON_GetObjectItem(json_value, "netmask")->valuestring:""));
		strcpy(wannode.gateway, (cJSON_GetObjectItem(json_value, "gateway")?cJSON_GetObjectItem(json_value, "gateway")->valuestring:""));
		strcpy(wannode.dns, (cJSON_GetObjectItem(json_value, "dns")?cJSON_GetObjectItem(json_value, "dns")->valuestring:""));
		ret=wanStaticConfigSet(&wannode);
        ProcFirewallSetMsgReq();
    }
    else
    {
            global_weberrorcode=ERR_VALUE_WRONG;
	}


	return;	
}

int ProcWanSetByApp(cJSON *json_value, cJSON *jsonOut)
{
	struct wanStaticConfig wannode = {0};
	struct wanPppoeConfig  pppoeNode = {0};
	int ret=0;
    char *type=NULL;
    type = cJSON_GetObjectItem(json_value, "connectiontype")?cJSON_GetObjectItem(json_value, "connectiontype")->valuestring:"";

    if(strncmp(type,"dhcp",strlen(type))==0)
    {
		ret=wanDhcpConfigSet();
    }
    else if(strncmp(type,"pppoe",strlen(type))==0)
    {
		strcpy(pppoeNode.username, (cJSON_GetObjectItem(json_value, "username")?cJSON_GetObjectItem(json_value, "username")->valuestring:""));
		strcpy(pppoeNode.password, (cJSON_GetObjectItem(json_value, "password")?cJSON_GetObjectItem(json_value, "password")->valuestring:""));
		ret=wanPppoeConfigSet(&pppoeNode);
    }
    else if(strncmp(type,"static",strlen(type))==0)
    {
	    strcpy(wannode.ipaddr, (cJSON_GetObjectItem(json_value, "ipaddr")?cJSON_GetObjectItem(json_value, "ipaddr")->valuestring:""));
		strcpy(wannode.netmask, (cJSON_GetObjectItem(json_value, "netmask")?cJSON_GetObjectItem(json_value, "netmask")->valuestring:""));
		strcpy(wannode.gateway, (cJSON_GetObjectItem(json_value, "gateway")?cJSON_GetObjectItem(json_value, "gateway")->valuestring:""));
		strcpy(wannode.dns, (cJSON_GetObjectItem(json_value, "dns")?cJSON_GetObjectItem(json_value, "dns")->valuestring:""));
		ret=wanStaticConfigSet(&wannode);
    }

	return ret;	
}

void get_wan_type(cJSON *json_value,cJSON *jsonOut)
{
    DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__);
    int type=0;
    int ret=0;

    cJSON *obj = NULL;
	obj = cJSON_CreateObject();

    ret=getWanConnectionType(&type);

    if(ret != 0)
    {
        global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
        return;
    }

    cJSON_AddItemToObject(jsonOut, "data", obj);

    if(type == WANDHCP)
    {
        cJSON_AddItemToObject(obj, "connectiontype", cJSON_CreateString("dhcp"));
    }
    else if(type == WANPPPOE)
    {
        cJSON_AddItemToObject(obj, "connectiontype", cJSON_CreateString("pppoe"));
    }
    else if(type == WANSTATIC)
    {
        cJSON_AddItemToObject(obj, "connectiontype", cJSON_CreateString("static"));
    }
}

void proc_wan_get(cJSON *json_value,cJSON *jsonOut)
{
    DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__);
    int type=0;
    int ret=0;
    
    cJSON *obj = NULL;
	obj = cJSON_CreateObject();
    
    ret=getWanConnectionType(&type);
    DEBUG_PRINTF("===[%s]===ret:%d==== type:%d====\n",__func__,ret,type);

    if(ret != 0)
    {
        global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
        return;
    }

    if(type == WANDHCP)
    {
        
        cJSON_AddItemToObject(jsonOut, "data", obj);
        cJSON_AddItemToObject(obj, "connectiontype", cJSON_CreateString("dhcp"));

        //èŽ·å–pppoe çš„é…ç½®
        struct wanPppoeConfig  pppoeNode = {0};
        ret = wanPppoeConfigGet(&pppoeNode);
        
        if(ret==0)
        {
            //¶Ôpppoe password ½øÐÐ×ªÂë
            char base64_password[256]={0};
            if(strlen(pppoeNode.password) != 0)
            {
                base64_encode(pppoeNode.password, base64_password, strlen(pppoeNode.password));
            }
    
           	cJSON_AddItemToObject(obj, "username", cJSON_CreateString(pppoeNode.username));
	        cJSON_AddItemToObject(obj, "password", cJSON_CreateString(base64_password));

        }


        //èŽ·å–static çš„é…ç½®
        struct wanStaticConfig wannode = {0};
        ret = wanStaticConfigGet(&wannode);

        if(ret == 0)
        {
            cJSON_AddItemToObject(obj, "ipaddr", cJSON_CreateString(wannode.ipaddr));
	        cJSON_AddItemToObject(obj, "netmask", cJSON_CreateString(wannode.netmask));
            cJSON_AddItemToObject(obj, "gateway", cJSON_CreateString(wannode.gateway));
            cJSON_AddItemToObject(obj, "dns", cJSON_CreateString(wannode.dns));
        }
        
        return;
    }
    else if(type == WANPPPOE)
    {
         
        struct wanPppoeConfig  pppoeNode = {0};
        ret = wanPppoeConfigGet(&pppoeNode);
        
        if(ret !=0)
        {
            global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
            return;
        }
        else
        {
            //¶Ôpppoe password ½øÐÐ×ªÂë
            char base64_password[256]={0};
            if(strlen(pppoeNode.password) != 0)
            {
                base64_encode(pppoeNode.password, base64_password, strlen(pppoeNode.password));
            }
            cJSON_AddItemToObject(jsonOut, "data", obj);
            cJSON_AddItemToObject(obj, "connectiontype", cJSON_CreateString("pppoe"));
           	cJSON_AddItemToObject(obj, "username", cJSON_CreateString(pppoeNode.username));
	        cJSON_AddItemToObject(obj, "password", cJSON_CreateString(base64_password));

            //èŽ·å–static çš„é…ç½®
            struct wanStaticConfig wannode = {0};
            ret = wanStaticConfigGet(&wannode);

            if(ret == 0)
            {
                cJSON_AddItemToObject(obj, "ipaddr", cJSON_CreateString(wannode.ipaddr));
	            cJSON_AddItemToObject(obj, "netmask", cJSON_CreateString(wannode.netmask));
                cJSON_AddItemToObject(obj, "gateway", cJSON_CreateString(wannode.gateway));
                cJSON_AddItemToObject(obj, "dns", cJSON_CreateString(wannode.dns));
            }

        }


    }
    else if(type == WANSTATIC)
    {
       
        struct wanStaticConfig wannode = {0};
        ret = wanStaticConfigGet(&wannode);
        
        if(ret !=0)
        {
            global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
            return;
        }
        else
        {
            cJSON_AddItemToObject(jsonOut, "data", obj);
            cJSON_AddItemToObject(obj, "connectiontype", cJSON_CreateString("static"));
           	cJSON_AddItemToObject(obj, "ipaddr", cJSON_CreateString(wannode.ipaddr));
	        cJSON_AddItemToObject(obj, "netmask", cJSON_CreateString(wannode.netmask));
            cJSON_AddItemToObject(obj, "gateway", cJSON_CreateString(wannode.gateway));
            cJSON_AddItemToObject(obj, "dns", cJSON_CreateString(wannode.dns));

            //èŽ·å–pppoe çš„é…ç½®
            struct wanPppoeConfig  pppoeNode = {0};
            ret = wanPppoeConfigGet(&pppoeNode);
            
            if(ret==0)
            {
                //¶Ôpppoe password ½øÐÐ×ªÂë
                char base64_password[256]={0};
                if(strlen(pppoeNode.password) != 0)
                {
                    base64_encode(pppoeNode.password, base64_password, strlen(pppoeNode.password));
                }
      
                cJSON_AddItemToObject(obj, "username", cJSON_CreateString(pppoeNode.username));
                cJSON_AddItemToObject(obj, "password", cJSON_CreateString(base64_password));
            
            }


            return;
        }
    }
    else
    {
        global_weberrorcode=ERR_VALUE_WRONG;
        return;
    }
    return;
    
}

int ProcWanGetByApp(cJSON *json_value,cJSON *jsonOut)
{
    DEBUG_PRINTF("====[%s]====%d===\n",__func__,__LINE__);
    int type=0;
    int ret=0;
    
    cJSON *obj = NULL;
	obj = cJSON_CreateObject();
    
    ret=getWanConnectionType(&type);
    DEBUG_PRINTF("===[%s]===ret:%d==== type:%d====\n",__func__,ret,type);

    if(ret != 0)
    {
        return ERR_INTERNALLOGIC_WRONG;
    }

    if(type == WANDHCP)
    {   
        cJSON_AddItemToObject(jsonOut, "data", obj);
        cJSON_AddItemToObject(obj, "connectiontype", cJSON_CreateString("dhcp"));

        //èŽ·å–pppoe çš„é…ç½®
        struct wanPppoeConfig  pppoeNode = {0};
        ret = wanPppoeConfigGet(&pppoeNode);

        if(ret == 0)
        {
            cJSON_AddItemToObject(obj, "username", cJSON_CreateString(pppoeNode.username));
	        cJSON_AddItemToObject(obj, "password", cJSON_CreateString(pppoeNode.password));
        }

        //èŽ·å–static çš„é…ç½®
        struct wanStaticConfig wannode = {0};
        ret = wanStaticConfigGet(&wannode);

        if(ret == 0)
        {
            cJSON_AddItemToObject(obj, "ipaddr", cJSON_CreateString(wannode.ipaddr));
	        cJSON_AddItemToObject(obj, "netmask", cJSON_CreateString(wannode.netmask));
            cJSON_AddItemToObject(obj, "gateway", cJSON_CreateString(wannode.gateway));
            cJSON_AddItemToObject(obj, "dns", cJSON_CreateString(wannode.dns));
        }
    }
    else if(type == WANPPPOE)
    {
         
        struct wanPppoeConfig  pppoeNode = {0};
        ret = wanPppoeConfigGet(&pppoeNode);
        
        if(ret !=0)
        {
            return ERR_INTERNALLOGIC_WRONG;
        }
        else
        {
            cJSON_AddItemToObject(jsonOut, "data", obj);
            cJSON_AddItemToObject(obj, "connectiontype", cJSON_CreateString("pppoe"));
           	cJSON_AddItemToObject(obj, "username", cJSON_CreateString(pppoeNode.username));
	        cJSON_AddItemToObject(obj, "password", cJSON_CreateString(pppoeNode.password));

            //èŽ·å–static çš„é…ç½®
            struct wanStaticConfig wannode = {0};
            ret = wanStaticConfigGet(&wannode);

            if(ret == 0)
            {
                cJSON_AddItemToObject(obj, "ipaddr", cJSON_CreateString(wannode.ipaddr));
	            cJSON_AddItemToObject(obj, "netmask", cJSON_CreateString(wannode.netmask));
                cJSON_AddItemToObject(obj, "gateway", cJSON_CreateString(wannode.gateway));
                cJSON_AddItemToObject(obj, "dns", cJSON_CreateString(wannode.dns));
            }

        }
        

    }
    else if(type == WANSTATIC)
    {
       
        struct wanStaticConfig wannode = {0};
        ret = wanStaticConfigGet(&wannode);
        
        if(ret !=0)
        {
            return ERR_INTERNALLOGIC_WRONG;
        }
        else
        {
            cJSON_AddItemToObject(jsonOut, "data", obj);
            cJSON_AddItemToObject(obj, "connectiontype", cJSON_CreateString("static"));
           	cJSON_AddItemToObject(obj, "ipaddr", cJSON_CreateString(wannode.ipaddr));
	        cJSON_AddItemToObject(obj, "netmask", cJSON_CreateString(wannode.netmask));
            cJSON_AddItemToObject(obj, "gateway", cJSON_CreateString(wannode.gateway));
			cJSON_AddItemToObject(obj, "dns", cJSON_CreateString(wannode.dns));

            //èŽ·å–pppoe çš„é…ç½®
            struct wanPppoeConfig  pppoeNode = {0};
            ret = wanPppoeConfigGet(&pppoeNode);

            if(ret == 0)
            {
                cJSON_AddItemToObject(obj, "username", cJSON_CreateString(pppoeNode.username));
	            cJSON_AddItemToObject(obj, "password", cJSON_CreateString(pppoeNode.password));
            }
        }
    }
    else
    {
        return ERR_VALUE_WRONG;
    }
    return 0;
    
}
//Õï¶ÏwanÄÜ·ñÉÏÍø
//ubus call network.interface.wan status 

#define test_internet_ipaddress "8.8.8.8"
void proc_wan_detect(cJSON *json_value, cJSON *jsonOut)
{
    DEBUG_PRINTF("===[%s]====\n",__func__);
    int ret=0;
    FILE *file;
    char wan_up[10]={0}; //È·ÈÏwan¿ÚÍøÏßÊÇ·ñ²åÉÏ£¬»òÕßpppoeÊÇ·ñÁ¬½Ó³É¹¦
    char wan_ipaddr[64]={0}; //»ñÈ¡wan ¿ÚipµØÖ·
    char wan_gw[64]={0}; //»ñÈ¡wan¿Ú gw µØÖ·
    char wan_dns[64]={0}; // »ñÈ¡wan¿Ú dnsµØÖ·
    char line[256]={0};
    int type=0;
    char wan_ifname[256]={0};
    rtcfgUciGet("network.wan.ifname",wan_ifname);
    ret=getWanConnectionType(&type);
    
    if(type == WANSTATIC)
    {
        struct wanStaticConfig wannode = {0};
        ret = wanStaticConfigGet(&wannode);
        if(ret !=0)
        {
            DEBUG_PRINTF("[%s]==can't get wan static config===\n",__func__);
            global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
        }

        //check wan gw
        ret=checkPingResult(wannode.gateway,wan_ifname);
        if(ret !=0)
        {
            global_weberrorcode=ERR_WAN_GWWRONG;
            return;
        }

        //ÔÝÊ±²»²âdns
        ret=checkPingResult(test_internet_ipaddress, wan_ifname);
        if(ret !=0)
        {
            global_weberrorcode=ERR_WAN_INTERNETWRONG;
            return;
        }
        
        return;
    }
    else
    {
        file=popen("ubus call network.interface.wan status", "r");

        if( NULL != file )
        {
            while (fgets(line,sizeof(line),file) != NULL)
            {
               
                if(strstr(line,"\"up\"")!=NULL)
                {
                    DEBUG_PRINTF("line=%s \n",line);
                    
                    sscanf(line," \"up\": %s,",wan_up);
                }

                if(strstr(line, "\"address\":")!=NULL)
                {
                    DEBUG_PRINTF("line=%s \n",line);
                    sscanf(line," \"address\": %s",wan_ipaddr);
                }

                if(strstr(line, "\"nexthop\":")!=NULL)
                {
                    DEBUG_PRINTF(
"line=%s \n",line);
                    sscanf(line," \"nexthop\": %s",wan_gw);
                }
         
            }

            if(strncmp(wan_up,"true,",strlen(wan_up)) != 0)
            {
                global_weberrorcode = ERR_WAN_NOTUP;
                return;
            }

            if( strlen(wan_ipaddr) ==0 )
            {
                global_weberrorcode = ERR_WAN_IPWRONG;
                return;
            }

            if( strlen(wan_gw) == 0)
            {
                global_weberrorcode = ERR_WAN_GWWRONG;
                return;
            }
            else
            {
                wan_gw[strlen(wan_gw)-1]='\0';
                DEBUG_PRINTF("[%s]====wan_gw: %s=====\n",__func__,wan_gw);
                ret=checkPingResult(wan_gw, wan_ifname);
                if(ret !=0)
                {
                    global_weberrorcode= ERR_WAN_GWWRONG;
                    return;
                }

                ret =checkPingResult(test_internet_ipaddress, wan_ifname);
                if(ret !=0 )
                {
                    global_weberrorcode=ERR_WAN_INTERNETWRONG;
                    return; 
                }
            }
        }
        else
        {
            DEBUG_PRINTF("[%s]===popen fail ===\n",__func__);
            global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
            return;
        }
        pclose(file);
    }

   
}

void HandleWanCfg(cJSON *jsonValue, cJSON *jsonOut)
{
    DEBUG_PRINTF("[%s]====\n",__func__);

    if( (request_method & CGI_GET_METHOD) != 0)
    {
        proc_wan_get(jsonValue, jsonOut);
    }
    else if ( (request_method & CGI_PUT_METHOD ) != 0 )
    {
        proc_wan_set(jsonValue, jsonOut);
    }
    else
    {
        global_weberrorcode=ERR_METHOD_NOT_SUPPORT;
    }
}

void ProcWanDetialGet(cJSON *jsonValue, cJSON *jsonOut)
{
    int ret = 0;
    int wwanflag = 0;
    char status[8] = {0};
	char buffer[64] = {0};
    char proto[64] = {0};
    char wanip[64] = {0};
    char gateway[64] = {0};
    char dns1[64] = {0};
    char dns2[64] = {0};
    FILE *fp;
    cJSON *obj = NULL;
    
	obj = cJSON_CreateObject();
    
    ret=rtcfgUciGet("network.wan.proto", proto);
    if(ret != 0)
    {
        global_weberrorcode=ERR_INTERNALLOGIC_WRONG;
        return;
    }

    system("ubus call network.interface.wwan status | grep '\"up\"' | sed -e 's/^.*: \\(.*\\),/\\1/g' > /tmp/wdsup");
    fp = fopen("/tmp/wdsup","r");
    if (fp != NULL)
    {
        fgets(status, sizeof(status), fp);
        if (!strncmp(status, "true", 4))
        {
            wwanflag = 1;
        }
        fclose(fp);
        fp = NULL;
    }

    if(wwanflag)
    {
        system("ubus call network.interface.wwan status | grep \"address\" | grep -oE '[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}' > /tmp/wanip.txt");
        system("ubus call network.interface.wwan status | grep nexthop | grep -oE '[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}' > /tmp/wangateway.txt");
        system("ubus call network.interface.wwan status | grep -A 3 dns-server | grep -oE '[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}' > /tmp/wandns.txt");
    }
    else
    {
        system("ubus call network.interface.wan status | grep \"address\" | grep -oE '[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}' > /tmp/wanip.txt");
        system("ubus call network.interface.wan status | grep nexthop | grep -oE '[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}' > /tmp/wangateway.txt");
        system("ubus call network.interface.wan status | grep -A 3 dns-server | grep -oE '[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}' > /tmp/wandns.txt");
    }
    
    fp = fopen("/tmp/wanip.txt", "r");
    if(fp)
    {
		memset(buffer, 0, 64);
	    fgets(buffer,64,fp);
		sscanf(buffer, "%s\n", wanip);
	    fclose(fp);
        fp = 0;
    }

    fp = fopen("/tmp/wangateway.txt", "r");
    if(fp)
    {
		memset(buffer, 0, 64);
	    fgets(buffer,64,fp);
        if(0 == strcmp(buffer, "0.0.0.0\n"))
        {
            memset(buffer, 0, 64);
    	    fgets(buffer,64,fp);
    		sscanf(buffer, "%s\n", gateway);
        }
        else
        {
            sscanf(buffer, "%s\n", gateway);
        }
        
        fclose(fp);
        fp = 0;
    }

    fp = fopen("/tmp/wandns.txt", "r");
    if(fp)
    {
		memset(buffer, 0, 64);
	    fgets(buffer,64,fp);
		sscanf(buffer, "%s\n", dns1);
		memset(buffer, 0, 64);
	    fgets(buffer,64,fp);
		sscanf(buffer, "%s\n", dns2);
	    fclose(fp);
        fp = 0;
    }

    cJSON_AddItemToObject(jsonOut, "data", obj);
    cJSON_AddItemToObject(obj, "connectiontype", cJSON_CreateString(proto));
   	cJSON_AddItemToObject(obj, "ipaddr", cJSON_CreateString(wanip));
    cJSON_AddItemToObject(obj, "gateway", cJSON_CreateString(gateway));
    cJSON_AddItemToObject(obj, "dns1", cJSON_CreateString(dns1));
    cJSON_AddItemToObject(obj, "dns2", cJSON_CreateString(dns2));

    return;
}

