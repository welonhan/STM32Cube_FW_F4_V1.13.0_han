/**
  ******************************************************************************
  * @file    main.c
  * @author  Hanwl
  * @version V1.0
  * @date    2016-12-19
  * @brief   Main program body
  ******************************************************************************
**/

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4_discovery_lcd.h"
#include "stm32f4_discovery_uart.h"
#include "stm32f4_discovery_touch.h"
#include "stm32f4_discovery_sd.h"
#include "drv_RF24l01.h"
#include "drv_adc.h"

/* FatFs includes component */
#include "ff_gen_drv.h"
#include "sd_diskio.h"
#include <stdio.h>


/*GUI includes component */
#include "GUI.h"
#include "GUIApp.h"
#include "DIALOG.h"

/* TOUCH ---------------------------------------------------------------------*/
I2C_HandleTypeDef I2cHandle;
TOUCH_XY_Typedef TOUCH_Dat;

/* UART ----------------------------------------------------------------------*/
UART_HandleTypeDef UartHandle;
__IO ITStatus UartReady = RESET;

/* SD card -------------------------------------------------------------------*/
SD_HandleTypeDef uSdHandle;
SD_CardInfo uSdCardInfo;

/* LCD backlight PWM ---------------------------------------------------------*/
TIM_HandleTypeDef    TimHandle;
TIM_OC_InitTypeDef sConfig;

/* ADC -----------------------------------------------------------------------*/
ADC_HandleTypeDef  AdcHandle;
uint32_t ADC_Bat_Num=0, ADC_Tmp_Num=0;
uint8_t ADC_Count=0;


/* FATFS ---------------------------------------------------------------------*/
FATFS SDFatFs;  /* File system object for SD card logical drive */
FIL MyFile;     /* File object */
char SDPath[4]; /* SD card logical drive path */
uint8_t duty=0;

uint8_t GUI_Initialized   = 0;

GUI_PID_STATE Touch_State;

/* NRF24L01 ------------------------------------------------------------------*/
#ifdef DEBUG_MODE

char		NRF24L01_TX_DATA[32]="aaaaaaaaaaaaaaaaaaaazzzzzzzzzzb";
uint8_t NRF24L01_ACK_DATA[32]={0};
uint8_t NRF24L01_ACK[8]={88,00,99,99,00,00,66,55};
uint8_t NRF24L01_TX_NUM=sizeof(NRF24L01_TX_DATA);
uint8_t NRF24L01_ACK_NUM=sizeof(NRF24L01_ACK_DATA);
uint16_t ADC_DATA_Debug[4]={0,0,0,0};
uint32_t *DMA_ADC_MEM_ADDR =(uint32_t *)&ADC_DATA_Debug[0];
RC_DATA_TypeDef RC_DATA;

#else
RC_DATA_TypeDef RC_DATA;
uint8_t 	NRF24L01_TX_NUM=sizeof(RC_DATA.RC_TX_Data);
uint8_t 	NRF24L01_ACK_NUM=sizeof(RC_DATA.RC_ACK_Data);
uint32_t 	*DMA_ADC_MEM_ADDR=(uint32_t *)&RC_DATA.ADC_DATA;

#endif

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);

void 				FATFS_Test(void);


uint8_t DASH_BOARD[4]=
{
	180,180,180,180
};

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
		/* STM32F4xx HAL library initialization:
       - Configure the Flash prefetch, instruction and Data caches
       - Configure the Systick to generate an interrupt each 1 msec
       - Set NVIC Group Priority to 4
       - Global MSP (MCU Support Package) initialization
     */
	uint32_t i,temp1=0,temp2=0;
  HAL_Init();
	
	//0 bits for preemption priority
  //4 bits for subpriority
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_0);
  
  /* Configure the system clock to 168 MHz */
  SystemClock_Config();
  
  /* Configure LED1 and LED3 */
  BSP_LED_Init(LED4);
  BSP_LED_Init(LED3);
	
	BSP_PB_Init(BUTTON_KEY,BUTTON_MODE_EXTI);
	
	BSP_UART2_Init();	
	printf("UART2 init ok!\n\r");
	
	LCD_BACKLIGHT_PWM_Init();
	
	BSP_LCD_Init();
	printf("LCD init ok!\n\r");
	
	BSP_TOUCH_Init();
	
	BSP_ADC_Init();
	
	for(i=0;i<10;i++)
	{
		temp1+=RC_DATA.ADC_DATA.ADC_STR;
		temp2+=RC_DATA.ADC_DATA.ADC_THRT;
		HAL_Delay(2);
	}
	RC_DATA.ADC_StrMid=(uint16_t)(temp1/10);
	RC_DATA.ADC_ThrtMid=(uint16_t)(temp2/10);

	//NRF24L01_check();
	
	NRF24L01_Init();	
	
	//FATFS_Test();
	
	//STemWin need enable CRC
	__HAL_RCC_CRC_CLK_ENABLE();
	
