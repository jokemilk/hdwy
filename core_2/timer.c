#include "timer.h"


//定时器T0初始化
void timer0_init(void)
{
	TCCR0  = 0x00;//停止定时器
	TCNT0  = 0x00;//初始值
	OCR0   = 149;//匹配值
	TIMSK |= 0x02;//中断允许
//	TCCR0  = 0x0D;//启动定时器
}

//定时T1初始化
void timer1_init(void)
{
	TCCR1B = 0x00;//停止定时器
	TIMSK |= 0x10;//中断允许
	TCNT1H = 0x00;
	TCNT1L = 0x00;//初始值
	OCR1AH = 0x00;
	OCR1AL = 0x00;//匹配A值
	OCR1BH = 0xFE;
	OCR1BL = 0xFF;//匹配B值
	ICR1H  = 0xFF;
	ICR1L  = 0xFF;//输入捕捉匹配值
	TCCR1A = 0x00;
}

