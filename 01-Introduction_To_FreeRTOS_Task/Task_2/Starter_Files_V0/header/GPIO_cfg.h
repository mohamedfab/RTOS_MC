

#ifndef GPIO_CFG_H_
#define GPIO_CFG_H_

/************* Type def section ************/
#include "GPIO.h"
#include "stdint.h"
typedef struct
{
	portX_t Port;
	pinX_t Pin;
	pinDir_t Direction;
	
}PinConfig_t;


extern PinConfig_t PinConfig_array[];
extern uint16_t PinConfig_array_size;


#endif 
