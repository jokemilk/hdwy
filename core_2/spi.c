#include "spi.h"

//SPI初始化
void spi_init(void)
{
	//spi初始化
	SPCR = 0x54;
	SPSR = 0x01;
}


