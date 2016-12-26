/**
  ******************************************************************************
  * @file    drv_adc.c
  * @author  Hanwl
  * @version 
  * @date    22-Dec-2016
  * @brief   
	******************************************************************************
 **/

/* Includes ------------------------------------------------------------------*/
#include "drv_adc.h"

extern 	uint32_t 	*DMA_ADC_MEM_ADDR;
extern 	ADC_HandleTypeDef    AdcHandle;
/**
  * @brief :BSP_ADC init
  * @param :
  *			@ADC_DMA_MemAddr: ADC 
  * @note  :?
  * @retval:
  *			None
  */
void BSP_ADC_Init(void)
{ 
  ADC_ChannelConfTypeDef 	sConfig;
  //ADC_HandleTypeDef    		AdcHandle;
	ADC_Common_TypeDef			AdcCommon;
    
  /*##-1- Configure the ADC peripheral #######################################*/
  AdcHandle.Instance = ADCx;
  
  AdcHandle.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV4;				//21M, Fadc max:36M sysclk:168M, APH2 prescale:2  ADC prescal:4
  AdcHandle.Init.Resolution = ADC_RESOLUTION_12B;
  AdcHandle.Init.ScanConvMode = ENABLE;
  AdcHandle.Init.ContinuousConvMode = ENABLE;
  AdcHandle.Init.DiscontinuousConvMode = DISABLE;
  AdcHandle.Init.NbrOfDiscConversion = 0;
  AdcHandle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  AdcHandle.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T1_CC1;
  AdcHandle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	AdcHandle.Init.NbrOfConversion = 4;															
  AdcHandle.Init.DMAContinuousRequests = ENABLE;
  AdcHandle.Init.EOCSelection = DISABLE;
	
	AdcHandle.State=HAL_ADC_STATE_RESET;
      
  if(HAL_ADC_Init(&AdcHandle) != HAL_OK)
  {
    /* Initialization Error */
    printf("adc init error\n\r"); 
  }
  
	AdcCommon.CCR |=ADC_CCR_TSVREFE;																		//Wakeup temperature sensor from power down mode
  
	/*##-2- Configure ADC regular channel ######################################*/
  /* Note: Considering IT occurring after each number of size of              */
  /*       "uhADCxConvertedValue"  ADC conversions (IT by DMA end             */
  /*       of transfer), select sampling time and ADC clock with sufficient   */
  /*       duration to not create an overhead situation in IRQHandler.        */  
  
	//config steering channel
	sConfig.Channel = ADCx_STR_CHANNEL;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES;
  sConfig.Offset = 0;
  
  if(HAL_ADC_ConfigChannel(&AdcHandle, &sConfig) != HAL_OK)
  {
    /* Channel Configuration Error */
    printf("adc channel config STR init error\n\r");
  }
	
	//config throttle lever channel
	sConfig.Channel = ADCx_THRT_CHANNEL;
  sConfig.Rank = 2;
  sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES;
  sConfig.Offset = 0;
  
  if(HAL_ADC_ConfigChannel(&AdcHandle, &sConfig) != HAL_OK)
  {
    /* Channel Configuration Error */
    printf("adc channel config THRT init error\n\r");
  }
	
	//config battery channel
	sConfig.Channel = ADCx_BAT_CHANNEL;
  sConfig.Rank = 3;
  sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES;
  sConfig.Offset = 0;
  
	
  if(HAL_ADC_ConfigChannel(&AdcHandle, &sConfig) != HAL_OK)
  {
    /* Channel Configuration Error */
    printf("adc channel config BAT init error\n\r");
  }
	
	//config temperature sensor channel
	sConfig.Channel = ADCx_TMP_CHANNEL;
  sConfig.Rank = 4;
  sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES;
  sConfig.Offset = 0;
  
  if(HAL_ADC_ConfigChannel(&AdcHandle, &sConfig) != HAL_OK)
  {
    /* Channel Configuration Error */
    printf("adc channel config TMP init error\n\r");
  }
	
  /*##-3- Start the conversion process and enable interrupt ##################*/
  /* Note: Considering IT occurring after each number of ADC conversions      */
  /*       (IT by DMA end of transfer), select sampling time and ADC clock    */
  /*       with sufficient duration to not create an overhead situation in    */
  /*        IRQHandler. */ 

  if(HAL_ADC_Start_DMA(&AdcHandle, DMA_ADC_MEM_ADDR, 4) != HAL_OK)
 	{
    /* Start Conversation Error */
    printf("ADC DMA start error\n\r");
  }

}


