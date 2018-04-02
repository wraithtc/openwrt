/*
 * SPI testing utility (using spidev driver)
 *
 * Copyright (c) 2007  MontaVista Software, Inc.
 * Copyright (c) 2007  Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * Cross-compile with cross-gcc -I/path/to/cross-kernel/include
 */

#include "spi.h"


int GetRandom(char *randBuf, int len)
{
    unsigned char xor = 0x0;
    unsigned char temp = 0x0;
    unsigned char rcvBuf[32] = {0};
    unsigned char byte[32] = {0};
    int ret = 0;
	int fd;
    int i, count = 0;

    fd = open(SPI_DEV_NAME, O_RDWR);
	if (fd < 0)
		pabort("can't open device");

    ret = SendCmd(fd, SPI_CMD_RANDOM_REQUEST, NULL, (int)strlen(SPI_CMD_RANDOM_REQUEST), 0);    
    if (ret != 0)
        return ret;

    if (!IsChipIdle(fd))
        return -1;
    printf("===request result===\n");
    SpiSendByte(fd,0xA1);
    
    SpiRecvByte(fd, &temp);

    if (temp > 3)
        return -2;
    
    SpiRecv(fd, rcvBuf, temp);

    ret = SendCmd(fd, SPI_CMD_GET_RANDOM_NUM, NULL, (int)strlen(SPI_CMD_GET_RANDOM_NUM), 0);  
    if (ret != 0)
        return ret;

    if (!IsChipIdle(fd))
        return -1;
        
    SpiSendByte(fd,0xA1);
    
    SpiRecvByte(fd, &temp);
    if (temp != 0x13)
        return -2;
    printf("===random data===\n");
    SpiRecv(fd, rcvBuf, temp);
    memcpy(randBuf, rcvBuf, len);
    close(fd);

    return 0;
        
}

int SaveData(const char *data, int len)
{
    unsigned char xor = 0x0;
    unsigned char temp = 0x0;
    unsigned char rcvBuf[32] = {0};
    int ret = 0;
	int fd;
    int i, count = 0, dataLen;

    fd = open(SPI_DEV_NAME, O_RDWR);
	if (fd < 0)
		pabort("can't open device");
    
    ret = SendCmd(fd, SPI_CMD_FLUSH_BUF, NULL, (int)strlen(SPI_CMD_FLUSH_BUF), 0);
    if (ret != 0)
        return ret;
    
    ret = SendCmd(fd, SPI_CMD_DATA_IMPORT, data, (int)strlen(SPI_CMD_DATA_IMPORT), len);
    if (ret != 0)
        return ret;
    
    if (!IsChipIdle(fd))
        return -1;
    printf("===request result===\n");
    SpiSendByte(fd,0xA1);
    
    SpiRecvByte(fd, &temp);
    
    SpiRecv(fd, rcvBuf, temp);

    ret = SendCmd(fd, SPI_CMD_DATA_SAVE, NULL, (int)strlen(SPI_CMD_DATA_SAVE), 0);
    if (ret != 0)
        return ret;

    if (IsChipIdle(fd))
        return -1;
    
    SpiSendByte(fd,0xA1);
    
    SpiRecvByte(fd, &temp);
    if (temp > 3)
        return -2;
    printf("===save data result===\n");
    
    SpiRecv(fd, rcvBuf, temp);
    
    close(fd);

    return 0;
        
}


