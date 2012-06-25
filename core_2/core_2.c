//包含所需头文件
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>

#include "stdio.h"
#include "math.h"

#include "marco.h"
#include "config_io.h"
#include "watchdog.h"
#include "usart.h"
#include "sinedata.h"
#include "timer.h"
#include "spi.h"

#define MY_PI	6.283118707

#define _PORT_MODE_

#define _WD_	//开看门狗

#define _7673_	//硬件、模拟da切换
//#define _PART_EEPROM_



#define	START_TIMER_0	TCCR0  = 0x0F;//启动定时器	分频 1/8/32/64/128/256/1024	1/2/3/4/5/6/7   1024
#define PAUSE_TIMER_0	TCCR0  = 0x00;//启动定时器

#define	START_TIMER_1	TCCR1B = 0x09;//启动定时器	分频 1/8/64/256/1024	1/2/3/4/5
#define PAUSE_TIMER_1	TCCR1B = 0x00;//停止定时器

#define	START_TIMER_2	TCCR2  = 0x01;//启动定时器	分频 1/8/64/256/1024	1/2/3/4/5
#define PAUSE_TIMER_2	TCCR2  = 0x00;//启动定时器

/**************************************** 全局变量 ****************************************/
volatile unsigned char command[CLen];
volatile unsigned char reply[4];

unsigned int	HIGH =4000;
int LOW = 0;

unsigned char	TURN	= 0;
unsigned long	PERIOD1	= 0;//周期
unsigned int	PERIOD2 = 0;
unsigned char	PERIOD3 = 0;
unsigned long	PERIOD4 = 0;

volatile unsigned char	UPDOWN = 0;


unsigned long	TIMER0_MASK = 0;
//unsigned long	TIMER1_MASK = 0;

volatile unsigned char	INC = 16;

volatile unsigned int	TRI_NUM = 10;


volatile unsigned long	VREF_1 = 1;
volatile unsigned int	VREF_2 = 2;
volatile unsigned int	VREF_3 = 0;
volatile unsigned int	VREF_4 = 0;//调整零飘

volatile unsigned char m_index = 0;
volatile char	flag_transover = 0;
volatile char	sys_status = 0;			// 0无配置无工作状态	1配置加载完成	未工作 2信号源工作中


volatile long	PH_index;

double t3 = 0;

//volatile char	TRH = 0xFD;
//volatile char	TRL = 0x00+67;


volatile char WAVE_MODE = 0;//0 正弦 	1 锯齿波	2 三角波	3 方波	 4 阶梯 	5 ***长时正弦	 6     7长时锯齿 8 长时三角 9 


/***************************************** 函数申明 ****************************************/
void init_devices(void);
void init_sys();
void usarterror();
void catched();
void request_set();
void request_on();

void volreset();

void sinset(unsigned long num);

void adc_init(void);

void write(unsigned int temp);

unsigned int adc_calc(void);

#ifdef _PORT_MODE_
//功能:使用SPI发送一个字节
void spi_write(uchar sData);
//功能:使用SPI接收一个字节
uchar spi_read(void);
#endif



#ifdef _PART_EEPROM_
void flashwrite(uchar num);
void flashread(uchar num);
#endif

/***************************************** 子函数 ****************************************/
void init_devices(void)
{
	cli(); //禁止所有中断
/**********************************/
	port_init();	//初始化端口
	usart_init();	//初始化串口
	adc_init();		//初始化ad
#ifdef _WD_
	timer0_init();
	START_TIMER_0;
	watchdog_ini();
#endif
	spi_init();
	timer1_init();	
//	START_TIMER_1;


/**********************************/
	sei();//开全局中断
}

void init_sys()
{
	m_index = 0;
	flag_transover = 0;
	PH_index=0;
	sys_status = 0;
}

void usarterror()
{
	int i;
	m_index = 0;
	flag_transover = 0;
//	sys_status = 0;
	for(i=0;i<CLen;i++)
	{
		command[i] = 0;	
	}		
	reply[0] = head;
	reply[1] = fail;
	reply[2] = ~fail;
	reply[3] = tail;
//	puts_0((unsigned char *)reply);	
	puts_0((unsigned char *)reply,4);
}

