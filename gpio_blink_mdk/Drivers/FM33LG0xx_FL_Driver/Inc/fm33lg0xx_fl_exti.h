 /**    
  *******************************************************************************************************
  * @file    fm33lg0xx_fl_exti.h
  * @author  FMSH Application Team
  * @brief   Head file of EXTI FL Module
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



/* Define to prevent recursive inclusion---------------------------------------------------------------*/
#ifndef __FM33LG0XX_FL_EXTI_H
#define __FM33LG0XX_FL_EXTI_H

#ifdef __cplusplus
extern "C" {
#endif
/* Includes -------------------------------------------------------------------------------------------*/
#include "fm33lg0xx_fl_def.h"
/** @addtogroup FM33LG0XX_FL_Driver
  * @{
  */

/** @defgroup EXTI EXTI 
  * @brief EXTI FL driver
  * @{
  */

/* Exported types -------------------------------------------------------------------------------------*/
/** @defgroup EXTI_FL_ES_INIT EXTI Exported Init structures
  * @{
  */

/**
  * @brief FL EXTI Common Init Sturcture definition
  */
typedef struct
{
    /*! EXTI时钟源配置 */
    uint32_t clockSource;

} FL_EXTI_CommonInitTypeDef;

/**
  * @brief FL EXTI Init Sturcture definition
  */
typedef struct
{
    /*! EXTI输入配置 */
    uint32_t input;

    /*! EXTI触发边沿配置 */
    uint32_t triggerEdge;

    /*! EXTI数字滤波配置 */
    uint32_t filter;

} FL_EXTI_InitTypeDef;

/**
  * @}
  */
/* Exported constants ---------------------------------------------------------------------------------*/
/** @defgroup EXTI_FL_Exported_Constants EXTI Exported Constants
  * @{
  */



#define    FL_GPIO_EXTI_INPUT_GROUP0                              (0x0U << 0U)
#define    FL_GPIO_EXTI_INPUT_GROUP1                              (0x1U << 0U)
#define    FL_GPIO_EXTI_INPUT_GROUP2                              (0x2U << 0U)
#define    FL_GPIO_EXTI_INPUT_GROUP3                              (0x3U << 0U)



/**
  * @}
  */
/* Exported functions ---------------------------------------------------------------------------------*/
/** @defgroup EXTI_FL_Exported_Functions EXTI Exported Functions
  * @{
  */

/**
  * @}
  */

/** @defgroup EXTI_FL_EF_Init Initialization and de-initialization functions
  * @{
  */

FL_ErrorStatus  FL_EXTI_CommonInit(FL_EXTI_CommonInitTypeDef *init);
FL_ErrorStatus  FL_EXTI_CommonDeinit(void);
void            FL_EXTI_CommonStructInit(FL_EXTI_CommonInitTypeDef *init);

FL_ErrorStatus  FL_EXTI_Init(uint32_t extiLineX, FL_EXTI_InitTypeDef *init);
FL_ErrorStatus  FL_EXTI_DeInit(uint32_t extiLineX);
void            FL_EXTI_StructInit(FL_EXTI_InitTypeDef *init);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __FM33LG0XX_FL_EXTI_H*/

/*************************Py_Code_Generator Version: 0.1-0.14-0.2 @ 2021-03-16*************************/
/*************************(C) COPYRIGHT Fudan Microelectronics **** END OF FILE*************************/
