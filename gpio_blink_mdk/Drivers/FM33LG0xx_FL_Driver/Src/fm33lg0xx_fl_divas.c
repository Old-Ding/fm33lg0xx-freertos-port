 /**    
  *******************************************************************************************************
  * @file    fm33lg0xx_fl_divas.c
  * @author  FMSH Application Team
  * @brief   Src file of DIVAS FL Module
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

/** @addtogroup DIVAS
  * @{
  */

#ifdef FL_DIVAS_DRIVER_ENABLED

/* Private macros ------------------------------------------------------------*/
/** @addtogroup DIVAS_FL_Private_Macros
  * @{
  */
#define         IS_DIVAS_ALL_INSTANCE(INTENCE)              ((INTENCE) == DIVAS)

#define         IS_FL_DIVAS_DIVISOR(__VALUE__)              (((__VALUE__) != 0))

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup DIVAS_FL_EF_Init
  * @{
  */

/**
  * @brief  复位对应HDIV控制寄存器.
  *
  * @param  DIVASx 外设入口地址
  *
  * @retval FL_ErrorStatus枚举值
  *         -FL_PASS 配置成功
  *         -FL_FAIL 配置过程发生错误
  */
FL_ErrorStatus FL_DIVAS_DeInit(DIVAS_Type *DIVASx)
{
    /* 入口参数检查 */
    assert_param(IS_DIVAS_ALL_INSTANCE(DIVASx));
    /* 外设复位使能 */
    FL_RMU_EnablePeripheralReset(RMU);
    /* 恢复寄存器值为默认值 */
    FL_RMU_EnableResetAPBPeripheral(RMU, FL_RMU_RSTAPB_DIVAS);
    FL_RMU_DisableResetAPBPeripheral(RMU, FL_RMU_RSTAPB_DIVAS);
    /* 关闭总线时钟 */
    FL_CMU_DisableGroup2BusClock(FL_CMU_GROUP2_BUSCLK_DIVAS);
    /* 锁定外设复位功能 */
    FL_RMU_DisablePeripheralReset(RMU);
    return FL_PASS;
}

/**
  * @brief  根据 初始化对应外设DIVAS.
  *
  * @param  DIVASx 外设入口地址
  *
  * @retval FL_ErrorStatus枚举值
  *         -FL_PASS 配置成功
  *         -FL_FAIL 配置过程发生错误
  */
FL_ErrorStatus FL_DIVAS_Init(DIVAS_Type *DIVASx)
{
    /* 入口参数检查 */
    assert_param(IS_DIVAS_ALL_INSTANCE(DIVASx));
    /* 使能时钟总线 */
    FL_CMU_EnableGroup2BusClock(FL_CMU_GROUP2_BUSCLK_DIVAS);
    return FL_PASS;
}
/**
  * @brief 硬件除法器计算函数
  *
  * @param  DIVASx 外设入口地址
  * @param  DivisorEnd 32位有符号被除数
  * @param  Divisor 16位有符号除数，注意不能为0
  * @param  Quotient 指向 @ref int32_t 指针 保存商的地址
  * @param  Residue 指向 @ref int16_t 指针 保存余数的地址
  *
  * @retval 计算正确性与否
  *         -0 计算结果正确
  *         -非0 计算过程发生错误
  */
uint32_t FL_DIVAS_Hdiv_Calculation(DIVAS_Type *DIVASx, int32_t DivisorEnd, int16_t Divisor, int32_t *Quotient, int16_t *Residue)
{
    uint32_t  TimeOut ;
    FL_DIVAS_SetMode(DIVASx, FL_DIVAS_MODE_DIV);
    FL_DIVAS_WriteOperand(DIVASx, (uint32_t)DivisorEnd);
    FL_DIVAS_WriteDivisor(DIVASx, (uint32_t)Divisor);
    if(FL_DIVAS_IsActiveFlag_DividedZero(DIVASx))
    {
        /*除数为0 */
        *Quotient = 0;
        *Residue  = 0;
        return 1;
    }
    TimeOut = FL_DIVAS_SR_BUSY_TIMEOUT;
    while(FL_DIVAS_IsActiveFlag_Busy(DIVASx))
    {
        TimeOut--;
        if(TimeOut == 0)
        {
            /* 计算超时*/
            *Quotient = 0;
            *Residue  = 0;
            return 3;
        }
    }
    *Quotient = FL_DIVAS_ReadQuotient(DIVASx);
    *Residue  = FL_DIVAS_ReadResidue(DIVASx);
    return 0;
}

/**
  * @brief 硬件开方计算函数
  *
  * @param  DIVASx 外设入口地址
  * @param  Root 32bit被开方数
  * @param  Result 指向 @ref int16_t 指针 保存结果开方根
  *
  * @retval 计算正确性与否
  *         -0 计算结果正确
  *         -非0 计算过程发生错误
  */
uint32_t FL_DIVAS_Root_Calculation(DIVAS_Type *DIVASx, uint32_t Root, uint16_t *Result)
{
    uint32_t  TimeOut ;
    FL_DIVAS_SetMode(DIVASx, FL_DIVAS_MODE_ROOT);
    FL_DIVAS_WriteOperand(DIVASx, Root);
    TimeOut = FL_DIVAS_SR_BUSY_TIMEOUT;
    while(FL_DIVAS_IsActiveFlag_Busy(DIVASx))
    {
        TimeOut --;
        if(TimeOut == 0)
        {
            /* 计算超时*/
            *Result = 0;
            return 1;
        }
    }
    *Result = FL_DIVAS_ReadRoot(DIVASx);
    return 0;
}

/**
  * @}
  */
 
#endif /* FL_DIVAS_DRIVER_ENABLED */

/**
  * @}
  */

/**
  * @}
  */

/********************** (C) COPYRIGHT Fudan Microelectronics **** END OF FILE ***********************/

