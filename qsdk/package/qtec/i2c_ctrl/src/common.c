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

#include "i2c.h"
#include <stdio.h>
#include <string.h>

static unsigned short displaybuffer[8] = {0};

int zero[5][5] = {
    {1,1,1,1,1},
    {1,0,0,1,1},
    {1,0,1,0,1},
    {1,1,0,0,1},
    {1,1,1,1,1},
};

int one[5][5] = {
    {0,0,1,0,0},
    {0,1,1,0,0},
    {0,0,1,0,0},
    {0,0,1,0,0},
    {0,1,1,1,0},
};

int two[5][5] = {
    {1,1,1,1,1},
    {0,0,0,0,1},
    {1,1,1,1,1},
    {1,0,0,0,0},
    {1,1,1,1,1},
};

int three[5][5] = {
    {1,1,1,1,1},
    {0,0,0,0,1},
    {1,1,1,1,1},
    {0,0,0,0,1},
    {1,1,1,1,1},
};

int four[5][5] = {
    {1,0,0,0,1},
    {1,0,0,0,1},
    {1,1,1,1,1},
    {0,0,0,0,1},
    {0,0,0,0,1},
};
    

int five[5][5] = {
    {1,1,1,1,0},
    {1,0,0,0,0},
    {1,1,1,1,1},
    {0,0,0,0,1},
    {1,1,1,1,1},
};


int six[5][5] = {
    {1,1,1,1,0},
    {1,0,0,0,0},
    {1,1,1,1,1},
    {1,0,0,0,1},
    {1,1,1,1,1},
};

int seven[5][5] = {
    {1,1,1,1,1},
    {0,0,0,0,1},
    {0,0,0,1,0},
    {0,0,1,0,0},
    {0,0,1,0,0},
};

int eight[5][5] = {
    {1,1,1,1,1},
    {1,0,0,0,1},
    {1,1,1,1,1},
    {1,0,0,0,1},
    {1,1,1,1,1},
};

int nine[5][5] = {
    {1,1,1,1,1},
    {1,0,0,0,1},
    {1,1,1,1,1},
    {0,0,0,0,1},
    {1,1,1,1,1},
};

int empty[5][5] = {
    {0,0,0,0,0},
    {0,0,0,0,0},
    {0,0,0,0,0},
    {0,0,0,0,0},
    {0,0,0,0,0},
};

int kilo[5][5] = {
    {1,0,0,0,1},
    {1,0,0,1,0},
    {1,1,1,0,0},
    {1,0,0,1,0},
    {1,0,0,0,1},
};
    
int milion[5][5] = {
    {1,0,0,0,1},
    {1,1,0,1,1},
    {1,0,1,0,1},
    {1,0,0,0,1},
    {1,0,0,0,1},
};


POINT_T start_pos[6] = {{0, 1}, {6, 1}, {12, 1}, {0, 7}, {6, 7}, {12, 7}};
NUMBER_SHAPE_T numberShape[13] = {0};
int display1[16] = {0};
int display2[16] = {0};
int left1 = 0;
int left2 = 0;

void setBlink(int devid, int blinkType)
{
    char cmd[128] = {0};

    char value = HT16K33_BLINK_CMD | HT16K33_BLINK_DISPLAYON | (blinkType << 1);

    snprintf(cmd, sizeof(cmd), "i2cset -y 0 0x%.2x 0x%.2x",devid, value);
    system(cmd);
}

void setBrightness(int devid, int brightness)
{
    char cmd[128] = {0};

    char value = HT16K33_CMD_BRIGHTNESS | brightness;

    snprintf(cmd, sizeof(cmd), "i2cset -y 0 0x%.2x 0x%.2x",devid, value);
    system(cmd);
}


void begin(int devid)
{
    char cmd[128] = {0};

    snprintf(cmd, sizeof(cmd), "i2cset -y 0 0x%.2x 0x21",devid);
    system(cmd);
    setBlink(devid, 0);
    setBrightness(devid, 15);
}

void drawPixel(int devid, int x, int y)
{
#if 0
  x += 7;
  x %= 8;


  if (color) {
    displaybuffer[y] |= 1 << x;
  } else {
    displaybuffer[y] &= ~(1 << x);
  }
#endif
    char cmd[128] = {0};

    snprintf(cmd, sizeof(cmd), "i2cset -y 0 0x%.2x 0x%x 0x%x",devid, x, y);
    system(cmd);
}

