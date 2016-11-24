#ifdef  use_capacitive_touch_panel
#include  "stm32f4_discovery_touch.h"

I2C_HandleTypeDef I2cHandle;

void GUI_TOUCH_X_ActivateX(void) {}
void GUI_TOUCH_X_ActivateY(void) {}

volatile u8 keyId = 0;
	

	
//*************************************************
//???? : void Touch_GPIO_Config(void)  
//???? : ?????IIC??,??????????IIC??
//???? : 
//???? : 
//???  : 
//*************************************************
void TOUCH_GPIO_Config(void) 
{
	GPIO_InitTypeDef  GPIO_InitStruct;
 	//I2C_HandleTypeDef I2cHandle;
	
  __HAL_RCC_GPIOB_CLK_ENABLE();
  
  /* Touch pin configuration */
	GPIO_Init_Structure.Mode      = GPIO_MODE_INPUT_PP;
  GPIO_Init_Structure.Pull      = GPIO_PULLUP;
  GPIO_Init_Structure.Speed     = GPIO_SPEED_HIGH;
    
  //TOUCH INT--------------PB7										 
  GPIO_Init_Structure.Pin   = GPIO_PIN_7  ;   
  HAL_GPIO_Init(GPIOA, &GPIO_Init_Structure);
	
	//TOUCH RESET------------PB8
  GPIO_Init_Structure.Pin   = GPIO_PIN_8  ;   
  HAL_GPIO_Init(GPIOA, &GPIO_Init_Structure);   
  
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
    Error_Handler();    
  }
  
  /* RESET THE TOUCH */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);			//TOUCH RESET=0 
	HAL_Delay(5);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);				//TOUCH RESET=1	
}
	


void EXTI4_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line4) != RESET)
	{
		EXTI->PR = EXTI_Line4;  				// ??????
    keyId = 1;
	}
}
	


/****************************************************************************************
																	??? FT6336????

***************************************************************************************/
typedef struct
{
	uint16_t cx1; //CTP_X1
	uint16_t cy1; //CTP_Y1
	uint16_t cx2; //CTP_X2
	uint16_t cy2; //CTP_Y2
}CTP_Stru;

CTP_Stru	CTP_Dat;

typedef struct 
{
	uint8_t packet_id;
	uint8_t xh_yh;
	uint8_t xl;
	uint8_t yl;
	uint8_t dxh_dyh;
	uint8_t dxl;
	uint8_t dyl;
  uint8_t checksum;
}TpdTouchData;

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

uint8_t FT6206_Read_Reg(uint8_t *pbuf,uint32_t len)
{
	
	int8_t i=0;

	I2C1_Start();
	I2C1_Send_Byte(FT6206_ADDR);
	I2C1_WaitAck();	
	
	I2C1_Send_Byte(0);
	I2C1_WaitAck();	
  I2C1_Stop();
  
	I2C1_Start();
	I2C1_Send_Byte(FT6206_ADDR+1);
	I2C1_WaitAck();	
	
	for(i=0;i<len;i++)
	{
		if(i==(len-1))  *(pbuf+i)=I2C1_Read_Byte(0);
		else            *(pbuf+i)=I2C1_Read_Byte(1);
	}		
	I2C1_Stop();
  
	return 0;
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
* Parameter      : startaddr: First address
*	pbuf	 				 : The Pointer Point to a buffer
*	len						 : The length of the read Data 
* Return         : 1:Fail; 0:Success
*******************************************************************************/

uint8_t CTP_Read(uint8_t flag)
{
	__IO uint16_t DCX = 0,DCY = 0;
	
	TpdTouchData TpdTouchData;

	//memset((uint8_t*)&TpdTouchData,0,sizeof(TpdTouchData));

	//Read the FT6206

	
	if(FT6206_Read_Reg((uint8_t*)&TpdTouchData, sizeof(TpdTouchData)))
	{
//		printf("FT6206 Read Fail!\r\n");
		return 0;
	}
	
	//Check The ID of FT6206
	if(TpdTouchData.packet_id != 0x52)	
	{
//		printf("The ID of FT6206 is False!\r\n");
		return 0;	
	}		
	
	//CheckSum
	if(!CheckSum((uint8_t*)(&TpdTouchData)))
	{
//		printf("Checksum is False!\r\n");
		return 0;
	}
	
	//The Key Of TP		
	if(TpdTouchData.xh_yh == 0xff && TpdTouchData.xl == 0xff
		&& TpdTouchData.yl == 0xff && TpdTouchData.dxh_dyh == 0xff && TpdTouchData.dyl == 0xff)
	{
		/*switch(TpdTouchData.dxl)
		{
			case 0:	return 0;
			case 1: printf("R-KEY\r\n"); break;	 //Right Key
			case 2: printf("M-KEY\r\n"); break;	 //Middle Key
			case 4: printf("L-KEY\r\n"); break;	 //Left Key
			default:;
		}*/		
	}
	else 
	{
		//The First Touch Point
		CTP_Dat.cx1 = (TpdTouchData.xh_yh&0xf0)<<4 | TpdTouchData.xl;
		CTP_Dat.cy1 = (TpdTouchData.xh_yh&0x0f)<<8 | TpdTouchData.yl;

		//The Second Touch Point
		if(TpdTouchData.dxh_dyh != 0 || TpdTouchData.dxl != 0 || TpdTouchData.dyl != 0)
		{	
			DCX = (TpdTouchData.dxh_dyh&0xf0)<<4 | TpdTouchData.dxl;
			DCY = (TpdTouchData.dxh_dyh&0x0f)<<8 | TpdTouchData.dyl;

			DCX <<= 4;
			DCX >>= 4;
			DCY <<= 4;
			DCY >>= 4;

			if(DCX >= 2048)
				DCX -= 4096;
			if(DCY >= 2048)
				DCY -= 4096;

			CTP_Dat.cx2 = CTP_Dat.cx1 + DCX;
			CTP_Dat.cy2 = CTP_Dat.cy1 + DCY;
		}		
	}

	if(CTP_Dat.cx1 == 0 && CTP_Dat.cy1 == 0 && CTP_Dat.cx2 == 0 && CTP_Dat.cy2 == 0)
	{
		return 0;
	}
	
//	if(flag)
//	{	
//		printf("#CP%04d,%04d!%04d,%04d;%04d,%04d\r\n",0,0,CTP_Dat.cx1,CTP_Dat.cy1,CTP_Dat.cx2,CTP_Dat.cy2);
//		memset((uint8_t*)&CTP_Dat, 0, sizeof(CTP_Dat));
//	}
	return 1;
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