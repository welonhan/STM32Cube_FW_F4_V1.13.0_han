/**
  ******************************************************************************
  * @file    stm32f4_discovery_lcd.c
  * @author  Hanwl
  * @version 
  * @date    22-Dec-2016
  * @brief   
	******************************************************************************
 **/
 
#include "main.h"
#include "stm32f4_discovery_lcd.h"

SRAM_HandleTypeDef hsram;
FSMC_NORSRAM_TimingTypeDef SRAM_Timing;

volatile void LCD_Delay(char j);
void LCD_Rst(void);
void LCD_ColorBox(uint16_t xStart,uint16_t yStart,uint16_t xLong,uint16_t yLong,uint16_t Color);

extern TIM_HandleTypeDef    	TimHandle;
extern TIM_OC_InitTypeDef 		sConfig;

/**
  * @brief :LCD_BACKLIGHT_PWM init
  * @param :
  *			None 
  * @note  :?
  * @retval:
  *			None
  */
void LCD_BACKLIGHT_PWM_Init(void)
{
	/* Compute the prescaler value to have TIM3 counter clock equal to 8 MHz */
  uint32_t uwPrescalerValue = 0;
	uwPrescalerValue = ((168000000 /2) / 8000000) - 1;

  
  /*##-1- Configure the TIM peripheral #######################################*/ 
  /* Initialize TIMx peripheral as follow:
       + Prescaler = (SystemCoreClock/2)/8000000
       + Period = 1800  (to have an output frequency equal to 10 KHz)
       + ClockDivision = 0
       + Counter direction = Up
  */
  TimHandle.Instance = TIM2;
  
  TimHandle.Init.Prescaler     = uwPrescalerValue;
  TimHandle.Init.Period        = PERIOD_VALUE;
  TimHandle.Init.ClockDivision = 0;
  TimHandle.Init.CounterMode   = TIM_COUNTERMODE_UP;
  if(HAL_TIM_PWM_Init(&TimHandle) != HAL_OK)
  {
    /* Initialization Error */
    //Error_Handler();
		printf("lcd backlight pwm init fail\n\r");
  }
  
  /*##-2- Configure the PWM channels #########################################*/ 
  /* Common configuration for all channels */
  sConfig.OCMode     = TIM_OCMODE_PWM1;
  sConfig.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfig.OCFastMode = TIM_OCFAST_DISABLE;

  /* Set the pulse value for channel 1 */
  sConfig.Pulse = PULSE1_VALUE;  
  if(HAL_TIM_PWM_ConfigChannel(&TimHandle, &sConfig, TIM_CHANNEL_2) != HAL_OK)
  {
    /* Configuration Error */
    printf("lcd backlight pwm setting pulse1 fail\n\r");
  }
  
  /*##-3- Start PWM signals generation #######################################*/ 
  /* Start channel 1 */
  if(HAL_TIM_PWM_Start(&TimHandle, TIM_CHANNEL_2) != HAL_OK)
  {
    /* Starting Error */
    printf("lcd backlight pwm start fail\n\r");
  }	
}
/**
  * @brief :LCD_BACKLIGHT_PWM init
  * @param :
  *			None 
  * @note  :?
  * @retval:
  *			None
  */
void LCD_BACKLIGHT_PWM_50duty(void)
{
	/* Set the pulse value for channel 1 */
  sConfig.Pulse = PULSE1_VALUE;  
  if(HAL_TIM_PWM_ConfigChannel(&TimHandle, &sConfig, TIM_CHANNEL_2) != HAL_OK)
  {
    /* Configuration Error */
    printf("lcd backlight pwm setting pulse1 fail\n\r");
  }
  
  /*##-3- Start PWM signals generation #######################################*/ 
  /* Start channel 1 */
	
  if(HAL_TIM_PWM_Start(&TimHandle, TIM_CHANNEL_2) != HAL_OK)
  {
   
    printf("lcd backlight pwm start fail\n\r");
  }	
}

void LCD_BACKLIGHT_PWM_25duty(void)
{
	/* Set the pulse value for channel 1 */
  sConfig.Pulse = PULSE3_VALUE;  
  if(HAL_TIM_PWM_ConfigChannel(&TimHandle, &sConfig, TIM_CHANNEL_2) != HAL_OK)
  {
    /* Configuration Error */
    printf("lcd backlight pwm setting pulse1 fail\n\r");
  }
  
  /*##-3- Start PWM signals generation #######################################*/ 
  /* Start channel 1 */
	
  if(HAL_TIM_PWM_Start(&TimHandle, TIM_CHANNEL_2) != HAL_OK)
  {
    
    printf("lcd backlight pwm start fail\n\r");
  }	
}

/**
  * @brief :LCD GPIO,DATA interface init
  * @param :
  *			None 
  * @note  :?
  * @retval:
  *			None
  */