void catched()
{
	int i;
	reply[0] = head;
	reply[1] = command[3];
	reply[2] = ~reply[1];
	reply[3] = tail;
//	puts_0((unsigned char *)reply);
	puts_0((unsigned char *)reply,4);
	for(i=0;i<CLen;i++)
		command[i] = 0;
}


void request_set()
{
	reply[0] = head;
	reply[1] = req_set;
	reply[2] = ~req_set;
	reply[3] = tail;
	puts_0((unsigned char *)reply,4);
//	puts_0((unsigned char *)reply);
}

void request_on()
{
	reply[0] = head;
	reply[1] = req_on;
	reply[2] = ~req_on;
	reply[3] = tail;
//	puts_0((unsigned char *)reply);
	puts_0((unsigned char *)reply,4);
}

#ifdef _PART_EEPROM_
void flashwrite(uchar num)
{
	eeprom_busy_wait(); //等待 EEPROM 读写就绪 
	switch(num)
	{
		case	0:	eeprom_write_float(0,VREF);break;
		case	1:	eeprom_write_float(10,PHASE);break;
		case	2:	eeprom_write_word(20,PERIOD1);break;
		case	3:	eeprom_write_word(30,sys_status);break;
		default:break;
	}
}

void flashread(uchar num)
{
	eeprom_busy_wait(); //等待 EEPROM 读写就绪 
	switch(num)
	{
		case	0:	VREF = eeprom_read_float(0);break;
		case	1:	PHASE = eeprom_read_float(10);break;
		case	2:	PERIOD1 = eeprom_read_word(20);break;
		case	3:	sys_status = eeprom_read_word(30);break;
		default:break;
	}	
}

#endif


//功能:使用SPI发送一个字节
void spi_write(uchar sData)
{
	SPDR = sData;
	while(!(SPSR & BIT(SPIF)));
	//sData=SPDR;//读从机发回来的数据
}


//功能:使用SPI接收一个字节
uchar spi_read(void)
{
	SPDR = 0x00;
	while(!(SPSR & BIT(SPIF)));
	return SPDR;
}

void sinset(unsigned long num)
{
	OCR1A = 768-1;
	switch(num)
	{
		case 1:	INC = 16;TRI_NUM = 1;TURN = 0;PERIOD3 = 1;break;
		case 2:	INC = 8;TRI_NUM = 1;TURN = 0;PERIOD3 = 1;break;
		case 3:	INC = 16;TRI_NUM = 3;TURN = 6;PERIOD3 = 5;break;
		case 4:	INC = 4;TRI_NUM = 1;TURN = 0;PERIOD3 = 1;break;
		case 5:	INC = 16;TRI_NUM = 5;TURN = 6;PERIOD3 = 13;break;
		case 6:	INC = 8;TRI_NUM = 3;TURN = 6;PERIOD3 = 5;break;
		case 7:	INC = 16;TRI_NUM = 7;TURN = 6;PERIOD3 = 9;break;
		case 8:	INC = 2;TRI_NUM = 1;TURN = 0;PERIOD3 = 1;break;
		case 9:	INC = 16;TRI_NUM = 9;TURN = 6;PERIOD3 = 7;break;
		case 10:INC = 8;TRI_NUM = 5;TURN = 6;PERIOD3 = 13;break;
		case 11:INC = 16;TRI_NUM = 11;TURN = 6;PERIOD3 = 6;break;
		case 12:INC = 4;TRI_NUM = 3;TURN = 6;PERIOD3 = 22;break;
		case 13:INC = 16;TRI_NUM = 13;TURN = 6;PERIOD3 = 5;break;
		case 14:INC = 8;TRI_NUM = 7;TURN = 6;PERIOD3 = 9;break;
		case 15:INC = 16;TRI_NUM = 15;TURN = 6;PERIOD3 = 8;break;
		case 16:INC = 2;TRI_NUM = 2;TURN = 6;PERIOD3 = 32;break;
		case 17:INC = 16;TRI_NUM = 17;TURN = 6;PERIOD3 = 4;break;
		case 18:INC = 8;TRI_NUM = 9;TURN = 6;PERIOD3 = 7;break;
		case 19:INC = 16;TRI_NUM = 19;TURN = 6;PERIOD3 = 3;break;
		case 20:INC = 4;TRI_NUM = 5;TURN = 6;PERIOD3 = 13;break;
		case 21:INC = 16;TRI_NUM = 21;TURN = 6;PERIOD3 = 3;break;
		case 22:INC = 8;TRI_NUM = 11;TURN = 6;PERIOD3 = 6;break;
		case 23:INC = 16;TRI_NUM = 23;TURN = 6;PERIOD3 = 3;break;
		case 24:INC = 2;TRI_NUM = 3;TURN = 6;PERIOD3 = 22;break;
		case 25:INC = 16;TRI_NUM = 25;TURN = 6;PERIOD3 = 3;break;
		default	:	WAVE_MODE = 5;
					PERIOD1 = num*10;
					OCR1A = 15360-1;
					break;		
	}
	INC /=2;
}

