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
#include <stdio.h>
#include <string.h>
#include "sec_api.h"

void HexStrToByte(const char* source, unsigned char* dest, int sourceLen)
{
    short i;
    unsigned char highByte, lowByte;
    
    for (i = 0; i < sourceLen; i += 2)
    {
        highByte = toupper(source[i]);
        lowByte  = toupper(source[i + 1]);


        if (highByte > 0x39)
            highByte -= 0x37;
        else
            highByte -= 0x30;


        if (lowByte > 0x39)
            lowByte -= 0x37;
        else
            lowByte -= 0x30;


        dest[i / 2] = (highByte << 4) | lowByte;
    }
    return ;
}


int main(int argc, char *argv[])
{
	char randBuf[16] = {0};
    char saveBuf[16] = {0};
    int i;

    switch (atoi(argv[2]))
    {
    case 0:
        GetRandom(randBuf, 16);
	    printf("rand number:\n");
        for (i = 0; i < 16; i ++)
            printf("0x%.2x ", randBuf[i]);
        printf("\n");
        break;

    case 1:
        HexStrToByte(argv[1], saveBuf, strlen(argv[1]));
        SaveData(saveBuf, strlen(argv[1])/2);
        break;

    case 2:
        LoadData(randBuf, 16);
        printf("load data:\n");
        for (i = 0; i < 16; i ++)
            printf("0x%.2x ", randBuf[i]);
        printf("\n");
        break;

    default:
        printf("unsupported argumen 2\n");
        break;
    }
	return 0;
}
