#include <avr/io.h>
#include <avr/interrupt.h>
#include "usart.h"
#include "marco.h"


void usart_init(void)
{
	UCSR1B = 0x00;//��ֹ�ж�
	UCSR1A = 0x00;
	UCSR1C = 0x26;
	UBRR1L  = 0x63;
	UBRR1H  = 0x00;
	UCSR1B = 0x98;
}

//�� UART0 дһ�ֽ� 
int usart_putchar_0(char c) 
{ 
 	while( !(UCSR1A & (1<<UDRE1)) ); 
 	UDR1=c; 
	return 0; 
}


void puts_0(unsigned char *ptr,char num)
{
	int i = 0;
	while (i++ != num)
	{
		usart_putchar_0(*ptr++);
	}
}