void LCD_Init(void)
{
	/*Configure the LCD  control GPIO */
	//LCD RESET-----------PA8
	//LCD BACKLIGHT-------PA1 TIM5_CH2
	GPIO_InitTypeDef GPIO_Init_Structure;
   
  /* Enable GPIOs clock */
  __HAL_RCC_GPIOA_CLK_ENABLE();
    
  /* Common GPIO configuration */
  GPIO_Init_Structure.Mode      = GPIO_MODE_OUTPUT_PP;
  GPIO_Init_Structure.Pull      = GPIO_PULLUP;
  GPIO_Init_Structure.Speed     = GPIO_SPEED_HIGH;
    
  /* GPIOD configuration */
	// RESET BACKLIGHT_EN												 
  GPIO_Init_Structure.Pin   = GPIO_PIN_8 ;   
  HAL_GPIO_Init(GPIOA, &GPIO_Init_Structure);	
	LCD_Rst();
			
	/*##-1- Configure the SRAM device ##########################################*/
  /* SRAM device configuration */ 
  hsram.Instance  = FSMC_NORSRAM_DEVICE;
  hsram.Extended  = FSMC_NORSRAM_EXTENDED_DEVICE;
  
  SRAM_Timing.AddressSetupTime       = 2;
  SRAM_Timing.AddressHoldTime        = 1;
  SRAM_Timing.DataSetupTime          = 5;
  SRAM_Timing.BusTurnAroundDuration  = 1;
  //SRAM_Timing.CLKDivision            = 2;
  SRAM_Timing.DataLatency            = 2;
  SRAM_Timing.AccessMode             = FSMC_ACCESS_MODE_B;
  
  hsram.Init.NSBank             = FSMC_NORSRAM_BANK1;
  hsram.Init.DataAddressMux     = FSMC_DATA_ADDRESS_MUX_DISABLE;
  hsram.Init.MemoryType         = FSMC_MEMORY_TYPE_NOR;
  hsram.Init.MemoryDataWidth    = SRAM_MEMORY_WIDTH;
  hsram.Init.BurstAccessMode    = FSMC_BURST_ACCESS_MODE_DISABLE;
  hsram.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
  hsram.Init.WrapMode           = FSMC_WRAP_MODE_DISABLE;
  hsram.Init.WaitSignalActive   = FSMC_WAIT_TIMING_BEFORE_WS;
  hsram.Init.WriteOperation     = FSMC_WRITE_OPERATION_ENABLE;
  hsram.Init.WaitSignal         = FSMC_WAIT_SIGNAL_DISABLE;
  hsram.Init.ExtendedMode       = FSMC_EXTENDED_MODE_DISABLE;
  hsram.Init.AsynchronousWait   = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
  hsram.Init.WriteBurst         = FSMC_WRITE_BURST_DISABLE;

  /* Initialize the SRAM controller */
  if(HAL_SRAM_Init(&hsram, &SRAM_Timing, &SRAM_Timing) != HAL_OK)
	{printf("LCD init fail\n\r");}
		
}

volatile void LCD_Delay(char j)
{
volatile uint16_t i;	
	while(j--)
for(i=7200;i>0;i--);
}

void LCD_Rst(void)
{			
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);			//LCD RESET=0 
	HAL_Delay(5);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);				//LCD RESET=1	
	//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);				//LCD BACKLIGHT=1
}

 void WriteComm(uint16_t CMD)
{			
	*(__IO uint16_t *) (Bank1_LCD_C) = CMD;
}

 void WriteData(uint16_t tem_data)
{			
	*(__IO uint16_t *) (Bank1_LCD_D) = tem_data;
}


/**
  * @brief :LCD init
  * @param :
  *			None 
  * @note  :?
  * @retval:
  *			None
  */