/* Activate the use of memory device feature */
  //WM_SetCreateFlags(WM_CF_MEMDEV);
  
  /* Init the STemWin GUI Library */
  GUI_Init();
  
  GUI_Initialized = 1;

  /* Start Demo */
  MainTask();
	
/*	
	while(1)
	{
		//定时发送遥控数据，延时可根据需要修改
		HAL_Delay(20);
		RC_TX_DataHandle(&RC_DATA);
		NRF24L01_TxPacket( (uint8_t *)&RC_DATA.RC_TX_Data	, NRF24L01_TX_NUM );
		
	}
*/	
}

/**
  * @brief  FATFS SD card test
  * @param  None
  * @retval None
  */
void FATFS_Test(void)
{
	FRESULT res;                                          /* FatFs function common result code */
  uint32_t byteswritten, bytesread;                     /* File write/read counts */
  uint8_t wtext[] = "This is STM32 working with FatFs"; /* File write buffer */
  uint8_t rtext[100];                                   /* File read buffer */
  /*##-1- Link the micro SD disk I/O driver ##################################*/
  if(FATFS_LinkDriver(&SD_Driver, SDPath) == 0)
  {
    /*##-2- Register the file system object to the FatFs module ##############*/
    if(f_mount(&SDFatFs, (TCHAR const*)SDPath, 0) != FR_OK)
    {
      /* FatFs Initialization Error */
      Error_Handler();
    }
    else
    {
      /*##-3- Create a FAT file system (format) on the logical drive #########*/
      /* WARNING: Formatting the uSD card will delete all content on the device */
      if(f_mkfs((TCHAR const*)SDPath, 0, 0) != FR_OK)
      {
        /* FatFs Format Error */
        Error_Handler();
      }
      else
      {
        /*##-4- Create and Open a new text file object with write access #####*/
        if(f_open(&MyFile, "STM32.TXT", FA_CREATE_ALWAYS | FA_WRITE) != FR_OK)
        {
          /* 'STM32.TXT' file Open for write Error */
          Error_Handler();
        }
        else
        {
          /*##-5- Write data to the text file ################################*/
          res = f_write(&MyFile, wtext, sizeof(wtext), (void *)&byteswritten);
          
          if((byteswritten == 0) || (res != FR_OK))
          {
            /* 'STM32.TXT' file Write or EOF Error */
            Error_Handler();
          }
          else
          {
            /*##-6- Close the open text file #################################*/
            f_close(&MyFile);
            
            /*##-7- Open the text file object with read access ###############*/
            if(f_open(&MyFile, "STM32.TXT", FA_READ) != FR_OK)
            {
              /* 'STM32.TXT' file Open for read Error */
              Error_Handler();
            }
            else
            {
              /*##-8- Read data from the text file ###########################*/
              res = f_read(&MyFile, rtext, sizeof(rtext), (UINT*)&bytesread);
              
              if((bytesread == 0) || (res != FR_OK))
              {
                /* 'STM32.TXT' file Read or EOF Error */
                Error_Handler();
              }
              else
              {
                /*##-9- Close the open text file #############################*/
                f_close(&MyFile);
                
                /*##-10- Compare read data with the expected data ############*/
                if((bytesread != byteswritten))
                {                
                  /* Read data is different from the expected data */
                  Error_Handler();
                }
                else
                {
                  /* Success of the demo: no error occurrence */
                  BSP_LED_On(LED4);
                }
              }
            }
          }
        }
      }
    }
  }
  
  /*##-11- Unlink the micro SD disk I/O driver ###############################*/
  FATFS_UnLinkDriver(SDPath);	
	
}


