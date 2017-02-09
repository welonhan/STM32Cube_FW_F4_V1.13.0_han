#include "main.h"

#ifndef  __TOUCH_CTP_H__
#define  __TOUCH_CTP_H__

typedef struct
{
	uint16_t x; //CTP_X1
	uint16_t y; //CTP_Y1
	uint8_t pressed;
}TOUCH_XY_Typedef;

typedef struct 
{
	uint8_t packet_id;
	uint8_t points;				//
	uint8_t key;
	uint8_t xh;						//
	uint8_t xl;
	uint8_t yh;
	uint8_t yl;
}TOUCH_DATA_Typedef;
					
#define FT6206_ADDR		0x70
#define TOUCH_ADD			0x70  //0x38
						
void BSP_TOUCH_Init(void);
uint8_t BSP_TOUCH_Read(TOUCH_XY_Typedef CTP_Dat);
void BSP_TOUCH_Test(void);


                                  
#endif
