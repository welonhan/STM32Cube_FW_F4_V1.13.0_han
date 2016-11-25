#include "main.h"

#ifndef  __TOUCH_CTP_H__
#define  __TOUCH_CTP_H__

typedef struct
{
	uint16_t cx1; //CTP_X1
	uint16_t cy1; //CTP_Y1
	uint16_t cx2; //CTP_X2
	uint16_t cy2; //CTP_Y2
}TOUCH_XY_Typedef;

typedef struct 
{
	uint8_t packet_id;
	uint8_t xh_yh;				//
	uint8_t xl;						//
	uint8_t yl;
	uint8_t dxh_dyh;
	uint8_t dxl;
	uint8_t dyl;
  uint8_t checksum;
}TOUCH_DATA_Typedef;
					
#define FT6206_ADDR		0x70
#define TOUCH_ADD			0x70  //0x38
						
void BSP_TOUCH_Init(void);
uint8_t BSP_TOUCH_Read(TOUCH_XY_Typedef CTP_Dat);
void BSP_TOUCH_Test(void);


                                  
#endif