void BSP_LCD_Init(void)
{	
	LCD_Init();
	
	//ENABLE PAGE 1
	WriteComm(0xF000);WriteData(0x55);
	WriteComm(0xF001);WriteData(0xAA);
	WriteComm(0xF002);WriteData(0x52);
	WriteComm(0xF003);WriteData(0x08);
	WriteComm(0xF004);WriteData(0x01);

	//GAMMA SETING  RED
	WriteComm(0xD100);WriteData(0x00);
	WriteComm(0xD101);WriteData(0x00);
	WriteComm(0xD102);WriteData(0x1b);
	WriteComm(0xD103);WriteData(0x44);
	WriteComm(0xD104);WriteData(0x62);
	WriteComm(0xD105);WriteData(0x00);
	WriteComm(0xD106);WriteData(0x7b);
	WriteComm(0xD107);WriteData(0xa1);
	WriteComm(0xD108);WriteData(0xc0);
	WriteComm(0xD109);WriteData(0xee);
	WriteComm(0xD10A);WriteData(0x55);
	WriteComm(0xD10B);WriteData(0x10);
	WriteComm(0xD10C);WriteData(0x2c);
	WriteComm(0xD10D);WriteData(0x43);
	WriteComm(0xD10E);WriteData(0x57);
	WriteComm(0xD10F);WriteData(0x55);
	WriteComm(0xD110);WriteData(0x68);
	WriteComm(0xD111);WriteData(0x78);
	WriteComm(0xD112);WriteData(0x87);
	WriteComm(0xD113);WriteData(0x94);
	WriteComm(0xD114);WriteData(0x55);
	WriteComm(0xD115);WriteData(0xa0);
	WriteComm(0xD116);WriteData(0xac);
	WriteComm(0xD117);WriteData(0xb6);
	WriteComm(0xD118);WriteData(0xc1);
	WriteComm(0xD119);WriteData(0x55);
	WriteComm(0xD11A);WriteData(0xcb);
	WriteComm(0xD11B);WriteData(0xcd);
	WriteComm(0xD11C);WriteData(0xd6);
	WriteComm(0xD11D);WriteData(0xdf);
	WriteComm(0xD11E);WriteData(0x95);
	WriteComm(0xD11F);WriteData(0xe8);
	WriteComm(0xD120);WriteData(0xf1);
	WriteComm(0xD121);WriteData(0xfa);
	WriteComm(0xD122);WriteData(0x02);
	WriteComm(0xD123);WriteData(0xaa);
	WriteComm(0xD124);WriteData(0x0b);
	WriteComm(0xD125);WriteData(0x13);
	WriteComm(0xD126);WriteData(0x1d);
	WriteComm(0xD127);WriteData(0x26);
	WriteComm(0xD128);WriteData(0xaa);
	WriteComm(0xD129);WriteData(0x30);
	WriteComm(0xD12A);WriteData(0x3c);
	WriteComm(0xD12B);WriteData(0x4A);
	WriteComm(0xD12C);WriteData(0x63);
	WriteComm(0xD12D);WriteData(0xea);
	WriteComm(0xD12E);WriteData(0x79);
	WriteComm(0xD12F);WriteData(0xa6);
	WriteComm(0xD130);WriteData(0xd0);
	WriteComm(0xD131);WriteData(0x20);
	WriteComm(0xD132);WriteData(0x0f);
	WriteComm(0xD133);WriteData(0x8e);
	WriteComm(0xD134);WriteData(0xff);
	//GAMMA SETING GREEN
	WriteComm(0xD200);WriteData(0x00);
	WriteComm(0xD201);WriteData(0x00);
	WriteComm(0xD202);WriteData(0x1b);
	WriteComm(0xD203);WriteData(0x44);
	WriteComm(0xD204);WriteData(0x62);
	WriteComm(0xD205);WriteData(0x00);
	WriteComm(0xD206);WriteData(0x7b);
	WriteComm(0xD207);WriteData(0xa1);
	WriteComm(0xD208);WriteData(0xc0);
	WriteComm(0xD209);WriteData(0xee);
	WriteComm(0xD20A);WriteData(0x55);
	WriteComm(0xD20B);WriteData(0x10);
	WriteComm(0xD20C);WriteData(0x2c);
	WriteComm(0xD20D);WriteData(0x43);
	WriteComm(0xD20E);WriteData(0x57);
	WriteComm(0xD20F);WriteData(0x55);
	WriteComm(0xD210);WriteData(0x68);
	WriteComm(0xD211);WriteData(0x78);
	WriteComm(0xD212);WriteData(0x87);
	WriteComm(0xD213);WriteData(0x94);
	WriteComm(0xD214);WriteData(0x55);
	WriteComm(0xD215);WriteData(0xa0);
	WriteComm(0xD216);WriteData(0xac);
	WriteComm(0xD217);WriteData(0xb6);
	WriteComm(0xD218);WriteData(0xc1);
	WriteComm(0xD219);WriteData(0x55);
	WriteComm(0xD21A);WriteData(0xcb);
	WriteComm(0xD21B);WriteData(0xcd);
	WriteComm(0xD21C);WriteData(0xd6);
	WriteComm(0xD21D);WriteData(0xdf);
	WriteComm(0xD21E);WriteData(0x95);
	WriteComm(0xD21F);WriteData(0xe8);
	WriteComm(0xD220);WriteData(0xf1);
	WriteComm(0xD221);WriteData(0xfa);
	WriteComm(0xD222);WriteData(0x02);
	WriteComm(0xD223);WriteData(0xaa);
	WriteComm(0xD224);WriteData(0x0b);
	WriteComm(0xD225);WriteData(0x13);
	WriteComm(0xD226);WriteData(0x1d);
	WriteComm(0xD227);WriteData(0x26);
	WriteComm(0xD228);WriteData(0xaa);
	WriteComm(0xD229);WriteData(0x30);
	WriteComm(0xD22A);WriteData(0x3c);
	WriteComm(0xD22B);WriteData(0x4a);
	WriteComm(0xD22C);WriteData(0x63);
	WriteComm(0xD22D);WriteData(0xea);
	WriteComm(0xD22E);WriteData(0x79);
	WriteComm(0xD22F);WriteData(0xa6);
	WriteComm(0xD230);WriteData(0xd0);
	WriteComm(0xD231);WriteData(0x20);
	WriteComm(0xD232);WriteData(0x0f);
	WriteComm(0xD233);WriteData(0x8e);
	WriteComm(0xD234);WriteData(0xff);

	//GAMMA SETING BLUE
	WriteComm(0xD300);WriteData(0x00);
	WriteComm(0xD301);WriteData(0x00);
	WriteComm(0xD302);WriteData(0x1b);
	WriteComm(0xD303);WriteData(0x44);
	WriteComm(0xD304);WriteData(0x62);
	WriteComm(0xD305);WriteData(0x00);
	WriteComm(0xD306);WriteData(0x7b);
	WriteComm(0xD307);WriteData(0xa1);
	WriteComm(0xD308);WriteData(0xc0);
	WriteComm(0xD309);WriteData(0xee);
	WriteComm(0xD30A);WriteData(0x55);
	WriteComm(0xD30B);WriteData(0x10);
	WriteComm(0xD30C);WriteData(0x2c);
	WriteComm(0xD30D);WriteData(0x43);
	WriteComm(0xD30E);WriteData(0x57);
	WriteComm(0xD30F);WriteData(0x55);
	WriteComm(0xD310);WriteData(0x68);
	WriteComm(0xD311);WriteData(0x78);
	WriteComm(0xD312);WriteData(0x87);
	WriteComm(0xD313);WriteData(0x94);
	WriteComm(0xD314);WriteData(0x55);
	WriteComm(0xD315);WriteData(0xa0);
	WriteComm(0xD316);WriteData(0xac);
	WriteComm(0xD317);WriteData(0xb6);
	WriteComm(0xD318);WriteData(0xc1);
	WriteComm(0xD319);WriteData(0x55);
	WriteComm(0xD31A);WriteData(0xcb);
	WriteComm(0xD31B);WriteData(0xcd);
	WriteComm(0xD31C);WriteData(0xd6);
	WriteComm(0xD31D);WriteData(0xdf);
	WriteComm(0xD31E);WriteData(0x95);
	WriteComm(0xD31F);WriteData(0xe8);
	WriteComm(0xD320);WriteData(0xf1);
	WriteComm(0xD321);WriteData(0xfa);
	WriteComm(0xD322);WriteData(0x02);
	WriteComm(0xD323);WriteData(0xaa);
	WriteComm(0xD324);WriteData(0x0b);
	WriteComm(0xD325);WriteData(0x13);
	WriteComm(0xD326);WriteData(0x1d);
	WriteComm(0xD327);WriteData(0x26);
	WriteComm(0xD328);WriteData(0xaa);
	WriteComm(0xD329);WriteData(0x30);
	WriteComm(0xD32A);WriteData(0x3c);
	WriteComm(0xD32B);WriteData(0x4A);
	WriteComm(0xD32C);WriteData(0x63);
	WriteComm(0xD32D);WriteData(0xea);
	WriteComm(0xD32E);WriteData(0x79);
	WriteComm(0xD32F);WriteData(0xa6);
	WriteComm(0xD330);WriteData(0xd0);
	WriteComm(0xD331);WriteData(0x20);
	WriteComm(0xD332);WriteData(0x0f);
	WriteComm(0xD333);WriteData(0x8e);
	WriteComm(0xD334);WriteData(0xff);

	//GAMMA SETING  RED
	WriteComm(0xD400);WriteData(0x00);
	WriteComm(0xD401);WriteData(0x00);
	WriteComm(0xD402);WriteData(0x1b);
	WriteComm(0xD403);WriteData(0x44);
	WriteComm(0xD404);WriteData(0x62);
	WriteComm(0xD405);WriteData(0x00);
	WriteComm(0xD406);WriteData(0x7b);
	WriteComm(0xD407);WriteData(0xa1);
	WriteComm(0xD408);WriteData(0xc0);
	WriteComm(0xD409);WriteData(0xee);
	WriteComm(0xD40A);WriteData(0x55);
	WriteComm(0xD40B);WriteData(0x10);
	WriteComm(0xD40C);WriteData(0x2c);
	WriteComm(0xD40D);WriteData(0x43);
	WriteComm(0xD40E);WriteData(0x57);
	WriteComm(0xD40F);WriteData(0x55);
	WriteComm(0xD410);WriteData(0x68);
	WriteComm(0xD411);WriteData(0x78);
	WriteComm(0xD412);WriteData(0x87);
	WriteComm(0xD413);WriteData(0x94);
	WriteComm(0xD414);WriteData(0x55);
	WriteComm(0xD415);WriteData(0xa0);
	WriteComm(0xD416);WriteData(0xac);
	WriteComm(0xD417);WriteData(0xb6);
	WriteComm(0xD418);WriteData(0xc1);
	WriteComm(0xD419);WriteData(0x55);
	WriteComm(0xD41A);WriteData(0xcb);
	WriteComm(0xD41B);WriteData(0xcd);
	WriteComm(0xD41C);WriteData(0xd6);
	WriteComm(0xD41D);WriteData(0xdf);
	WriteComm(0xD41E);WriteData(0x95);
	WriteComm(0xD41F);WriteData(0xe8);
	WriteComm(0xD420);WriteData(0xf1);
	WriteComm(0xD421);WriteData(0xfa);
	WriteComm(0xD422);WriteData(0x02);
	WriteComm(0xD423);WriteData(0xaa);
	WriteComm(0xD424);WriteData(0x0b);
	WriteComm(0xD425);WriteData(0x13);
	WriteComm(0xD426);WriteData(0x1d);
	WriteComm(0xD427);WriteData(0x26);
	WriteComm(0xD428);WriteData(0xaa);
	WriteComm(0xD429);WriteData(0x30);
	WriteComm(0xD42A);WriteData(0x3c);
	WriteComm(0xD42B);WriteData(0x4A);
	WriteComm(0xD42C);WriteData(0x63);
	WriteComm(0xD42D);WriteData(0xea);
	WriteComm(0xD42E);WriteData(0x79);
	WriteComm(0xD42F);WriteData(0xa6);
	WriteComm(0xD430);WriteData(0xd0);
	WriteComm(0xD431);WriteData(0x20);
	WriteComm(0xD432);WriteData(0x0f);
	WriteComm(0xD433);WriteData(0x8e);
	WriteComm(0xD434);WriteData(0xff);

	//GAMMA SETING GREEN
	WriteComm(0xD500);WriteData(0x00);
	WriteComm(0xD501);WriteData(0x00);
	WriteComm(0xD502);WriteData(0x1b);
	WriteComm(0xD503);WriteData(0x44);
	WriteComm(0xD504);WriteData(0x62);
	WriteComm(0xD505);WriteData(0x00);
	WriteComm(0xD506);WriteData(0x7b);
	WriteComm(0xD507);WriteData(0xa1);
	WriteComm(0xD508);WriteData(0xc0);
	WriteComm(0xD509);WriteData(0xee);
	WriteComm(0xD50A);WriteData(0x55);
	WriteComm(0xD50B);WriteData(0x10);
	WriteComm(0xD50C);WriteData(0x2c);
	WriteComm(0xD50D);WriteData(0x43);
	WriteComm(0xD50E);WriteData(0x57);
	WriteComm(0xD50F);WriteData(0x55);
	WriteComm(0xD510);WriteData(0x68);
	WriteComm(0xD511);WriteData(0x78);
	WriteComm(0xD512);WriteData(0x87);
	WriteComm(0xD513);WriteData(0x94);
	WriteComm(0xD514);WriteData(0x55);
	WriteComm(0xD515);WriteData(0xa0);
	WriteComm(0xD516);WriteData(0xac);
	WriteComm(0xD517);WriteData(0xb6);
	WriteComm(0xD518);WriteData(0xc1);
	WriteComm(0xD519);WriteData(0x55);
	WriteComm(0xD51A);WriteData(0xcb);
	WriteComm(0xD51B);WriteData(0xcd);
	WriteComm(0xD51C);WriteData(0xd6);
	WriteComm(0xD51D);WriteData(0xdf);
	WriteComm(0xD51E);WriteData(0x95);
	WriteComm(0xD51F);WriteData(0xe8);
	WriteComm(0xD520);WriteData(0xf1);
	WriteComm(0xD521);WriteData(0xfa);
	WriteComm(0xD522);WriteData(0x02);
	WriteComm(0xD523);WriteData(0xaa);
	WriteComm(0xD524);WriteData(0x0b);
	WriteComm(0xD525);WriteData(0x13);
	WriteComm(0xD526);WriteData(0x1d);
	WriteComm(0xD527);WriteData(0x26);
	WriteComm(0xD528);WriteData(0xaa);
	WriteComm(0xD529);WriteData(0x30);
	WriteComm(0xD52A);WriteData(0x3c);
	WriteComm(0xD52B);WriteData(0x4a);
	WriteComm(0xD52C);WriteData(0x63);
	WriteComm(0xD52D);WriteData(0xea);
	WriteComm(0xD52E);WriteData(0x79);
	WriteComm(0xD52F);WriteData(0xa6);
	WriteComm(0xD530);WriteData(0xd0);
	WriteComm(0xD531);WriteData(0x20);
	WriteComm(0xD532);WriteData(0x0f);
	WriteComm(0xD533);WriteData(0x8e);
	WriteComm(0xD534);WriteData(0xff);

	//GAMMA SETING BLUE
	WriteComm(0xD600);WriteData(0x00);
	WriteComm(0xD601);WriteData(0x00);
	WriteComm(0xD602);WriteData(0x1b);
	WriteComm(0xD603);WriteData(0x44);
	WriteComm(0xD604);WriteData(0x62);
	WriteComm(0xD605);WriteData(0x00);
	WriteComm(0xD606);WriteData(0x7b);
	WriteComm(0xD607);WriteData(0xa1);
	WriteComm(0xD608);WriteData(0xc0);
	WriteComm(0xD609);WriteData(0xee);
	WriteComm(0xD60A);WriteData(0x55);
	WriteComm(0xD60B);WriteData(0x10);
	WriteComm(0xD60C);WriteData(0x2c);
	WriteComm(0xD60D);WriteData(0x43);
	WriteComm(0xD60E);WriteData(0x57);
	WriteComm(0xD60F);WriteData(0x55);
	WriteComm(0xD610);WriteData(0x68);
	WriteComm(0xD611);WriteData(0x78);
	WriteComm(0xD612);WriteData(0x87);
	WriteComm(0xD613);WriteData(0x94);
	WriteComm(0xD614);WriteData(0x55);
	WriteComm(0xD615);WriteData(0xa0);
	WriteComm(0xD616);WriteData(0xac);
	WriteComm(0xD617);WriteData(0xb6);
	WriteComm(0xD618);WriteData(0xc1);
	WriteComm(0xD619);WriteData(0x55);
	WriteComm(0xD61A);WriteData(0xcb);
	WriteComm(0xD61B);WriteData(0xcd);
	WriteComm(0xD61C);WriteData(0xd6);
	WriteComm(0xD61D);WriteData(0xdf);
	WriteComm(0xD61E);WriteData(0x95);
	WriteComm(0xD61F);WriteData(0xe8);
	WriteComm(0xD620);WriteData(0xf1);
	WriteComm(0xD621);WriteData(0xfa);
	WriteComm(0xD622);WriteData(0x02);
	WriteComm(0xD623);WriteData(0xaa);
	WriteComm(0xD624);WriteData(0x0b);
	WriteComm(0xD625);WriteData(0x13);
	WriteComm(0xD626);WriteData(0x1d);
	WriteComm(0xD627);WriteData(0x26);
	WriteComm(0xD628);WriteData(0xaa);
	WriteComm(0xD629);WriteData(0x30);
	WriteComm(0xD62A);WriteData(0x3c);
	WriteComm(0xD62B);WriteData(0x4A);
	WriteComm(0xD62C);WriteData(0x63);
	WriteComm(0xD62D);WriteData(0xea);
	WriteComm(0xD62E);WriteData(0x79);
	WriteComm(0xD62F);WriteData(0xa6);
	WriteComm(0xD630);WriteData(0xd0);
	WriteComm(0xD631);WriteData(0x20);
	WriteComm(0xD632);WriteData(0x0f);
	WriteComm(0xD633);WriteData(0x8e);
	WriteComm(0xD634);WriteData(0xff);

	//AVDD VOLTAGE SETTING
	WriteComm(0xB000);WriteData(0x05);
	WriteComm(0xB001);WriteData(0x05);
	WriteComm(0xB002);WriteData(0x05);
	//AVEE VOLTAGE SETTING
	WriteComm(0xB100);WriteData(0x05);
	WriteComm(0xB101);WriteData(0x05);
	WriteComm(0xB102);WriteData(0x05);

	//AVDD Boosting
	WriteComm(0xB600);WriteData(0x34);
	WriteComm(0xB601);WriteData(0x34);
	WriteComm(0xB603);WriteData(0x34);
	//AVEE Boosting
	WriteComm(0xB700);WriteData(0x24);
	WriteComm(0xB701);WriteData(0x24);
	WriteComm(0xB702);WriteData(0x24);
	//VCL Boosting
	WriteComm(0xB800);WriteData(0x24);
	WriteComm(0xB801);WriteData(0x24);
	WriteComm(0xB802);WriteData(0x24);
	//VGLX VOLTAGE SETTING
	WriteComm(0xBA00);WriteData(0x14);
	WriteComm(0xBA01);WriteData(0x14);
	WriteComm(0xBA02);WriteData(0x14);
	//VCL Boosting
	WriteComm(0xB900);WriteData(0x24);
	WriteComm(0xB901);WriteData(0x24);
	WriteComm(0xB902);WriteData(0x24);
	//Gamma Voltage
	WriteComm(0xBc00);WriteData(0x00);
	WriteComm(0xBc01);WriteData(0xa0);//vgmp=5.0
	WriteComm(0xBc02);WriteData(0x00);
	WriteComm(0xBd00);WriteData(0x00);
	WriteComm(0xBd01);WriteData(0xa0);//vgmn=5.0
	WriteComm(0xBd02);WriteData(0x00);
	//VCOM Setting
	WriteComm(0xBe01);WriteData(0x3d);//3
	//ENABLE PAGE 0
	WriteComm(0xF000);WriteData(0x55);
	WriteComm(0xF001);WriteData(0xAA);
	WriteComm(0xF002);WriteData(0x52);
	WriteComm(0xF003);WriteData(0x08);
	WriteComm(0xF004);WriteData(0x00);
	//Vivid Color Function Control
	WriteComm(0xB400);WriteData(0x10);
	//Z-INVERSION
	WriteComm(0xBC00);WriteData(0x05);
	WriteComm(0xBC01);WriteData(0x05);
	WriteComm(0xBC02);WriteData(0x05);

	//*************** add on 20111021**********************//
	WriteComm(0xB700);WriteData(0x22);//GATE EQ CONTROL
	WriteComm(0xB701);WriteData(0x22);//GATE EQ CONTROL

	WriteComm(0xC80B);WriteData(0x2A);//DISPLAY TIMING CONTROL
	WriteComm(0xC80C);WriteData(0x2A);//DISPLAY TIMING CONTROL
	WriteComm(0xC80F);WriteData(0x2A);//DISPLAY TIMING CONTROL
	WriteComm(0xC810);WriteData(0x2A);//DISPLAY TIMING CONTROL
	//*************** add on 20111021**********************//
	//PWM_ENH_OE =1
	WriteComm(0xd000);WriteData(0x01);
	//DM_SEL =1
	WriteComm(0xb300);WriteData(0x10);
	//VBPDA=07h
	WriteComm(0xBd02);WriteData(0x07);
	//VBPDb=07h
	WriteComm(0xBe02);WriteData(0x07);
	//VBPDc=07h
	WriteComm(0xBf02);WriteData(0x07);
	//ENABLE PAGE 2
	WriteComm(0xF000);WriteData(0x55);
	WriteComm(0xF001);WriteData(0xAA);
	WriteComm(0xF002);WriteData(0x52);
	WriteComm(0xF003);WriteData(0x08);
	WriteComm(0xF004);WriteData(0x02);
	//SDREG0 =0
	WriteComm(0xc301);WriteData(0xa9);
	//DS=14
	WriteComm(0xfe01);WriteData(0x94);
	//OSC =60h
	WriteComm(0xf600);WriteData(0x60);
	//TE ON
	WriteComm(0x3500);WriteData(0x00);
	//SLEEP OUT 
	WriteComm(0x1100);
	LCD_Delay(120);
	//DISPLY ON
	WriteComm(0x2900);
	LCD_Delay(100);
	//Lcd_Light_ON;
	
	WriteComm(0x3A00); WriteData(0x55);
//	WriteComm(0x3600); WriteData(0xA8);
	WriteComm(0x3600); WriteData(0x68);
//	LCD_ColorBox(0,0,800,480,0x07E0);
	
// 	LCD_Fill_Pic(80,160,320,480, gImage_MM_T035);
// 	BlockWrite(0,0,799,479);
}