/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 168000000
  *            HCLK(Hz)                       = 168000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 8
  *            PLL_N                          = 336
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();
  
  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }

  /* STM32F405x/407x/415x/417x Revision Z devices: prefetch is supported  */
  if (HAL_GetREVID() == 0x1001)
  {
    /* Enable the Flash prefetch */
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
  }
}

/**
  * @brief EXTI line detection callbacks
  * @param GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	uint8_t NRF24L01_IRQ_status;
  if(GPIO_Pin == TOUCH_INT_PIN)										//PB7 TOUCH INT
  {

    if(BSP_TOUCH_Read(TOUCH_Dat)==0)
		{	/*
			BSP_LCD_DrawPixel(TOUCH_Dat.x, TOUCH_Dat.y, RED);
			BSP_LCD_DrawPixel(TOUCH_Dat.x, TOUCH_Dat.y+1, RED);
			BSP_LCD_DrawPixel(TOUCH_Dat.x+1, TOUCH_Dat.y, RED);
			BSP_LCD_DrawPixel(TOUCH_Dat.x+1, TOUCH_Dat.y, RED);
			
			Touch_State.Layer=0;
			Touch_State.x=TOUCH_Dat.x;
			Touch_State.y=TOUCH_Dat.y;
			Touch_State.Pressed=TOUCH_Dat.pressed;
			GUI_PID_StoreState(&Touch_State);
			*/
			BSP_LED_Toggle(LED4);
		}
		else
			Touch_State.Pressed=TOUCH_Dat.pressed;
			
		
  }
	
	if(GPIO_Pin == KEY_BUTTON_PIN)									//PA0 KEY INT
  {
    BSP_LED_Toggle(LED3);

#ifdef DEBUG_MODE		
		NRF24L01_TxPacket( (uint8_t *)NRF24L01_TX_DATA, NRF24L01_TX_NUM );


		if (duty==0)
		{	LCD_BACKLIGHT_PWM_50duty();
			duty=1;
		}
			else 
		{
			LCD_BACKLIGHT_PWM_25duty();
			duty=0;
		}
#endif		
  }
	
	if(GPIO_Pin == RF24L01_IRQ_GPIO_PIN)						//RF24L01 IRQ
	{
#ifdef DEBUG_MODE
		NRF24L01_IRQ_status=NRF24L01_IRQ((uint8_t *) &NRF24L01_ACK_DATA,NRF24L01_ACK_NUM);
		if(NRF24L01_IRQ_status ==(TX_OK|RX_OK))
		{	
			for(i=0;i<8;i++)
			{
				if(NRF24L01_ACK[i]!=NRF24L01_ACK_DATA[i])
					return;
			}	
			BSP_LED_Toggle(LED4);			
		}
#else
		NRF24L01_IRQ_status=NRF24L01_IRQ((uint8_t *)&RC_DATA.RC_ACK_Data.RX_BAT_L,NRF24L01_ACK_NUM);	
		if(NRF24L01_IRQ_status ==(TX_OK|RX_OK))
		{	
			RC_DATA.RX_BAT=(uint16_t)RC_DATA.RC_ACK_Data.RX_BAT_H;
			RC_DATA.RX_BAT=(RC_DATA.RX_BAT <<8) + RC_DATA.RC_ACK_Data.RX_BAT_L;
			BSP_LED_Toggle(LED4);			
		}
#endif		
	}
	
}

/**
  * @brief  处理转向油门ADC数据
  * @param  RC_DATA_TypeDef
  * @retval None
  */