void volreset()
{
	unsigned char status;
	status = MCUCSR;
	if((status&(BIT(2)|BIT(3))) != 0)	
	{
		#ifdef _PART_EEPROM_
		flashread(3);
		#endif	
		if(sys_status == 1)
		{
			request_set();
		}else if(sys_status == 2)
		{
			request_on();
		}
	}
}

void adc_init(void)
{
	//adc转换初始化
	ADCSRA	= 0x00;	//禁止AD转换
	ADMUX	= 0xC0;
	SFIOR |= 0x00;
	ACSR	= 0x80; //禁止模拟比较器
	ADCSRA	= 0x84;
}


unsigned int adc_calc(void)
{
	//计算实际电压
	unsigned long value=0;
	unsigned int voltage=0; //电压单位为(mV)
	value=ADCL;		 //首先读低位
	value|=(int)ADCH << 8; //然后读高位
	voltage=(value*2560)>>10;
	return voltage;
}

int readadc(void)
{
	unsigned int temp;	
	ADCSRA|=_BV(ADSC);
	while(ADCSRA&_BV(ADSC));
	temp = adc_calc();
	if(temp<2250 || temp>2750 )
		return 1;
	else
		return 0;
}

void write(unsigned int temp)
{
	unsigned char SB = 0;
/////////////////////////////////////////////////
	PORTB &= ~(BIT(0));
	SB = (char)((temp>>10)&0xff);
	spi_write(SB);
	SB = (char)((temp>>2)&0xff); 
	spi_write(SB);

	SB = (char)((temp<<6)&0xff); 
	spi_write(SB);
	PORTB ^= BIT(0);
/////////////////////////////////////////////////
}

/***************************************** 主函数 ****************************************/
int main(void)
{
	init_devices();
	init_sys();
// 	volreset();
	//在这继续添加你的代码
//	sinset(1);
//	OCR1A = 600-1;
//	PERIOD1 = 10;
//	PERIOD1 = 256*90;
//	START_TIMER_1;
//	TRH = 0xE8;TRL = 0x90; //timer 6000
//	TRH = 0xC4;TRL = 0x08+68; //25.6*600
//	TRH = 0xF4;TRL = 0x00+68; //256*12
//	PERIOD1 =10;
	while(1)
	{
		NOP();
//		if(readadc())
//			PAUSE_TIMER_0;			
	}
	return 0;
}

/***************************************** 中断处理 ****************************************/
//T0溢出中断服务程序
//#pragma interrupt_handler timer0_ovf_isr:10
//void timer0_ovf_isr(void)
SIGNAL(SIG_OUTPUT_COMPARE0)
{
	static int i=0;
	wdt_reset();
	if(i++ ==4)
	{
		i = 0;
		PORTG ^= BIT(0);
	}
}


