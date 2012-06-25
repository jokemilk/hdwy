#include "timer.h"
#include <avr/io.h>
#include "macro.h"

//��ʱT1��ʼ��
void timer1_init(void)
{
	TCCR1B = 0x00; //ֹͣ��ʱ��
	TIMSK |= 0x10; //�ж�����
	TCNT1H = 0x00;
	TCNT1L = 0x00; //��ʼֵ
	OCR1A = 24000 - 1; //24000->100ms
	OCR1BH = 0x00;
	OCR1BL = 0x03; //ƥ��Bֵ
	ICR1H = 0xFF;
	ICR1L = 0xFF; //���벶׽ƥ��ֵ
	TCCR1A = 0x00;
//	TCCR1B = 0x09;//������ʱ��
}