int LoadData(char *data, int len)
{
    unsigned char xor = 0x0;
    unsigned char temp = 0x0;
    unsigned char rcvBuf[64] = {0};
    unsigned char byte[32] = {0};
    int ret = 0;
	int fd;
    int i, count = 0;

    fd = open(SPI_DEV_NAME, O_RDWR);
	if (fd < 0)
		pabort("can't open device");

    ret = SendCmd(fd, SPI_CMD_FLUSH_BUF, NULL, (int)strlen(SPI_CMD_FLUSH_BUF), 0);
    if (ret != 0)
        return ret;

    ret = SendCmd(fd, SPI_CMD_DATA_LOAD, NULL, (int)strlen(SPI_CMD_DATA_LOAD), 0);
    if (ret != 0)
        return ret;

    if (!IsChipIdle(fd))
        return -1;
    printf("===request result===\n");
    SpiSendByte(fd,0xA1);
    
    SpiRecvByte(fd, &temp);
    
    SpiRecv(fd, rcvBuf, temp);
    
    ret = SendCmd(fd, SPI_CMD_DATA_EXPORT, NULL, (int)strlen(SPI_CMD_DATA_EXPORT), 0);
    if (ret != 0)
        return ret;

    if (!IsChipIdle(fd))
        return -1;
    SpiSendByte(fd,0xA1);
    
    SpiRecvByte(fd, &temp);
    if (temp > sizeof(rcvBuf))
        return -2;
    printf("===export data===\n");
    SpiRecv(fd, rcvBuf, temp);
    
    memcpy(data, rcvBuf, len);
    close(fd);

    return 0;
        
}


int SaveSmbPwd(const char *data, int len)
{
    unsigned char xor = 0x0;
    unsigned char temp = 0x0;
    unsigned char rcvBuf[32] = {0};
    int ret = 0;
	int fd;
    int i, count = 0, dataLen;

    fd = open(SPI_DEV_NAME, O_RDWR);
	if (fd < 0)
		pabort("can't open device");
    
    ret = SendCmd(fd, SPI_CMD_FLUSH_BUF, NULL, (int)strlen(SPI_CMD_FLUSH_BUF), 0);
    if (ret != 0)
        return ret;
    
    ret = SendCmd(fd, SPI_CMD_DATA_IMPORT, data, (int)strlen(SPI_CMD_DATA_IMPORT), len);
    if (ret != 0)
        return ret;
    
    if (!IsChipIdle(fd))
        return -1;
    printf("===request result===\n");
    SpiSendByte(fd,0xA1);
    
    SpiRecvByte(fd, &temp);
    
    SpiRecv(fd, rcvBuf, temp);

    ret = SendCmd(fd, SPI_CMD_SMB_PWD_SAVE, NULL, (int)strlen(SPI_CMD_SMB_PWD_SAVE), 0);
    if (ret != 0)
        return ret;

    if (IsChipIdle(fd))
        return -1;
    
    SpiSendByte(fd,0xA1);
    
    SpiRecvByte(fd, &temp);
    if (temp > 3)
        return -2;
    printf("===save data result===\n");
    
    SpiRecv(fd, rcvBuf, temp);
    
    close(fd);

    return 0;
        
}


int LoadSmbPwd(char *data, int len)
{
    unsigned char xor = 0x0;
    unsigned char temp = 0x0;
    unsigned char rcvBuf[64] = {0};
    unsigned char byte[32] = {0};
    int ret = 0;
	int fd;
    int i, count = 0;

    fd = open(SPI_DEV_NAME, O_RDWR);
	if (fd < 0)
		pabort("can't open device");

    ret = SendCmd(fd, SPI_CMD_FLUSH_BUF, NULL, (int)strlen(SPI_CMD_FLUSH_BUF), 0);
    if (ret != 0)
        return ret;

    ret = SendCmd(fd, SPI_CMD_SMB_PWD_LOAD, NULL, (int)strlen(SPI_CMD_SMB_PWD_LOAD), 0);
    if (ret != 0)
        return ret;

    if (!IsChipIdle(fd))
        return -1;
    printf("===request result===\n");
    SpiSendByte(fd,0xA1);
    
    SpiRecvByte(fd, &temp);
    
    SpiRecv(fd, rcvBuf, temp);
    
    ret = SendCmd(fd, SPI_CMD_DATA_EXPORT, NULL, (int)strlen(SPI_CMD_DATA_EXPORT), 0);
    if (ret != 0)
        return ret;

    if (!IsChipIdle(fd))
        return -1;
    SpiSendByte(fd,0xA1);
    
    SpiRecvByte(fd, &temp);
    if (temp > sizeof(rcvBuf))
        return -2;
    printf("===export data===\n");
    SpiRecv(fd, rcvBuf, temp);
    
    memcpy(data, rcvBuf, len);
    close(fd);

    return 0;
        
}