/******************************************
函数名：Lcd写命令函数
功能：向Lcd指定位置写入应有命令或数据
入口参数：Index 要寻址的寄存器地址
          ConfigTemp 写入的数据或命令值
******************************************/
void LCD_WR_REG(uint16_t Index,uint16_t CongfigTemp)
{
	*(__IO uint16_t *) (Bank1_LCD_C) = Index;	
	*(__IO uint16_t *) (Bank1_LCD_D) = CongfigTemp;
}

void BlockWrite(unsigned int Xstart,unsigned int Xend,unsigned int Ystart,unsigned int Yend) 
{
	//OTM8009

	WriteComm(0x2a00);   
	WriteData(Xstart>>8);
	WriteComm(0x2a01); 
	WriteData(Xstart);
	WriteComm(0x2a02); 
	WriteData(Xend>>8);
	WriteComm(0x2a03); 
	WriteData(Xend);

	WriteComm(0x2b00);   
	WriteData(Ystart>>8);
	WriteComm(0x2b01); 
	WriteData(Ystart);
	WriteComm(0x2b02); 
	WriteData(Yend>>8);
	WriteComm(0x2b03); 
	WriteData(Yend);

	WriteComm(0x2c00);
}


uint16_t BSP_LCD_GetPoint(uint16_t x,uint16_t y)
{
 	WriteComm(0x2a00);   
 	WriteData(x>>8);
 	WriteComm(0x2a01); 
 	WriteData(x);

 	WriteComm(0x2b00);   
 	WriteData(y>>8);
 	WriteComm(0x2b01); 
 	WriteData(y);

	WriteComm(0x2e00);
	
	x = *(__IO uint16_t *) (Bank1_LCD_D);

	x = *(__IO uint16_t *) (Bank1_LCD_D);

	y = *(__IO uint16_t *) (Bank1_LCD_D);
// 	printf("RIN=%04x\r\n",b);

	return (x&0xf800)|((x&0x00fc)<<3)|(y>>11);
}

