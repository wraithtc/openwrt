#include <stdio.h>
#include <string.h>
#include "devinfo.h"

static struct devInfo devlist[10] = {
					{"Jiuzhou Lock", "v1.0", "abc123", 1, 0},
					{"Jiuzhou Lock", "v1.0", "abc123", 2, 0},
					{"Jiuzhou Lock", "v1.0", "abc123", 3, 0},
					{"Jiuzhou Lock", "v1.0", "abc123", 4, 0},
					{"Jiuzhou Lock", "v1.0", "abc123", 5, 0},
					{"Jiuzhou Lock", "v1.0", "abc123", 6, 0},
					{"Jiuzhou Lock", "v1.0", "abc123", 7, 0},
					{"Jiuzhou Lock", "v1.0", "abc123", 8, 0},
					{"Jiuzhou Lock", "v1.0", "abc123", 9, 0},
					{"Jiuzhou Lock", "v1.0", "abc123", 10, 0}
				    };

static char * dddformat = "%d:%d-%d-%d-%d-%d-%d-%d-%d-%d-%d\r\n";

int searchDevList(struct devInfo *output)
{
	memcpy(output, devlist, sizeof(struct devInfo)*10);
	return 10;
}

int addBandDev(int devid)
{
	int bandlist[10] = {0,0,0,0,0,0,0,0,0,0};
	int bandnumber = 0;
	if (1>devid || devid > 10)
	return -1;
	mkdir("/etc/devlist", 0777);
	FILE *fd = fopen("/etc/devlist/devlistinfo", "r+");
	if (NULL!= fd)
	{
		fscanf(fd, dddformat, &bandnumber,&bandlist[0],&bandlist[1],&bandlist[2],&bandlist[3],&bandlist[4],&bandlist[5],&bandlist[6],&bandlist[7],&bandlist[8],&bandlist[9]);
		fclose(fd);
	}

	fd =  fopen("/etc/devlist/devlistinfo", "wb+");
	if (bandlist[devid-1]==0){bandlist[devid-1]= 1;bandnumber++;}

	fprintf(fd, dddformat, bandnumber, bandlist[0],bandlist[1],bandlist[2],bandlist[3],bandlist[4],bandlist[5],bandlist[6],bandlist[7],bandlist[8],bandlist[9]);		
	fclose(fd);
	return 0;
}

int delBandDev(int devid)
{
	int bandlist[10] = {0,0,0,0,0,0,0,0,0,0};
	int bandnumber = 0;
	if (1>devid || devid > 10)
	return -1;
	mkdir("/etc/devlist", 0777);

	FILE *fd = fopen("/etc/devlist/devlistinfo", "r+");
	if (NULL!= fd)
	{
		fscanf(fd, dddformat, &bandnumber,&bandlist[0],&bandlist[1],&bandlist[2],&bandlist[3],&bandlist[4],&bandlist[5],&bandlist[6],&bandlist[7],&bandlist[8],&bandlist[9]);			
		fclose(fd);
	}

	if(bandlist[devid-1] == 1){bandnumber--;bandlist[devid-1] = 0;}
	fd = fopen("/etc/devlist/devlistinfo", "wb+");
	fprintf(fd, dddformat, bandnumber, bandlist[0],bandlist[1],bandlist[2],bandlist[3],bandlist[4],bandlist[5],bandlist[6],bandlist[7],bandlist[8],bandlist[9]);
	fclose(fd);
	
	return 0;	
}

int getBandDev(struct devInfo *output)
{
	int bandlist[10] = {0,0,0,0,0,0,0,0,0,0};
	int bandnumber = 0;
	int i,j= 0;
	mkdir("/etc/devlist", 0777);
	FILE *fd = fopen("/etc/devlist/devlistinfo", "r+");
	if (NULL!= fd)
	{
		fscanf(fd, dddformat, &bandnumber,&bandlist[0],&bandlist[1],&bandlist[2],&bandlist[3],&bandlist[4],&bandlist[5],&bandlist[6],&bandlist[7],&bandlist[8],&bandlist[9]);
		fclose(fd);		
	}

	for (i = 0; i<10; i++)
	{
		if (bandlist[i] == 1)
		{
			memcpy(&output[j++], &devlist[i], sizeof(struct devInfo));
		}
	}

	return bandnumber;
}

