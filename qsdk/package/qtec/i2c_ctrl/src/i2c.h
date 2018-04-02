#ifndef __I2C_H__
#define __I2C_H__
#include <linux/i2c-dev.h>


#define LED_ON 1
#define LED_OFF 0

#define LED_RED 1
#define LED_YELLOW 2
#define LED_GREEN 3
 


#define HT16K33_BLINK_CMD 0x80
#define HT16K33_BLINK_DISPLAYON 0x01
#define HT16K33_BLINK_OFF 0
#define HT16K33_BLINK_2HZ  1
#define HT16K33_BLINK_1HZ  2
#define HT16K33_BLINK_HALFHZ  3

#define HT16K33_CMD_BRIGHTNESS 0xE0

#define SEVENSEG_DIGITS 5

typedef struct{
    int x;
    int y;
}POINT_T;

typedef struct{
    int shape[5][5]; 
}NUMBER_SHAPE_T;

#endif /* __I2C_H__ */