/**********************************************
函数名：Lcd块填充
功能：选定Lcd上指定的矩形区域

入口参数：xStart x方向的起始点
          ySrart y方向的终止点
          xEnd 
          yEnd  
返回值：无
***********************************************/
void BSP_LCD_FillRec(uint16_t xStart,uint16_t yStart,uint16_t xEnd,uint16_t yEnd,uint16_t Color)
{
	uint32_t temp;

	BlockWrite(xStart,xEnd,yStart,yEnd);
	for (temp=0; temp<(xEnd-xStart)*(yEnd-yStart); temp++)
	{
		*(__IO uint16_t *) (Bank1_LCD_D) = Color;
	}
}
/**********************************************
函数名：Lcd块选函数
功能：选定Lcd上指定的矩形区域

注意：xStart和 yStart随着屏幕的旋转而改变，位置是矩形框的四个角

入口参数：xStart x方向的起始点
          ySrart y方向的终止点
          xLong 要选定矩形的x方向长度
          yLong  要选定矩形的y方向长度
返回值：无
***********************************************/
void LCD_ColorBox(uint16_t xStart,uint16_t yStart,uint16_t xLong,uint16_t yLong,uint16_t Color)
{
	uint32_t temp;

	BlockWrite(xStart,xStart+xLong-1,yStart,yStart+yLong-1);
	for (temp=0; temp<xLong*yLong; temp++)
	{
		*(__IO uint16_t *) (Bank1_LCD_D) = Color;
	}
}