//串行接收结束中断服务程序
//#pragma interrupt_handler usart_rx_isr:12
//void usart_rx_isr(void)
SIGNAL(SIG_UART1_RECV)
{
	int temp = 0;
	unsigned long temp2 = 0;
	unsigned long period = 0;
	unsigned int vmax = 0;
	static	char LEN = 0;
	static  char CHECK = 0;
	char status,data;
	status=UCSR1A;
	data=UDR1;
	if ((status & (BIT(4) | BIT(3) | BIT(2)))==0)
	   {
			if(m_index == 0 && data != head)
		   		{usarterror();return;}	
			else
				command[m_index++]=data;
			if(m_index == 2)
				LEN = data;
			if((m_index == 3)&&(LEN !=~data))
				{usarterror();return;}
//			if((index > 3)&&(index > LEN))
//				{usarterror();return;}
//			if((index == LEN)&&(data!=tail))
//				{usarterror();return;}
			if((m_index == LEN) && (m_index>1))
				flag_transover = 1;	
		}
	if(flag_transover)
	{
		m_index = 0;
		flag_transover = 0;
		if((char)command[3] == (char)(~command[4]))
		{
			switch(command[3])
			{
				case 1:	if(sys_status == 1)// 0无配置无工作状态	1配置加载完成	未工作 2信号源工作中
						{
							START_TIMER_1;
							sys_status = 2;
							#ifdef _PART_EEPROM_
							flashwrite(3);
							#endif
						}
						break;//1	加载；2	暂停；3	卸载；4	加载校验；5	数据传送；6 传送时间
				case 2:	if(sys_status == 2)
						{
							PAUSE_TIMER_1;
							sys_status = 1;	
							#ifdef _PART_EEPROM_
							flashwrite(3);
							#endif					
						}
						break;
				case 3: //if(sys_status == 2)
						{
							sys_status = 1;
							temp = 0;
							PH_index = 0;
							PAUSE_TIMER_1;
							_delay_ms(10);
							write(0x8000+ VREF_4);
							_delay_ms(10);
							write(0x8000+ VREF_4);
							_delay_ms(10);
							write(0x8000+ VREF_4);
						}
						break;
				case 4:	if(command[5] == 0)
						{
							temp = command[6]*256;
							temp += temp+command[7];
							temp +=0x8000;
						}else if(command[5] == 1)
						{
							temp = command[6]*256;
							temp += temp+command[7];
							temp =0x8000 - temp;
						}
						PAUSE_TIMER_1;
						NOP();
						write(temp);
						break;
				case 5: {
							CHECK = 0;
							for(temp = 5;temp <= 18;temp++)
							{
								CHECK ^= command[temp];
							}
							if((CHECK != (char)command[19])||((char)command[19]!=~(char)command[20]))
								{usarterror();return;}
							PERIOD1 = PERIOD2 = PERIOD3 =PERIOD4 = 0;
							VREF_1 = 1;
							VREF_2 = 0;
							TRI_NUM = 0;
							UPDOWN = 0;
/////////////////////////////////////////////////////////////////////period
							period = command[14]*100;
							period = period*100;
							temp2 = command[15]*100;
							period += temp2; 
							period += command[16];
/////////////////////////////////////////////////////////////////////vref3
							temp2 = command[11]*10000;
							vmax += command[12]*100;
							vmax += command[13];
							temp2 = 65536*vmax;
							temp2 /= 10000;
							temp2 = 65536 - temp2;
							temp2 /=2;
							VREF_3 = (unsigned int)temp2;
							VREF_3 = VREF_3+VREF_4;
							if(command[8])
							{
							// <0
								temp2 = command[9]*100;
								temp2 +=command[10];
								temp2 = 65536*temp2/10000;
								VREF_3 = VREF_3-temp2;	
							}
							else
							{
							// >0
								temp2 = command[9]*100;
								temp2 +=command[10];
								temp2 = 65536*temp2/10000;
								VREF_3 = VREF_3+temp2;	
							}							
/////////////////////////////////////////////////////////////////////ph
							temp2 = command[6]*100;
							PH_index = temp2;
							PH_index += command[7];
							switch(command[5])
							{
								case 0:	{
											WAVE_MODE = 0;
											sinset(period);
											t3 = 65536*8192;
											t3 *= vmax;
											t3 /=10000;
											t3 /=60000;
											VREF_1 = (unsigned long)t3;
											VREF_2 = 13;
											if(period <=25)
											{
												PH_index = PH_index*200*period/360;		
											}
											else
											{
												PH_index = PH_index*10*period/360;
											}
										}
										break;
								case 1:	{								
											if(period <= 100)
											{
												WAVE_MODE = 1;
												OCR1A = 600-1;
												PH_index = PH_index*256*period/360;
												PERIOD1 = period *256;
												t3 = 65536*8192;
												t3 *= vmax;
												t3 /=10000;
												t3 /=PERIOD1;
												VREF_1 = (unsigned long)t3;
												VREF_2 = 13;
											}
											else
											{
												OCR1A = 3072-1;
												WAVE_MODE = 7;
												PH_index = PH_index*50*period/360;
												PERIOD1 = period;
												t3 = 65536*8192;
												t3 *= vmax;
												t3 /=10000;
												t3 /=60000;
												VREF_1 = (unsigned long)t3;
												VREF_2 = 13;
											}	
										}
										break;
								case 2: {
											if(period <= 100)
											{
												WAVE_MODE = 2;
												OCR1A = 600-1;
												PERIOD1 = period *128;
												if(PH_index<=180)
												{
													UPDOWN = 0;
													PH_index = PH_index*256*period/360;
												}
												else
												{
													UPDOWN = 1;
													PH_index = (PH_index-180)*256*period/360;
												}												
												t3 = 65536*8192;
												t3 *= vmax;
												t3 /=10000;
												t3 /=PERIOD1;
												VREF_1 = (unsigned long)t3;
												VREF_2 = 13;
											}
											else
											{
												OCR1A = 3072-1;
												WAVE_MODE = 8;
												if(PH_index<=180)
												{
													UPDOWN = 0;
													PH_index = PH_index*50*period/360;
												}
												else
												{
													UPDOWN = 1;
													PH_index = (PH_index-180)*50*period/360;
												}												
												PERIOD1 = period;
												t3 = 65536*8192;
												t3 *= vmax;
												t3 /=10000;
												t3 /=60000;
												VREF_1 = (unsigned long)t3;
												VREF_2 = 13;
											}	
										}
										break;
								case 3:	{
											WAVE_MODE = 3;
											OCR1A = 768-1;
											PERIOD1 = 200*period;
											PH_index = PERIOD1*PH_index/360;////////
											VREF_1 = 1;
											VREF_2 = 0;
											t3 = 65536*vmax;
											t3 /=10000;
											HIGH = (unsigned int)t3;
											LOW = 0;	
										}
										break;
								case 4:	{
											WAVE_MODE = 4;
											OCR1A = 768-1;
											temp2 = command[17];
											TRI_NUM = temp2;
											TRI_NUM +=command[18];
											PERIOD4 = period * 200 / TRI_NUM;
											t3 = 65536*vmax;
											t3 /=10000;
											t3 /=(TRI_NUM-1);
											PERIOD2 = t3;
											PERIOD1 = TRI_NUM*PH_index/360;
											PH_index = 0;
										}
										break;
							}
							if(VREF_1 == 0)//avoid VREF_1 is 0
								VREF_1 = 1;
							sys_status = 1;
							#ifdef _PART_EEPROM_
							flashwrite(3);
							#endif
						}
						break;
				case 6:	if(command[5] == 0)
						{
							temp = command[6]*256;
							temp += temp+command[7];
							temp +=0x8000;
						}else if(command[5] == 1)
						{
							temp = command[6]*256;
							temp += temp+command[7];
							temp =0x8000 - temp;
						}
						if(command[5] == 0)
						{
							VREF_4 = command[6]*256;
							VREF_4 = VREF_4 + command[7];
						}else if(command[5] == 1)
						{
							VREF_4 = command[6]*256;
							VREF_4 = VREF_4 + command[7];
							VREF_4 = 65535-VREF_4+1;
						}
						PAUSE_TIMER_1;
						NOP();
						write(temp);
						break;
			}		
		}
		else
			{usarterror();return;}
		catched();	
	}
}

