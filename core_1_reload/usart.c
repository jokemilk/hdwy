#include <avr/io.h>
#include <avr/interrupt.h>
#include "USART.h"

//����ͨ�ų�ʼ��
void usart_init(void)
{
//����0
	UCSR0B = 0x00; //��ֹ�ж�19200
	UCSR0A = 0x00;
	UCSR0C = 0x26;
	UBRR0L = 24;
	UBRR0H = 0x00;
	UCSR0B = 0x98;
//����1
	UCSR1B = 0x00; //��ֹ�ж�
	UCSR1A = 0x00;
	UCSR1C = 0x26;
	UBRR1L = 0x63;
	UBRR1H = 0x00;
	UCSR1B = 0x98;
}

//�� UART0 дһ�ֽ� 
int usart_putchar_0(char c)
{
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
	return 0;
}

//�� UART1 дһ�ֽ� 
int usart_putchar_1(char c)
{
	loop_until_bit_is_set(UCSR1A, UDRE1);
	UDR1 = c;
	return 0;
}

void puts_0(unsigned char *ptr, char num)
{
	int i = 0;
	while (i++ != num)
	{
		usart_putchar_0(*ptr++);
	}
}

void puts_1(unsigned char *ptr, char num)
{
	int i = 0;
	while (i++ != num)
	{
		usart_putchar_1(*ptr++);
	}
}
