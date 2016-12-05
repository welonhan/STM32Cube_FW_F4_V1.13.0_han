/**
 * @author  Tilen Majerle
 * @email   tilen@majerle.eu
 * @website http://stm32f4-discovery.com
 * @link    http://stm32f4-discovery.com/2014/04/library-05-spi-for-stm32f4xx/
 * @version v2.0
 * @ide     Keil uVision
 * @license GNU GPL v3
 * @brief   SPI library for STM32F4xx
 *	
@verbatim
   ----------------------------------------------------------------------
    Copyright (C) Tilen Majerle, 2015
    
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.
     
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
@endverbatim
 */
#ifndef TM_SPI_H
#define TM_SPI_H 200

/* C++ detection */
#ifdef __cplusplus
extern "C" {
#endif


#include "stm32f4xx.h"

/**
 * @defgroup TM_SPI_Typedefs
 * @brief    Library Typedefs
 * @{
 */

/**
 * @brief  SPI modes selection
 */
typedef enum {
	TM_SPI_Mode_0, /*!< Clock polarity low, clock phase 1st edge */
	TM_SPI_Mode_1, /*!< Clock polarity low, clock phase 2nd edge */
	TM_SPI_Mode_2, /*!< Clock polarity high, clock phase 1st edge */
	TM_SPI_Mode_3  /*!< Clock polarity high, clock phase 2nd edge */
} TM_SPI_Mode_t;

/**
 * @brief  SPI PinsPack enumeration to select pins combination for SPI
 */
typedef enum {
	TM_SPI_PinsPack_1,       /*!< Select PinsPack1 from Pinout table for specific SPI */
	TM_SPI_PinsPack_2,       /*!< Select PinsPack2 from Pinout table for specific SPI */
	TM_SPI_PinsPack_3,       /*!< Select PinsPack3 from Pinout table for specific SPI */
	TM_SPI_PinsPack_Custom   /*!< Select custom pins for specific SPI, callback will be called, look @ref TM_SPI_InitCustomPinsCallback */
} TM_SPI_PinsPack_t;

/**
 * @brief  Daza size enumeration
 */
typedef enum {
	TM_SPI_DataSize_8b, /*!< SPI in 8-bits mode */
	TM_SPI_DataSize_16b /*!< SPI in 16-bits mode */        
} TM_SPI_DataSize_t;

/**
 * @}
 */
 
 /**
 * @defgroup TM_SPI_Macros
 * @brief    Library defines
 * @{
 */

//#define SPIx (SPI1)
/**
 * @brief  Check SPI busy status
 */
#define SPI_IS_BUSY(SPIx) (((SPIx)->SR & (SPI_SR_TXE | SPI_SR_RXNE)) == 0 || ((SPIx)->SR & SPI_SR_BSY))

/**
 * @brief  SPI wait till end
 */
#define SPI_WAIT(SPIx)            while (SPI_IS_BUSY(SPIx))

/**
 * @brief  Checks if SPI is enabled
 */
#define SPI_CHECK_ENABLED(SPIx)   if (!((SPIx)->CR1 & SPI_CR1_SPE)) {return;}

/**
 * @brief  Checks if SPI is enabled and returns value from function if not 
 */
#define SPI_CHECK_ENABLED_RESP(SPIx, val)   if (!((SPIx)->CR1 & SPI_CR1_SPE)) {return (val);}

uint8_t TM_SPI_Send(SPI_TypeDef* SPIx, uint8_t data);
/**
 * @brief  Sends and receives multiple bytes over SPIx
 * @param  *SPIx: Pointer to SPIx peripheral you will use, where x is between 1 to 6
 * @param  *dataOut: Pointer to array with data to send over SPI
 * @param  *dataIn: Pointer to array to to save incoming data
 * @param  count: Number of bytes to send/receive over SPI
 * @retval None
 */
void TM_SPI_SendMulti(SPI_TypeDef* SPIx, uint8_t* dataOut, uint8_t* dataIn, uint32_t count);

/**
 * @brief  Writes multiple bytes over SPI
 * @param  *SPIx: Pointer to SPIx peripheral you will use, where x is between 1 to 6
 * @param  *dataOut: Pointer to array with data to send over SPI
 * @param  count: Number of elements to send over SPI
 * @retval None
 */
void TM_SPI_WriteMulti(SPI_TypeDef* SPIx, uint8_t* dataOut, uint32_t count);

/**
 * @brief  Receives multiple data bytes over SPI
 * @note   Selected SPI must be set in 16-bit mode
 * @param  *SPIx: Pointer to SPIx peripheral you will use, where x is between 1 to 6
 * @param  *dataIn: Pointer to 8-bit array to save data into
 * @param  dummy: Dummy byte  to be sent over SPI, to receive data back. In most cases 0x00 or 0xFF
 * @param  count: Number of bytes you want read from device
 * @retval None
 */
void TM_SPI_ReadMulti(SPI_TypeDef* SPIx, uint8_t *dataIn, uint8_t dummy, uint32_t count);

#if 0
/**
 * @brief  Sends single byte over SPI
 * @note   Selected SPI must be set in 16-bit mode
 * @param  *SPIx: Pointer to SPIx peripheral you will use, where x is between 1 to 6
 * @param  data: 16-bit data size to send over SPI
 * @retval Received 16-bit value from slave device
 */
static __INLINE uint16_t TM_SPI_Send16(SPI_TypeDef* SPIx, uint8_t data) {
	/* Check if SPI is enabled */
	SPI_CHECK_ENABLED_RESP(SPIx, 0);
	
	/* Wait for previous transmissions to complete if DMA TX enabled for SPI */
	SPI_WAIT(SPIx);
	
	/* Fill output buffer with data */
	SPIx->DR = data;
	
	/* Wait for transmission to complete */
	SPI_WAIT(SPIx);
	
	/* Return data from buffer */
	return SPIx->DR;
}

/**
 * @brief  Sends and receives multiple bytes over SPIx in 16-bit SPI mode
 * @note   Selected SPI must be set in 16-bit mode
 * @param  *SPIx: Pointer to SPIx peripheral you will use, where x is between 1 to 6
 * @param  *dataOut: Pointer to array with data to send over SPI
 * @param  *dataIn: Pointer to array to to save incoming data
 * @param  count: Number of 16-bit values to send/receive over SPI
 * @retval None
 */
void TM_SPI_SendMulti16(SPI_TypeDef* SPIx, uint16_t* dataOut, uint16_t* dataIn, uint32_t count);

/**
 * @brief  Writes multiple data via SPI in 16-bit SPI mode
 * @note   Selected SPI must be set in 16-bit mode
 * @param  *SPIx: Pointer to SPIx peripheral you will use, where x is between 1 to 6
 * @param  *dataOut: Pointer to 16-bit array with data to send over SPI
 * @param  count: Number of elements to send over SPI
 * @retval None
 */
void TM_SPI_WriteMulti16(SPI_TypeDef* SPIx, uint16_t* dataOut, uint32_t count);

/**
 * @brief  Receives multiple data bytes over SPI in 16-bit SPI mode
 * @note   Selected SPI must be set in 16-bit mode
 * @param  *SPIx: Pointer to SPIx peripheral you will use, where x is between 1 to 6
 * @param  *dataIn: Pointer to 16-bit array to save data into
 * @param  dummy: Dummy 16-bit value to be sent over SPI, to receive data back. In most cases 0x00 or 0xFF
 * @param  count: Number of 16-bit values you want read from device
 * @retval None
 */
void TM_SPI_ReadMulti16(SPI_TypeDef* SPIx, uint16_t* dataIn, uint16_t dummy, uint32_t count);

/**
 * @brief  Init custom SPI pins for your SPIx. This is callback function and will be called from my library if needed.
 * @note   When you call TM_SPI_Init() function, and if you pass TM_SPI_PinsPack_Custom to function,
 *         then this function will be called where you can initialize custom pins for SPI peripheral
 *
 * @note   You have to initialize MOSI, MISO and SCK pin
 *
 * @param  *SPIx: Pointer to SPIx peripheral for which you have to set your custom pin settings
 * @param  AlternateFunction: Alternate function which should be used for GPIO initialization
 * @retval None
 * @note   With __weak parameter to prevent link errors if not defined by user
 */
void TM_SPI_InitCustomPinsCallback(SPI_TypeDef* SPIx, uint16_t AlternateFunction);
#endif
/**
 * @}
 */
 
/**
 * @}
 */
 
/**
 * @}
 */

/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif

