#include "watchdog.h"

void watchdog_ini()
{
	wdt_enable(WDTO_1S);
	wdt_reset();//Î¹¹· 
}
