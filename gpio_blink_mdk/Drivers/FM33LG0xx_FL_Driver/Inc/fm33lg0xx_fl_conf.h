 /**    
  *******************************************************************************************************
  * @file    fm33lg0xx_fl_conf.h
  * @author  FMSH Application Team
  * @brief   Header file of FL Driver Library Configurations
  *******************************************************************************************************
  * @attention
  *
  * Copyright (c) 2022, SHANGHAI FUDAN MICROELECTRONICS GROUP CO., LTD.(FUDAN MICROELECTRONICS./ FUDAN MICRO.)    
  * All rights reserved.    
  *    
  * Processor:                   FM33LG0xxA    
  * http:                        http://www.fmdevelopers.com.cn/    
  *    
  * Redistribution and use in source and binary forms, with or without    
  * modification, are permitted provided that the following conditions are met    
  *    
  * 1. Redistributions of source code must retain the above copyright notice,    
  *    this list of conditions and the following disclaimer.    
  *    
  * 2. Redistributions in binary form must reproduce the above copyright notice,    
  *    this list of conditions and the following disclaimer in the documentation    
  *    and/or other materials provided with the distribution.    
  *    
  * 3. Neither the name of the copyright holder nor the names of its contributors    
  *    may be used to endorse or promote products derived from this software    
  *    without specific prior written permission.    
  *    
  * 4. To provide the most up-to-date information, the revision of our documents     
  *    on the World Wide Web will be the most Current. Your printed copy may be      
  *    an earlier revision. To verify you have the latest information avaliable,    
  *    refer to: http://www.fmdevelopers.com.cn/.    
  *    
  * THIS SOFTWARE IS PROVIDED BY FUDAN MICRO "AS IS" AND ANY EXPRESSED     
  * ORIMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES     
  * OF MERCHANTABILITY NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE    
  * ARE DISCLAIMED.IN NO EVENT SHALL FUDAN MICRO OR ITS CONTRIBUTORS BE LIABLE     
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL     
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS     
  * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER    
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,     
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISINGIN ANY WAY OUT OF THE     
  * USE OF THIS SOFTWARE, EVEN IF ADVISED OFTHE POSSIBILITY OF SUCH DAMAGE.    
 */    


/* Define to prevent recursive inclusion --------------------------------------------------------------*/
#ifndef __FM33LG0XX_FL_CONF_H
#define __FM33LG0XX_FL_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Defines -------------------------------------------------------------------------------------------*/

/**
  * @brief List of drivers to be used.
  *
  * @note Uncomment following lines to disable specified driver.
  */
#define FL_ADC_DRIVER_ENABLED
#define FL_AES_DRIVER_ENABLED
#define FL_ATIM_DRIVER_ENABLED
#define FL_BSTIM16_DRIVER_ENABLED
#define FL_BSTIM32_DRIVER_ENABLED
#define FL_CAN_DRIVER_ENABLED
#define FL_CDIF_DRIVER_ENABLED
#define FL_CMU_DRIVER_ENABLED
#define FL_COMP_DRIVER_ENABLED
#define FL_CRC_DRIVER_ENABLED
#define FL_DAC_DRIVER_ENABLED
#define FL_DIVAS_DRIVER_ENABLED
#define FL_DMA_DRIVER_ENABLED
#define FL_EXTI_DRIVER_ENABLED
#define FL_FLASH_DRIVER_ENABLED
#define FL_GPIO_DRIVER_ENABLED
#define FL_GPTIM_DRIVER_ENABLED
#define FL_I2C_DRIVER_ENABLED
#define FL_IWDT_DRIVER_ENABLED
#define FL_LCD_DRIVER_ENABLED
#define FL_LPTIM16_DRIVER_ENABLED
#define FL_LPTIM32_DRIVER_ENABLED
#define FL_LPUART_DRIVER_ENABLED
#define FL_PMU_DRIVER_ENABLED
#define FL_RMU_DRIVER_ENABLED
#define FL_RNG_DRIVER_ENABLED
#define FL_RTCA_DRIVER_ENABLED
#define FL_SPI_DRIVER_ENABLED
#define FL_SVD_DRIVER_ENABLED
#define FL_UART_DRIVER_ENABLED
#define FL_VAO_DRIVER_ENABLED
#define FL_VREF_DRIVER_ENABLED
#define FL_VREFP_DRIVER_ENABLED
#define FL_WWDT_DRIVER_ENABLED

/* Device Includes ------------------------------------------------------------------------------------*/
/**
  * @brief Include peripheral's header file
  */

#if defined(FL_ADC_DRIVER_ENABLED)
#include "fm33lg0xx_fl_adc.h"
#endif /* FL_ADC_DRIVER_ENABLED */

#if defined(FL_AES_DRIVER_ENABLED)
#include "fm33lg0xx_fl_aes.h"
#endif /* FL_AES_DRIVER_ENABLED */

#if defined(FL_ATIM_DRIVER_ENABLED)
#include "fm33lg0xx_fl_atim.h"
#endif /* FL_ATIM_DRIVER_ENABLED */

#if defined(FL_BSTIM16_DRIVER_ENABLED)
#include "fm33lg0xx_fl_bstim16.h"
#endif /* FL_BSTIM16_DRIVER_ENABLED */

#if defined(FL_BSTIM32_DRIVER_ENABLED)
#include "fm33lg0xx_fl_bstim32.h"
#endif /* FL_BSTIM32_DRIVER_ENABLED */

