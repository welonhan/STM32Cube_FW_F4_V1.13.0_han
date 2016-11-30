#include  "stm32f4_discovery_touch.h"
#include "stm32f4_discovery_lcd.h"

extern I2C_HandleTypeDef I2cHandle;
extern TOUCH_XY_Typedef TOUCH_Dat;
//volatile u8 keyId = 0;
	

	
/*************************************************
* Function Name  : BSP_LCD_Init
* Description    : Init I2C, INT, RESET GPIO
* Parameter      : 	
* Return         : 
**************************************************/
void BSP_TOUCH_Init(void) 
{
	GPIO_InitTypeDef  GPIO_InitStruct;
 	//I2C_HandleTypeDef I2cHandle;
	
  __HAL_RCC_GPIOB_CLK_ENABLE();
  
  /* Touch pin configuration */
	GPIO_InitStruct.Mode 			= GPIO_MODE_IT_FALLING; 
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_LOW;
    
  //TOUCH INT--------------PB7										 
  GPIO_InitStruct.Pin   = GPIO_PIN_7  ;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
   
  /* Enable and set Button EXTI Interrupt to the lowest priority */
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 7);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
  	
	
	//TOUCH RESET------------PB8
	GPIO_InitStruct.Mode 	= GPIO_MODE_OUTPUT_PP; 
  GPIO_InitStruct.Pin   = GPIO_PIN_8  ;   
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);   
  
   /*##-1- Configure the I2C peripheral ######################################*/
  I2cHandle.Instance             = I2Cx;
  
  I2cHandle.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
  I2cHandle.Init.ClockSpeed      = 400000;
  I2cHandle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  I2cHandle.Init.DutyCycle       = I2C_DUTYCYCLE_16_9;
  I2cHandle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  I2cHandle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;
  //I2cHandle.Init.OwnAddress1     = I2C_ADDRESS;
  //I2cHandle.Init.OwnAddress2     = 0xFE;
  
  if(HAL_I2C_Init(&I2cHandle) != HAL_OK)
  {
    /* Initialization Error */
    //Error_Handler();   
		printf("touch init fail\n\r");
  }
  
  /* RESET THE TOUCH */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);			//TOUCH RESET=0 
	HAL_Delay(5);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);				//TOUCH RESET=1	
}
	
/*******************************************************************************
* Function Name  : FT6206_Read_Reg
* Description    : Read The FT6206
* Parameter      : startaddr: First address
*	pbuf	 				 : The Pointer Point to a buffer
*	len						 : The length of the read Data 
* Return         : 1:Fail; 0:Success
*******************************************************************************/
uint8_t FT6206_Read_Reg(uint8_t *pbuf,uint32_t len)
{
	//HAL_I2C_Mem_Read_IT
	//uint8_t *p_data_addr=0;

	if(HAL_I2C_Mem_Read(&I2cHandle, FT6206_ADDR, 0,I2C_MEMADD_SIZE_8BIT , pbuf, len, 10000) !=HAL_OK)
	{
		printf("FT6206_Read_Reg error\n\r");
		return 1;
	}
	return 0;
}

static uint8_t CheckSum(uint8_t *buf)
{
	__IO uint8_t i;
	__IO uint16_t sum = 0;

	for(i=0;i<7;i++)
	{
		sum += buf[i];		
	}

	sum &= 0xff;
	sum = 0x0100-sum;
	sum &= 0xff;

	return (sum == buf[7]);
}

/*******************************************************************************
* Function Name  : FT6206_Read_Data
* Description    : Read The FT6206
* Parameter      : CTP_Dat: 
* Return         : 1:Fail; 0:Success
*******************************************************************************/

uint8_t BSP_TOUCH_Read(TOUCH_XY_Typedef CTP_Dat)
{
	__IO uint16_t DCX = 0,DCY = 0;
	uint8_t tp_data[7];
	//TOUCH_DATA_Typedef TpdTouchData;
	//TOUCH_XY_Typedef CTP_Dat;
	//memset((uint8_t*)&TpdTouchData,0,sizeof(TpdTouchData));
	
	if(FT6206_Read_Reg(tp_data, 7)==0)
	{
		if ((tp_data[2]&0x0f) == 1)
		{
			TOUCH_Dat.y = ((uint16_t)(tp_data[3] )& 0x0F)<<8 | (uint16_t)(tp_data[4]);
			TOUCH_Dat.x = 800-(((uint16_t)(tp_data[5] )& 0x0F)<<8 | (uint16_t)(tp_data[6]));
			return 0;
		}
	}
	return 1;
}

void BSP_TOUCH_Test(void)
{
	BSP_TOUCH_Read(TOUCH_Dat);
	if(BSP_TOUCH_Read(TOUCH_Dat)==0)
	{
		BSP_LCD_DrawPixel(TOUCH_Dat.x, TOUCH_Dat.y, WHITE);
	}
	else
		printf("touch test fail\n\r");
}

#if 0
static uint8_t FT6206_Write_Reg(uint8_t startaddr,uint8_t *pbuf,uint32_t len)
{
	for(i=0;i<len;i++,pbuf++)
		I2Cx_WriteData(FT6206_ADDR, startaddr, *pbuf);	
}