/******************************************
函数名：Lcd图像填充
功能：向Lcd指定位置填充图像
入口参数：
******************************************/
void BSP_LCD_FillPic(uint16_t x, uint16_t y,uint16_t pic_H, uint16_t pic_V, uint16_t* pic)
{
  unsigned long i;
//	unsigned int j;

// 	WriteComm(0x3600); //Set_address_mode
// 	WriteData(0x00); //横屏，从左下角开始，从左到右，从下到上
	BlockWrite(x,x+pic_H-1,y,y+pic_V-1);
	for (i = 0; i < pic_H*pic_V; i++)
	{
		*(__IO uint16_t *) (Bank1_LCD_D) = pic[i];
	}
// 	WriteComm(0x3600); //Set_address_mode
// 	WriteData(0xA0);
}


/******************************************
函数名：Lcd图像填充100*100
功能：向Lcd指定位置填充图像
入口参数：Index 要寻址的寄存器地址
          ConfigTemp 写入的数据或命令值
******************************************/
void LCD_Fill_Pic(uint16_t x, uint16_t y,uint16_t pic_H, uint16_t pic_V, const unsigned char* pic)
{
  unsigned long i;
	unsigned int j;

// 	WriteComm(0x3600); //Set_address_mode
// 	WriteData(0x00); //横屏，从左下角开始，从左到右，从下到上
	BlockWrite(x,x+pic_H-1,y,y+pic_V-1);
	for (i = 0; i < pic_H*pic_V*2; i+=2)
	{
		j=pic[i];
		j=j<<8;
		j=j+pic[i+1];
		*(__IO uint16_t *) (Bank1_LCD_D) = j;
	}
// 	WriteComm(0x3600); //Set_address_mode
// 	WriteData(0xA0);
}

