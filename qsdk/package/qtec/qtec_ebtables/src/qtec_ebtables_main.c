#include "qtec_ebtables_basic.h"

bool qtec_ebtables_pr_debug=false;

static int usage(void)
{
	fprintf(stderr, "qtec_ebtables [-q][-o] print\n");
	fprintf(stderr, "qtec_ebtables [-q][-o] {start|stop}\n");
	

	return 1;
}

#define qtec_ebtables_logfile "/tmp/.qtec_ebtables"

static void init_log()
{
	FILE *f1;
	if(access(qtec_ebtables_logfile,F_OK) !=0)
	{	
		return;
	}
	f1 = open(qtec_ebtables_logfile, O_RDWR | O_APPEND);

	if(f1!=NULL)
	{
		dup2(f1,1);
		dup2(f1,2);

		close(f1);
	}
	
}

static int expand_ebtables_speedlimit_rule(struct ebtables_speedlimit_rule *rule)
{
    DEBUG_PRINTF("[%s]============rule name:%s =========\n",__func__, rule->name);
    int limit=0;
    int limit_brust=0;
    char cmd[256]={0};
    if(rule->enabled == false)
    {
        DEBUG_PRINTF("[%s]=====this rule not enabled====\n",__func__);
        return 0;
    }


    //input 
    if(rule->dest == 0)
    {
        limit = (rule->limit)/1000 +1 ;
        limit_brust = (limit*3)/2+1;
        memset(cmd,0,256);
        snprintf(cmd,256,"ebtables -I Landownspeedlimit -d %s -j DROP",rule->mac);
        system(cmd);

        memset(cmd,0,256);
        snprintf(cmd,256,"ebtables -I Landownspeedlimit -d %s  --limit %d/s --limit-burst %d -j RETURN", rule->mac, limit,limit_brust);
        system(cmd); 
    }
    //output
    else if(rule->dest == 1)
    {
        limit = (rule->limit)/1000+1;
        limit_brust=(limit*3)/2+1;
        memset(cmd,0,256);
        snprintf(cmd,256,"ebtables -I Lanupspeedlimit -s %s -j DROP",rule->mac);
        system(cmd);

        memset(cmd,0,256);
        snprintf(cmd,256,"ebtables -I Lanupspeedlimit -s %s  --limit %d/s --limit-burst %d -j RETURN", rule->mac, limit,limit_brust);
        system(cmd);
        
    }
    else
    {
        memset(cmd,0,256);
        snprintf(cmd, sizeof(cmd), "ebtables -D GuestWifiRule -p ipv4 --ip-dst %s -i %s -j %s", rule->dest_ip, rule->src_if, rule->target);
        DEBUG_PRINTF("[%s]=====%s====\n",__func__, cmd);
        system(cmd);
        
        memset(cmd,0,256);
        snprintf(cmd, sizeof(cmd), "ebtables -I GuestWifiRule -p ipv4 --ip-dst %s -i %s -j %s", rule->dest_ip, rule->src_if, rule->target);
        DEBUG_PRINTF("[%s]=====%s====\n",__func__, cmd);
        system(cmd);
    }
}


static int start(void)
{
    DEBUG_PRINTF("[%s]=====qtec_ebtables start======\n",__func__);

    //current only manage lan speed limit 
  
    struct ebtables_speedlimit_rule *rule;
    list_for_each_entry(rule,&global_ebtables_speedlimit_rule,list)
    {
        expand_ebtables_speedlimit_rule(rule);
    }

    return 0;
    
}

static int stop(void)
{
    DEBUG_PRINTF("[%s]=====qtec_ebtables stop ======\n", __func__);

    //current only manage lan speed limit 
    system("ebtables -F Lanupspeedlimit");
    system("ebtables -F Landownspeedlimit");
    system("ebtables -F GuestWifiRule");
}



void main(int argc, char **argv)
{
    DEBUG_PRINTF("=====hello world, qtec_ebtables =====\n");

	int ch,rv=1;
	
	while ((ch = getopt(argc, argv, "qoh")) != -1)
		{
			switch (ch)
			{

	
			case 'q':
				if (freopen("/dev/null", "w", stdout)) {}
				break;

			case 'o':
				init_log();
				break;
	
			case 'h':
				rv = usage();
				goto out;
			}
		}

	if(optind >=argc)
	{
		usage();
        goto out;
	}

    fw_load_ebtables();

    if(!strcmp(argv[optind],"start"))
    {
        if(qtec_ebtables_lock())
        {
            start();
            qtec_ebtables_unlock();
        }
    }
    else if(!strcmp(argv[optind],"stop"))
    {
        if(qtec_ebtables_lock())
        {
            stop();
            qtec_ebtables_unlock();
        }
    }
    else if(!strcmp(argv[optind],"restart"))
    {
        if(qtec_ebtables_lock())
        {
            stop();
            start();
            qtec_ebtables_unlock();
        }
    }

	out:
		return;
}
