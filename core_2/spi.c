#include "spi.h"

//SPI��ʼ��
void spi_init(void)
{
	//spi��ʼ��
	SPCR = 0x54;
	SPSR = 0x01;
}