//在指定座标上打一个点
void BSP_LCD_DrawPixel(uint16_t x, uint16_t y, int Color)
{
	BlockWrite(x,x,y,y);

	*(__IO uint16_t *) (Bank1_LCD_D) = Color;
}


#if 0
/*
 * 函数名：LCD_GPIO_Config
 * 描述  ：根据FSMC配置LCD的I/O
 * 输入  ：无
 * 输出  ：无
 * 调用  ：内部调用        
 */
static void LCD_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    /* Enable the FSMC Clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
    
    /* config lcd gpio clock base on FSMC */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE , ENABLE);
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    
    /* config tft Reset gpio */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;		
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    /* config tft BL gpio 背光开关控制引脚*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 ; 	 
    GPIO_Init(GPIOA, &GPIO_InitStructure);  		   
    
    /* config tft data lines base on FSMC
	 * data lines,FSMC-D0~D15: PD 14 15 0 1,PE 7 8 9 10 11 12 13 14 15,PD 8 9 10
	 */	
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_8 | GPIO_Pin_9 | 
                                  GPIO_Pin_10 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | 
                                  GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | 
                                  GPIO_Pin_15;
    GPIO_Init(GPIOE, &GPIO_InitStructure); 
    
    /* config tft control lines base on FSMC
	 * PD4-FSMC_NOE  :LCD-RD
   * PD5-FSMC_NWE  :LCD-WR
	 * PD7-FSMC_NE1  :LCD-CS
   * PD11-FSMC_A16 :LCD-DC
	 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4; 
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; 
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7; 
    GPIO_Init(GPIOD, &GPIO_InitStructure);  
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 ; 
    GPIO_Init(GPIOD, &GPIO_InitStructure);  
    
    /* tft control gpio init */	 
    GPIO_SetBits(GPIOD, GPIO_Pin_13);		 // Reset				
    GPIO_SetBits(GPIOD, GPIO_Pin_4);		 // RD = 1  
    GPIO_SetBits(GPIOD, GPIO_Pin_5);		 // WR = 1 
		GPIO_SetBits(GPIOD, GPIO_Pin_11);		 // RS
    GPIO_SetBits(GPIOD, GPIO_Pin_7);		 //	CS = 1

		GPIO_SetBits(GPIOA, GPIO_Pin_1);	 	 //	BL = 1 
		
		
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;	//按键 OK
    GPIO_Init(GPIOA, &GPIO_InitStructure);
																//  right         up           left        down
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_1 | GPIO_Pin_5 | GPIO_Pin_6;		
    GPIO_Init(GPIOE, &GPIO_InitStructure);

}

/*
 * 函数名：LCD_FSMC_Config
 * 描述  ：LCD  FSMC 模式配置
 * 输入  ：无
 * 输出  ：无
 * 调用  ：内部调用        
 */

static void LCD_FSMC_Config(void)
{
    FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
    FSMC_NORSRAMTimingInitTypeDef  p; 
    
    
    p.FSMC_AddressSetupTime = 0x02;	 //地址建立时间
    p.FSMC_AddressHoldTime = 0x00;	 //地址保持时间
    p.FSMC_DataSetupTime = 0x05;		 //数据建立时间
    p.FSMC_BusTurnAroundDuration = 0x00;
    p.FSMC_CLKDivision = 0x00;
    p.FSMC_DataLatency = 0x00;

    p.FSMC_AccessMode = FSMC_AccessMode_B;	 // 一般使用模式B来控制LCD
    
    FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM1;
    FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
    FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_NOR;
    FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
    FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
    FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
    FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
    FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
    FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &p;
    FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &p; 
    
    FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure); 
    
    /* Enable FSMC Bank1_SRAM Bank */
    FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);  
}
#endif