#if defined(FL_CAN_DRIVER_ENABLED)
#include "fm33lg0xx_fl_can.h"
#endif /* FL_CAN_DRIVER_ENABLED */

#if defined(FL_CDIF_DRIVER_ENABLED)
#include "fm33lg0xx_fl_cdif.h"
#endif /* FL_CDIF_DRIVER_ENABLED */

#if defined(FL_CMU_DRIVER_ENABLED)
#include "fm33lg0xx_fl_cmu.h"
#endif /* FL_CMU_DRIVER_ENABLED */

#if defined(FL_COMP_DRIVER_ENABLED)
#include "fm33lg0xx_fl_comp.h"
#endif /* FL_COMP_DRIVER_ENABLED */

#if defined(FL_CRC_DRIVER_ENABLED)
#include "fm33lg0xx_fl_crc.h"
#endif /* FL_CRC_DRIVER_ENABLED */

#if defined(FL_DAC_DRIVER_ENABLED)
#include "fm33lg0xx_fl_dac.h"
#endif /* FL_DAC_DRIVER_ENABLED */

#if defined(FL_DIVAS_DRIVER_ENABLED)
#include "fm33lg0xx_fl_divas.h"
#endif /* FL_DIVAS_DRIVER_ENABLED */

#if defined(FL_DMA_DRIVER_ENABLED)
#include "fm33lg0xx_fl_dma.h"
#endif /* FL_DMA_DRIVER_ENABLED */

#if defined(FL_EXTI_DRIVER_ENABLED)
#include "fm33lg0xx_fl_exti.h"
#endif /* FL_EXTI_DRIVER_ENABLED */

#if defined(FL_FLASH_DRIVER_ENABLED)
#include "fm33lg0xx_fl_flash.h"
#endif /* FL_FLASH_DRIVER_ENABLED */

#if defined(FL_GPIO_DRIVER_ENABLED)
#include "fm33lg0xx_fl_gpio.h"
#endif /* FL_GPIO_DRIVER_ENABLED */

#if defined(FL_GPTIM_DRIVER_ENABLED)
#include "fm33lg0xx_fl_gptim.h"
#endif /* FL_GPTIM_DRIVER_ENABLED */

#if defined(FL_I2C_DRIVER_ENABLED)
#include "fm33lg0xx_fl_i2c.h"
#endif /* FL_I2C_DRIVER_ENABLED */

#if defined(FL_IWDT_DRIVER_ENABLED)
#include "fm33lg0xx_fl_iwdt.h"
#endif /* FL_IWDT_DRIVER_ENABLED */

#if defined(FL_LCD_DRIVER_ENABLED)
#include "fm33lg0xx_fl_lcd.h"
#endif /* FL_LCD_DRIVER_ENABLED */

#if defined(FL_LPTIM16_DRIVER_ENABLED)
#include "fm33lg0xx_fl_lptim16.h"
#endif /* FL_LPTIM16_DRIVER_ENABLED */

#if defined(FL_LPTIM32_DRIVER_ENABLED)
#include "fm33lg0xx_fl_lptim32.h"
#endif /* FL_LPTIM32_DRIVER_ENABLED */

#if defined(FL_LPUART_DRIVER_ENABLED)
#include "fm33lg0xx_fl_lpuart.h"
#endif /* FL_LPUART_DRIVER_ENABLED */


#if defined(FL_PMU_DRIVER_ENABLED)
#include "fm33lg0xx_fl_pmu.h"
#endif /* FL_PMU_DRIVER_ENABLED */

#if defined(FL_RMU_DRIVER_ENABLED)
#include "fm33lg0xx_fl_rmu.h"
#endif /* FL_RMU_DRIVER_ENABLED */

#if defined(FL_RNG_DRIVER_ENABLED)
#include "fm33lg0xx_fl_rng.h"
#endif /* FL_RNG_DRIVER_ENABLED */

#if defined(FL_RTCA_DRIVER_ENABLED)
#include "fm33lg0xx_fl_rtca.h"
#endif /* FL_RTCA_DRIVER_ENABLED */

#if defined(FL_SPI_DRIVER_ENABLED)
#include "fm33lg0xx_fl_spi.h"
#endif /* FL_SPI_DRIVER_ENABLED */

#if defined(FL_SVD_DRIVER_ENABLED)
#include "fm33lg0xx_fl_svd.h"
#endif /* FL_SVD_DRIVER_ENABLED */

#if defined(FL_UART_DRIVER_ENABLED)
#include "fm33lg0xx_fl_uart.h"
#endif /* FL_UART_DRIVER_ENABLED */

#if defined(FL_VAO_DRIVER_ENABLED)
#include "fm33lg0xx_fl_vao.h"
#endif /* FL_VAO_DRIVER_ENABLED */

#if defined(FL_VREF_DRIVER_ENABLED)
#include "fm33lg0xx_fl_vref.h"
#endif /* FL_VREF_DRIVER_ENABLED */

#if defined(FL_VREFP_DRIVER_ENABLED)
#include "fm33lg0xx_fl_vrefp.h"
#endif /* FL_VREFP_DRIVER_ENABLED */

#if defined(FL_WWDT_DRIVER_ENABLED)
#include "fm33lg0xx_fl_wwdt.h"
#endif /* FL_WWDT_DRIVER_ENABLED */

#ifdef __cplusplus
}
#endif

#endif /* __FM33LG0XX_FL_CONF_H */

/********************** (C) COPYRIGHT Fudan Microelectronics **** END OF FILE ***********************/
