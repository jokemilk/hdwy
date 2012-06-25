#include "watchdog.h"

//��ʱ��T0��ʼ��
void timer0_init(void)
{
	TCCR0 = 0x00; //ֹͣ��ʱ��
	TCNT0 = 0x00; //��ʼֵ
	OCR0 = 149; //ƥ��ֵ
	TIMSK |= 0x02; //�ж�����
	TCCR0 = 0x0F; //������ʱ��
}

void watchdog_ini()
{
	timer0_init();
	wdt_enable(WDTO_2S);
	wdt_reset();
	//ι��
}
