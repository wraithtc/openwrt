#include <stdio.h>
#include <string.h>

void AuthMessageSet(char *key)
{
	FILE *fp;
	char buffer[256] = {0};
	
	if(NULL == key)
	{
		return;
	}


	fp = fopen("/html/www/lighttpd.user", "w+");
	if(fp == NULL)
	{
		printf("open file failed");
		return;
	}
	
	snprintf(buffer, 256, "admin:%s", key);

	fprintf(fp, "%s", buffer);

	return;
}