void RC_TX_DataHandle(RC_DATA_TypeDef* rc_data)
{
	int8_t tmp;
	uint32_t temp;
	
	/////////////////////////////////////////////////////////////////////////
	//转向采样小于回中值，向左转, 0-100
	if(rc_data->ADC_StrMid>rc_data->ADC_DATA.ADC_STR)
	{	
		tmp=(100*(rc_data->ADC_StrMid-rc_data->ADC_DATA.ADC_STR))/rc_data->ADC_StrMid;
		if(tmp<=10)
			tmp=0;
				
		else
			tmp=-tmp;
		rc_data->RC_TX_Data.CHANNEL1=100+tmp;
		
	}
	//转向采样大于回中值，向右转 100-200
	else
	{
		tmp=(100*(rc_data->ADC_DATA.ADC_STR-rc_data->ADC_StrMid))/(0xFFF-rc_data->ADC_StrMid);
		if(tmp<=10)
			tmp=0;
					
		else
			tmp=tmp;
			rc_data->RC_TX_Data.CHANNEL1=100+tmp;		
	}
	
	///////////////////////////////////////////////////////////////////////////
	//处理channel1 指示数据
	if(rc_data->RC_TX_Data.CHANNEL1<=100)
	{
		tmp=(45*rc_data->RC_TX_Data.CHANNEL1)/100;
		DASH_BOARD[0]=135+tmp;
	}
	else
	{
		tmp=(45*(rc_data->RC_TX_Data.CHANNEL1-100))/100;
		DASH_BOARD[0]=180+tmp;
	}
	
	///////////////////////////////////////////////////////////////////////////
	//油门采样小于回中值，向前，100-200
	if(rc_data->ADC_ThrtMid>rc_data->ADC_DATA.ADC_THRT)
	{	
		tmp=(100*(rc_data->ADC_ThrtMid-rc_data->ADC_DATA.ADC_THRT))/rc_data->ADC_ThrtMid;
		if(tmp<=10)
			tmp=0;
					
		else
			tmp=tmp;
		rc_data->RC_TX_Data.CHANNEL2=100+tmp;
					
	}
	//转向采样大于回中值，向后，0-100
	else
	{
		tmp=(100*(rc_data->ADC_DATA.ADC_THRT-rc_data->ADC_ThrtMid))/(0xFFF-rc_data->ADC_ThrtMid);
		if(tmp<=10)
			tmp=0;
		else 
			tmp=-tmp;
		rc_data->RC_TX_Data.CHANNEL2=100+tmp;
	}
	///////////////////////////////////////////////////////////////////////////
	//处理channel2 指示数据
	if(rc_data->RC_TX_Data.CHANNEL2<100)
	{
		tmp=(45*rc_data->RC_TX_Data.CHANNEL2)/100;
		DASH_BOARD[1]=135+tmp;
	}
	else
	{
		tmp=(45*(rc_data->RC_TX_Data.CHANNEL2-100))/100;
		DASH_BOARD[1]=180+tmp;
	}	
	
	//温度处理, 单位摄氏度
	//(Vsense-V25)/Vslope+25, Vref=2890mV, Vsense=Vref*Adc>>12, V25=760mV, Vslope=2.5mV/C     
	temp=(10*(((rc_data->ADC_DATA.ADC_TMP*2890)>>12)-760))/25+25;
	ADC_Tmp_Num +=temp;

	
	
	//电池电压处理，单位mV
	//Vsense=Vref*Adc>>12, Vref=2890mV  
	temp=((rc_data->ADC_DATA.ADC_BAT*2890)>>12)*2;
	ADC_Bat_Num+=temp;
	
	ADC_Count++;
	if(ADC_Count==10)
	{
		rc_data->TX_BAT=(ADC_Bat_Num/ADC_Count);
		rc_data->TX_TMP=(ADC_Tmp_Num/ADC_Count);
		ADC_Count=0;
		ADC_Bat_Num=0;
		ADC_Tmp_Num=0;
	}
}



/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  /* Turn LED3 on */
  BSP_LED_On(LED3);
  while(1)
  {
  }
}

#ifdef  USE_FULL_ASSERT


/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

int fputc(int ch, FILE *f)
{
	USART2->DR = ((uint8_t)ch & (uint16_t)0x01FF);
	while (!(USART2->SR & USART_SR_TXE));
	return (ch);
}



/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
