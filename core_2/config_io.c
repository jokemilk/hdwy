#include "config_io.h"
#include <avr/io.h>
#include "marco.h"

void port_init(void)
{
	PORTA = 0x00;
	DDRA  = 0x00;
	PORTB = 0x00|BIT(0);
	DDRB  = 0x00|BIT(0)|BIT(2)|BIT(1);
	PORTC = 0x00;
	DDRC  = 0x00;
	PORTD = 0x00;
	DDRD  = 0x00;
	PORTE = 0x00;
	DDRE  = 0x00;
	PORTF = 0x00;
	DDRF  = 0x00;
	PORTG = 0x00;
	DDRG  = 0x00|BIT(0);
}
