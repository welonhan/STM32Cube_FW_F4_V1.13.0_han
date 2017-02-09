/**
  ******************************************************************************
  * @file    drv_adc.c
  * @author  Hanwl
  * @version 
  * @date    22-Dec-2016
  * @brief   
	******************************************************************************
 **/

#include  "stm32f4_discovery_touch.h"
#include 	"stm32f4_discovery_lcd.h"

extern I2C_HandleTypeDef I2cHandle;
extern TOUCH_XY_Typedef TOUCH_Dat;

		
/*************************************************
* Function Name  : BSP_TOUCH_Init
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
			TOUCH_Dat.y = 480-(((uint16_t)(tp_data[3] )& 0x0F)<<8 | (uint16_t)(tp_data[4]));
			TOUCH_Dat.x = (((uint16_t)(tp_data[5] )& 0x0F)<<8 | (uint16_t)(tp_data[6]));
			TOUCH_Dat.pressed=1;
			return 0;
		}
		else
		{
			//TOUCH_Dat.y = ((uint16_t)(tp_data[3] )& 0x0F)<<8 | (uint16_t)(tp_data[4]);
			//TOUCH_Dat.x = 800-(((uint16_t)(tp_data[5] )& 0x0F)<<8 | (uint16_t)(tp_data[6]));
			TOUCH_Dat.pressed=0;
		}
	}
	return 1;
}




/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
