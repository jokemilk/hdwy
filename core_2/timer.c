#include "timer.h"


//��ʱ��T0��ʼ��
void timer0_init(void)
{
	TCCR0  = 0x00;//ֹͣ��ʱ��
	TCNT0  = 0x00;//��ʼֵ
	OCR0   = 149;//ƥ��ֵ
	TIMSK |= 0x02;//�ж�����
//	TCCR0  = 0x0D;//������ʱ��
}

//��ʱT1��ʼ��
void timer1_init(void)
{
	TCCR1B = 0x00;//ֹͣ��ʱ��
	TIMSK |= 0x10;//�ж�����
	TCNT1H = 0x00;
	TCNT1L = 0x00;//��ʼֵ
	OCR1AH = 0x00;
	OCR1AL = 0x00;//ƥ��Aֵ
	OCR1BH = 0xFE;
	OCR1BL = 0xFF;//ƥ��Bֵ
	ICR1H  = 0xFF;
	ICR1L  = 0xFF;//���벶׽ƥ��ֵ
	TCCR1A = 0x00;
}

