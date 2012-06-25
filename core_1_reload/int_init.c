#include "int_init.h"
#include <avr/io.h>
//外中断初始化
void int_init(void)
{
	// The low level of INT0 generates an interrupt request
//	EIMSK |= (1<<INT0); // External Interrupt Request 0 Enable
	// The low level of INT1 generates an interrupt request
	EICRA |= (1<<ISC11) | (1<<ISC10); // The rising edge of INT1 generates an interrupt request
	EIMSK |= (1<<INT1); // External Interrupt Request 1 Enable
}