uint8_t FT6206_Read_Reg1(uint8_t RegIndex)
{
	uint8_t value;
	value=I2Cx_ReadData(FT6206_ADDR, RegIndex);
	return value;
}

void FT6206_Write_Reg1(u8 RegIndex,u8 data)
{
	I2Cx_WriteData(FT6206_ADDR, RegIndex, data); 
}


void FT6206_Read_RegN(u8 *pbuf,u8 len)
{
	u8 i;
	I2C1_Start();
	I2C1_Send_Byte(FT6206_ADDR+1);
	I2C1_WaitAck();	
	
	for(i=0;i<len;i++)
	{
		if(i==(len-1))  *(pbuf+i)=I2C1_Read_Byte(0);
		else            *(pbuf+i)=I2C1_Read_Byte(1);
	}		
	I2C1_Stop();
}
/*******************************************************************************
* Function Name  : FT6206_Read_Reg
* Description    : Read The FT6206
* Parameter      : 	startaddr: First address
*				   					pbuf: The Pointer Point to a buffer
*										len: The length of the read Data 
* Return         : 1:Fail; 0:Success
*******************************************************************************/
uint8_t FT6206_Read_Reg0(uint8_t startaddr,uint8_t *pbuf,uint32_t len)
{
	__IO uint16_t i = 0;
	
	I2C1_Start();
	//I2C1_Send_Byte(FT6206_ADDR);	//Send Slave Address for Write

	do
	{
		I2C1_Send_Byte(FT6206_ADDR);	//Send Slave Address for Write
		if(++i>100)
		{
			I2C1_Stop();
			return 1;
		}
	}while(I2C1_WaitAck());	//Read Ack
///////////////////////////////////////////////////	
	i = 0;		
	do
	{	
		I2C1_Send_Byte(startaddr);	//Send Slave Address for Read
		if(++i > 100)	//Time Out
		{
			I2C1_Stop();
			return 1;
		}
			
	}while(I2C1_WaitAck());//Read Ack
///////////////////////////////////////////////////
	I2C1_Start();
	i = 0;		
	do
	{	
		I2C1_Send_Byte(FT6206_ADDR | 0x01);	//Send Slave Address for Read
			
		if(++i >= 100)	//Time Out
		{
			I2C1_Stop();
			return 1;
		}
			
	}while(I2C1_WaitAck());//Read Ack
	
	//len-1 Data
	for(i=0; i<len-1; i++)
	{
		pbuf[i] = I2C1_Read_Byte(1);	//??1???
		I2C1_Ack();	 //Ack
	}

	pbuf[i] = I2C1_Read_Byte(0);	//Read the last Byte
	I2C1_NoAck(); // NoAck
		
	I2C1_Stop();	 

	return 0;	 
}



u8 a,buf[10];
volatile static u16 touchX=0,touchY=0,lastY=0;

int GUI_TOUCH_X_MeasureX(void)
{
	FT6206_Read_Reg((uint8_t*)&buf, 7);

	if ((buf[2]&0x0f) == 1)
	{
		touchX = (s16)(buf[5] & 0x0F)<<8 | (s16)buf[6];
		touchY = (s16)(buf[3] & 0x0F)<<8 | (s16)buf[4];
		if(touchY==0)
			touchX=0;	
	}
	else
	{
		touchY =0;
		touchX =0;		
	}
	return touchX;
}

int GUI_TOUCH_X_MeasureY(void)
{
	return touchY;
}

void Touch_Test(void)
{
  int16_t x1,y1;

	GUI_SetColor(GUI_BLUE);
	GUI_SetFont(&GUI_Font32B_1);
  GUI_DispStringAt("x =",60,0);
	GUI_DispStringAt("y =",160,0);
	while(1)
	{
//		FT6206_Write_Reg1(0,0);
//		a = FT6206_Read_Reg1(0);
//		GUI_DispDecAt(a, 0, 50, 4);	
//		
//		a = FT6206_Read_Reg1(0xa3);
//		GUI_DispDecAt(a, 0, 0, 4);
//		a = FT6206_Read_Reg1(0xa6);
//		GUI_DispDecAt(a, 100, 0, 4);
//		a = FT6206_Read_Reg1(0xa8);
//		GUI_DispDecAt(a, 200, 0, 4);

//		a = FT6206_Read_Reg1(0xa7);
//		GUI_DispDecAt(a, 300, 0, 4);	
		
		if(1)
		{
//			keyId = 0;
			FT6206_Read_Reg((uint8_t*)&buf, 7);
	
			if ((buf[2]&0x0f) == 1)
			{
				//??????480*800
				x1 = (s16)(buf[3] & 0x0F)<<8 | (s16)buf[4];
				y1 = (s16)(buf[5] & 0x0F)<<8 | (s16)buf[6];
			}
			else
			{
				x1 = 0xFFFF;
				y1 = 0xFFFF;
			}
	    if((x1>0)||(y1>0))
			{
				GUI_DispDecAt(x1, 100, 0, 3);
				GUI_DispDecAt(y1, 200, 0, 3);
			}		
		}

	}
}


#endif