void drawNumber(int number, int id)
{
    int i,j;

    for (i = 0; i < 5; i++)
    {
        for (j = 0; j < 5; j++)
        {
            if (numberShape[number].shape[i][j] == 1)
            {
                int total = (i+start_pos[id].y)*16 + j + start_pos[id].x;
                int *devdisplay = (i+start_pos[id].y) >= 8?display2:display1;
                int x = (i+start_pos[id].y)%8 * 2;
                int y = (j + start_pos[id].x);
                if ((j + start_pos[id].x) == 0)
                {
                    if (i+start_pos[id].y < 8)
                    {
                        left1 |= 1 << (i+start_pos[id].y);
                    }
                    else
                    {
                        left2 |= 1 << ((i+start_pos[id].y)%8);
                    }
                }
                devdisplay[x] |= 1<<y;
                
            }
        }
    }
}

void clear()
{
    int i;
    for (i = 0; i < 8; i++)
        displaybuffer[i] = 0;
}

void display()
{
    int i = 0;
    char cmd[128] = {0};

    for (i = 0; i < 16; i+=2)
    {
        printf("0x%2x\n", display1[i]);
        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "i2cset -y -r 0 0x70 0x%x 0x%x", i, display1[i]>>1& 0xff);
        printf("%s\n",cmd);
        system(cmd);
        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "i2cset -y -r 0 0x70 0x%x 0x%x", i+1, (display1[i]>>1)>>8);
        printf("%s\n",cmd);
        system(cmd);
    }

    for (i = 0; i < 10; i+=2)
    {
        printf("0x%2x\n", display2[i]);
        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "i2cset -y -r 0 0x71 0x%x 0x%x", i, display2[i]>>1& 0xff);
        printf("%s\n",cmd);
        system(cmd);
        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "i2cset -y -r 0 0x71 0x%x 0x%x", i+1, (display2[i]>>1)>>8);
        printf("%s\n",cmd);
        system(cmd);
    }

    memset(cmd, 0, sizeof(cmd));

    snprintf(cmd, sizeof(cmd), "i2cset -y -r 0 0x71 10 0x%x", left1);
    printf("%s\n",cmd);
    system(cmd);
    memset(cmd, 0, sizeof(cmd));

    snprintf(cmd, sizeof(cmd), "i2cset -y -r 0 0x71 11 0x%x", left2);
    printf("%s\n",cmd);
    system(cmd);
}

int main(int argc, char *argv[])
{
	int ret = 0;
	int fd;
    int i;
    char *input = argv[1];
    if (!input || strlen(input) < 6)
    {
        printf("invalid input!\n");
        return 1;
    }

    printf("--------------input:%s------------------\n", input);
    memcpy(numberShape[0].shape, zero, sizeof(numberShape[0].shape));
    memcpy(numberShape[1].shape, one, sizeof(numberShape[1].shape));
    memcpy(numberShape[2].shape, two, sizeof(numberShape[2].shape));
    memcpy(numberShape[3].shape, three, sizeof(numberShape[3].shape));
    memcpy(numberShape[4].shape, four, sizeof(numberShape[4].shape));
    memcpy(numberShape[5].shape, five, sizeof(numberShape[5].shape));
    memcpy(numberShape[6].shape, six, sizeof(numberShape[6].shape));
    memcpy(numberShape[7].shape, seven, sizeof(numberShape[7].shape));
    memcpy(numberShape[8].shape, eight, sizeof(numberShape[8].shape));
    memcpy(numberShape[9].shape, nine, sizeof(numberShape[9].shape));
    memcpy(numberShape[10].shape, empty, sizeof(numberShape[10].shape));
    memcpy(numberShape[11].shape, kilo, sizeof(numberShape[11].shape));
    memcpy(numberShape[12].shape, milion, sizeof(numberShape[12].shape));
    //begin(devid);
    //begin(0x70);
    //begin(0x71);
    clear();
    printf("-------------------%s-----%d--------\n", __FUNCTION__,__LINE__);
    if (strncmp(input, "ffffff", 6) == 0)
    {
        printf("clean\n");
    }
    else
    {
        for (i = 0; i < 6; i++)
        {
            if ((input[i] - 0x30) >=0 && (input[i] - 0x30) <= 9)
            {
                drawNumber(input[i]-0x30, i);
            }
            else if (input[i] == 'f')
            {
                drawNumber(10, i);
            }
            else if (input[i] == 'k')
            {
                drawNumber(11, i);
            }
            else if (input[i] == 'm')
            {
                drawNumber(12, i);
            }
        }
    }
    display();
	return ret;
}
