#ifndef __SPI_H__
#define __SPI_H__
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define SPI_CMD_INIT    "fb33445566bf"
#define SPI_CMD_SET_STATUS    "fb00440066"
#define SPI_CMD_RANDOM_REQUEST    "BF55000010"
#define SPI_CMD_GET_RANDOM_NUM    "BF52000010"
#define SPI_CMD_DATA_SAVE    "BF5A000100"
#define SPI_CMD_DATA_LOAD    "BF5B000100"
#define SPI_CMD_DATA_EXPORT    "BF53000010"
#define SPI_CMD_DATA_IMPORT    "BF51000010"
#define SPI_CMD_SMB_PWD_SAVE    "BF5A000000"
#define SPI_CMD_SMB_PWD_LOAD    "BF5B000000"
#define SPI_CMD_FLUSH_BUF    "BF54000000"

#define SPI_DEV_NAME    "/dev/spidev1.0"

#define MIN(a, b) ((a) > (b))?(b):(a)

#define SPI_SPEED   6000000
#define SPI_DELAY   300
#define SPI_BITS    8

void pabort(const char *s);

unsigned char getxor(unsigned char *pbuff,unsigned int len);

void HexStrToByte(const char* source, unsigned char* dest, int sourceLen);

void SpiSendByte(int fd, uint8_t value);

void SpiSend(int fd, uint8_t *buf, int len);

void SpiRecvByte(int fd, char *recvBuf);

void SpiRecv(int fd, char* recvBuf, int lenth);

int SendCmd(int fd, const char *cmd, const char *data, int cmdLen, int dataLen);

int IsChipIdle(int fd);

#endif