//定时器T1匹配中断A服务程序
//#pragma interrupt_handler timer1_compa_isr:8
//void timer1_compa_isr(void)
SIGNAL(SIG_OUTPUT_COMPARE1A)
{
	unsigned int temp = 0;
	unsigned long temp2 = 0;
	static unsigned int a,b,c;
	char SB;

	switch(WAVE_MODE)
	{
		case 0:	if(PERIOD2 > 0)
				{
					temp = (c*PERIOD2);
					if(TURN !=0)
						temp = temp>>6;
					temp = temp*PERIOD3;
					if(b>a)
					{							
						temp = a+temp;
					}	
					else if(a>temp)
					{
						temp = a-temp;
					}
					else
						temp = a;
				}
				else
				{
					a = pgm_read_word(&sinedata[PH_index]);
					b = pgm_read_word(&sinedata[PH_index+INC]);
					if(b>a)
						c = b-a;
					else
						c = a-b;
					temp = a;
				}
				PERIOD2++;
				if(PERIOD2 == TRI_NUM)
				{
					PH_index += INC;
					PERIOD2 = 0;
				}
				if(PH_index == 1600)
					PH_index = 0;
				break;
		case 1:	if(PH_index++<PERIOD1)
				{
					temp = PH_index - 1;
				}
				if(PH_index == PERIOD1)
				{
					PH_index = 0;
				}
				break;
		case 2:	if(UPDOWN == 0)
				{
					if(PH_index++<PERIOD1)
					{
						temp = PH_index;
					}
					if(PH_index == PERIOD1)
					{
						UPDOWN = 1;
						PH_index = 0;
					}
				}
				else
				{
					if(PH_index++<PERIOD1)
					{
						temp = PERIOD1 - PH_index;
					}
					if(PH_index == PERIOD1)
					{
						UPDOWN = 0;
						PH_index = 0;						
					}
				}
				break;
		case 3:	if(PH_index++<PERIOD1/2)
					{
						temp = HIGH;
					}
					else
					{
						temp = LOW;
					}
				if(PH_index == PERIOD1)	
					PH_index = 0;					
				break;
		case 4:	if(PERIOD1 < TRI_NUM)
				{
					if(PH_index++<PERIOD4)
					{
						temp2 = PERIOD1*PERIOD2;	
					}
					if(PH_index == PERIOD4)
					{
						PERIOD1++;
						PH_index = 0;
					}					
				}
				else
				{
					PERIOD1 = 0;
				}
				temp = (int)temp2;
				break;
		case 5:	temp = 30000*(sin(M_PI*2*PH_index/PERIOD1)+1);
				if(PH_index++ == PERIOD1)
					m_index = 0;
				break;
		case 7:	if(PH_index++ == (long)PERIOD1*50)
					PH_index = 0;
				temp2 = 1200*PH_index/PERIOD1;
				temp = (int)temp2;
				break;
		case 8:	if(UPDOWN == 0)
				{
					if(PH_index++ < (long)PERIOD1*25)
					{
						temp2 = 2400*PH_index/PERIOD1;
						temp = (int)temp2;
					}
					if(PH_index == (long)PERIOD1*25)
					{
						UPDOWN = 1;
						PH_index = 0;
					}
				}
				else
				{
					if(PH_index++ < (long)PERIOD1*25)
					{
						temp2 = 2400*((long)PERIOD1*25-PH_index)/PERIOD1;
						temp = (int)temp2;
					}
					if(PH_index == (long)PERIOD1*25)
					{
						UPDOWN = 0;
						PH_index = 0;						
					}
				}
				break;
		case 9: break;			
	}
	temp2 = (long)temp*VREF_1;
	temp2 = temp2>>VREF_2;
	temp = (int)temp2;
	temp = temp + VREF_3;
#ifdef _7673_
/////////////////////////////////////////////////
	PORTB &= ~(BIT(0));
	SB = (char)((temp>>10)&0xff);
	spi_write(SB);
	SB = (char)((temp>>2)&0xff); 
	spi_write(SB);

	SB = (char)((temp<<6)&0xff); 
	spi_write(SB);
	PORTB ^= BIT(0);
/////////////////////////////////////////////////
#else
	PORTB &= ~(BIT(0));
	SB = (char)((temp>>8)&0xff);
	spi_write(SB);
	SB = (char)(temp&0xff); 
	spi_write(SB);
	PORTB ^= BIT(0);
	SB = (char)(0); 
	spi_write(SB);
#endif
}

/*******************************************************************************************/


