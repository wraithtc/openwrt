#include "spi.h"


void pabort(const char *s)
{
	perror(s);
	abort();
}


unsigned char getxor(unsigned char *pbuff,unsigned int len)
{
	unsigned char xor = 0x0;

	while(len--)
		xor ^= *pbuff++;

	return xor;
}

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


void SpiSendByte(int fd, uint8_t value)
{
	int ret;
	uint8_t tx[] = {
		0xFB, 0x33, 0x44, 0x55, 0x66, 0xBF/*0xFF, 0xFF,
		0x40, 0x00, 0x00, 0x00, 0x00, 0x95,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xDE, 0xAD, 0xBE, 0xEF, 0xBA, 0xAD,
		0xF0, 0x0D,*/
	};
	uint8_t rx[1] = {0, };
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)&value,
		.rx_buf = (unsigned long)rx,
		.len = 1,
		.delay_usecs = SPI_DELAY,
		.speed_hz = SPI_SPEED,
		.bits_per_word = SPI_BITS,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		pabort("can't send spi message");

	for (ret = 0; ret < 1; ret++) {
		if (!(ret % 6))
			puts("");
		printf("%.2X ", rx[ret]);
	}
	puts("");
    usleep(200);
}

void SpiSend(int fd, uint8_t *buf, int len)
{
    int i = 0;

    for (i = 0; i < len; i++)
    {
        SpiSendByte(fd, buf[i]);
    }
}

void SpiRecvByte(int fd, char *recvBuf)
{
	int ret;
	uint8_t tx[] = {
		0x01, /*, 0xFF,
		0x40, 0x00, 0x00, 0x00, 0x00, 0x95,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xDE, 0xAD, 0xBE, 0xEF, 0xBA, 0xAD,
		0xF0, 0x0D,*/
	};
	uint8_t rx[ARRAY_SIZE(tx)] = {0,};
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)recvBuf,
		.len = ARRAY_SIZE(tx),
		.delay_usecs = SPI_DELAY,
		.speed_hz = SPI_SPEED,
		.bits_per_word = SPI_BITS,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		pabort("can't send spi message");

	for (ret = 0; ret < ARRAY_SIZE(tx); ret++) {
		if (!(ret % 6))
			puts("");
		printf("%.2X ", *recvBuf);
	}
	puts("");
    usleep(200);
}

void SpiRecv(int fd, char* recvBuf, int lenth)
{
	int i = 0;

    for (i = 0; i < lenth; i++)
    {
        SpiRecvByte(fd, &recvBuf[i]);
    }
}

int IsChipIdle(int fd)
{
    char temp = 0x0;
    int count = 0;
    
    while (temp != 0x01 && count < 5)
    {
		SpiSendByte(fd, 0xE5);
		SpiRecvByte(fd, &temp);
        usleep(200);
        count ++;
    }

    return (temp == 0x01)?1:0;
}


int SendCmd(int fd, const char *cmd, const char *data, int cmdLen, int dataLen)
{
    char byte[64] = {0};
    char input[32] = {0};
    unsigned char xor = 0x0;
    int byteLen = cmdLen/2 + dataLen;
    int ret;

    printf("cmd:[%s]", cmd);

    if (!IsChipIdle(fd))
        return -1;
    
    HexStrToByte(cmd, byte, (int)cmdLen);
    if (dataLen != 0)
    {
        memcpy(byte + cmdLen/2, data, MIN((int)sizeof(byte) - cmdLen/2, dataLen));
    }
    xor = byteLen + 1;
    xor ^= getxor(byte, (unsigned int)byteLen);
    SpiSendByte(fd, 0xC1);
    
    SpiSendByte(fd, byteLen + 1);
    
    printf("===send command===\n");
    for (ret = 0; ret < byteLen; ret++)
        printf("0x%x ", byte[ret]);
    printf("\n");
	SpiSend(fd, byte, (int)byteLen);
    
    SpiSendByte(fd,xor);

    return 0;
}



