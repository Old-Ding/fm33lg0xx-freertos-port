 /** 
  *******************************************************************************************************
  * @file    fm33lg0xx_fl_bstim32.c
  * @author  FMSH Application Team
  * @brief   Src file of BSTIM FL Module
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

/* Includes ------------------------------------------------------------------*/
#include "fm33lg0xx_fl.h"

/** @addtogroup FM33LG0XX_FL_Driver
  * @{
  */

/** @addtogroup BSTIM32
  * @{
  */

#ifdef FL_BSTIM32_DRIVER_ENABLED

/* Private macros ------------------------------------------------------------*/
/** @addtogroup BSTIM32_FL_Private_Macros
  * @{
  */
#define         IS_FL_BSTIM32_INSTANCE(INTANCE)                     ((INTANCE) == BSTIM32)


#define         IS_FL_BSTIM32_AUTORELOAD_MODE(__VALUE__)           (((__VALUE__) == FL_ENABLE)||\
                                                                    ((__VALUE__) == FL_DISABLE))

#define         IS_FL_BSTIM32_CLOCK_SRC(__VALUE__)                 (((__VALUE__) == FL_CMU_BSTIM32_CLK_SOURCE_APBCLK)||\
                                                                    ((__VALUE__) == FL_CMU_BSTIM32_CLK_SOURCE_RCLP)||\
                                                                    ((__VALUE__) == FL_CMU_BSTIM32_CLK_SOURCE_RCLF)||\
                                                                    ((__VALUE__) == FL_CMU_BSTIM32_CLK_SOURCE_LSCLK))

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup BSTIM_FL_EF_Init
  * @{
  */

/**
  * @brief  复位对应BSTIM32寄存器.
  * @param  BSTIMx
  * @retval ErrorStatus枚举值:
  *         -FL_PASS 外设寄存器值恢复复位值
  *         -FL_FAIL 未成功执行
  */
FL_ErrorStatus FL_BSTIM32_DeInit(BSTIM32_Type *BSTIM32x)
{
    assert_param(IS_FL_BSTIM32_INSTANCE(BSTIM32x));
    /* 使能外设复位 */
    FL_RMU_EnablePeripheralReset(RMU);
    /* 复位外设寄存器 */
    FL_RMU_EnableResetAPBPeripheral(RMU, FL_RMU_RSTAPB_BSTIM32);
    FL_RMU_DisableResetAPBPeripheral(RMU, FL_RMU_RSTAPB_BSTIM32);
    /* 关闭外设总线始时钟和工作时钟 */
    FL_CMU_DisableGroup4BusClock(FL_CMU_GROUP4_BUSCLK_BSTIM32);
    FL_CMU_DisableGroup3OperationClock(FL_CMU_GROUP3_OPCLK_BSTIM32);
    /* 锁定外设复位 */
    FL_RMU_DisablePeripheralReset(RMU);
    return FL_PASS;
}
/**
  * @brief  根据 BSTIM32_InitStruct 的配置信息初始化对应外设入口地址的寄存器值.
  * @param  BSTIMx BSTIMx
  * @param  BSTIM32_InitStruct 指向一个 @ref FL_BSTIM32_InitTypeDef 结构体
  *         其中包含了外设的相关配置信息.
  * @retval ErrorStatus枚举值
  *         -FL_FAIL 配置过程发生错误
  *         -FL_PASS 配置成功
  */
FL_ErrorStatus FL_BSTIM32_Init(BSTIM32_Type *BSTIM32x, FL_BSTIM32_InitTypeDef *init)
{
    uint32_t i = 5;
    /* 参数检查 */
    assert_param(IS_FL_BSTIM32_INSTANCE(BSTIM32x));
    assert_param(IS_FL_BSTIM32_CLOCK_SRC(init->clockSource));
    assert_param(IS_FL_BSTIM32_AUTORELOAD_MODE(init->autoReloadState));
    /* 时钟使能 */
    FL_CMU_EnableGroup4BusClock(FL_CMU_GROUP4_BUSCLK_BSTIM32);
    /* 选择时钟源 */
    FL_CMU_SetBSTIM32ClockSource(init->clockSource);
    FL_CMU_EnableGroup3OperationClock(FL_CMU_GROUP3_OPCLK_BSTIM32);
    /* 分频系数 */
    FL_BSTIM32_WritePrescaler(BSTIM32x, init->prescaler);
    /* 自动重装载值 */
    FL_BSTIM32_EnableUpdateEvent(BSTIM32x);
    FL_BSTIM32_WriteAutoReload(BSTIM32x, init->autoReload);
    if(init->autoReloadState == FL_ENABLE)
    {
        FL_BSTIM32_EnableARRPreload(BSTIM32x);
    }
    else
    {
        FL_BSTIM32_DisableARRPreload(BSTIM32x);
    }
    FL_BSTIM32_GenerateUpdateEvent(BSTIM32x);
    while((!FL_BSTIM32_IsActiveFlag_Update(BSTIM32x))&&i)
    {
        i--;
    }
    return FL_PASS;
}

/**
  * @brief  设置 BSTIM32_InitStruct 为默认配置
  * @param  BSTIM32_InitStruct 指向需要将值设置为默认配置的结构体 @ref FL_BSTIM_InitTypeDef 结构体
  *
  * @retval None
  */
void FL_BSTIM32_StructInit(FL_BSTIM32_InitTypeDef *init)
{
    init->prescaler         = 0;
    init->autoReload        = 0xFFFFFFFF;
    init->autoReloadState   = FL_ENABLE;
    init->clockSource       = FL_CMU_BSTIM32_CLK_SOURCE_APBCLK;
}

/**
  * @}
  */

#endif /* FL_BSTIM32_DRIVER_ENABLED */

/**
  * @}
  */

/**
  * @}
  */


/********************** (C) COPYRIGHT Fudan Microelectronics **** END OF FILE ***********************/
