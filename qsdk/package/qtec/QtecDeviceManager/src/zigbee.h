
typedef struct zb_devinfo
{
	char devid[6];
	char ieee_addr[8];
	int nw_addr;
	char name[16];
	char version[8];
	char model[8];
	char seq[8];
	int  type;	
	int status;	
}zb_devinfo_t;

struct fpinfo
{
	char devid[10];
	char usrid[40];
	char fpid[10];
	char fpname[40];
};

#if 0
int initGdata();
int ZBQKeyPassReq(char *devid, int keynum, char* idlist, char* keylist);
int ZBQKeyPassRes();
int ZBDevStatusReq(char *session, char *devid);
int ZBDevUnlockReq(char *session, char *devid, char *usrid);
int ZBDevSearchReq(char *session);
int ZBDevAddReq(char *session, char *devid);
int ZBDevDelReq(char *session, char *devid);
int ZBAddFPReq(char *session, char *usrid, char *devid);
int ZBDelFPReq(char *session, char *devid, char *usrid, char *fpid);
int ZBgetdevSearch(char *session);
int ZBgetFP(char *session, char* devid ,char* usrid);
int updatelog(char * time, int opratetype, char* usrid, int code, char *devid);
int updatedevlist(zb_devinfo_t *devlist);
int dodevres(char *session, char *data, char *msg, int code);
int ProcgetdevSearch(char *session, zb_devinfo_t *devlist);
int readUART();
int openUART();
int closeUART();
#endif 
