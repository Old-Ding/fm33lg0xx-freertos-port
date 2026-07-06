 /** 
  ****************************************************************************************************
  * @file    fm33lg0xx_fl_aes.c
  * @author  FMSH Application Team
  * @brief   Src file of AES FL Module
  ****************************************************************************************************
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

/** @addtogroup FM33LG0xx_FL_Driver
  * @{
  */

/** @addtogroup AES
  * @{
  */

#ifdef FL_AES_DRIVER_ENABLED

/* Private macros ------------------------------------------------------------*/
/** @addtogroup AES_FL_Private_Macros
  * @{
  */
#define         IS_FL_AES_INSTANCE(INSTANCE)                (((INSTANCE) == AES))

#define         IS_FL_AES_KEYLENTH(__VALUE__)               (((__VALUE__) == FL_AES_KEY_LENGTH_128B)||\
                                                             ((__VALUE__) == FL_AES_KEY_LENGTH_192B)||\
                                                             ((__VALUE__) == FL_AES_KEY_LENGTH_256B))

#define         IS_FL_AES_CIPHERMODE(__VALUE__)             (((__VALUE__) == FL_AES_CIPHER_ECB)||\
                                                             ((__VALUE__) == FL_AES_CIPHER_CBC)||\
                                                             ((__VALUE__) == FL_AES_CIPHER_CTR)||\
                                                             ((__VALUE__) == FL_AES_CIPHER_MULTH))

#define         IS_FL_AES_OPERATIONMODE(__VALUE__)          (((__VALUE__) == FL_AES_OPERATION_MODE_ENCRYPTION)||\
                                                             ((__VALUE__) == FL_AES_OPERATION_MODE_KEYDERIVATION)||\
                                                             ((__VALUE__) == FL_AES_OPERATION_MODE_DECRYPTION)||\
                                                             ((__VALUE__) == FL_AES_OPERATION_MODE_KEYDERIVATION_DECRYPTION))

#define         IS_FL_AES_DATATYPE(__VALUE__)               (((__VALUE__) == FL_AES_DATA_TYPE_32B)||\
                                                             ((__VALUE__) == FL_AES_DATA_TYPE_16B)||\
                                                             ((__VALUE__) == FL_AES_DATA_TYPE_8B)||\
                                                             ((__VALUE__) == FL_AES_DATA_TYPE_1B))
/**
  *@}
  */
 
/* Exported functions --------------------------------------------------------*/
/** @addtogroup AES_FL_EF_Init
  * @{
  */

/**
  * @brief  复位AES 外设寄存器值为复位值
  *
  * @param  None
  *
  * @retval FL_ErrorStatus枚举值
  *         -FL_PASS 配置成功
  *         -FL_FAIL 配置过程发生错误
  */
FL_ErrorStatus FL_AES_DeInit(void)
{
    /* 外设复位使能 */
    FL_RMU_EnablePeripheralReset(RMU);
    /* 复位AES */
    FL_RMU_EnableResetAPBPeripheral(RMU, FL_RMU_RSTAPB_AES);
    FL_RMU_DisableResetAPBPeripheral(RMU, FL_RMU_RSTAPB_AES);
    /* 关闭总线时钟 */
    FL_CMU_DisableGroup2BusClock(FL_CMU_GROUP2_BUSCLK_AES);
    /* 锁定外设复位功能 */
    FL_RMU_DisablePeripheralReset(RMU);
    return FL_PASS;
}
/**
  * @brief  根据 AES_InitStructer初始化对应外设入口地址的寄存器值.
  *
  * @param  AESx 外设入口地址
  * @param  AES_InitStructer 指向 @ref FL_AES_InitTypeDef 结构体的指针
  *
  * @retval FL_ErrorStatus枚举值
  *         -FL_PASS 配置成功
  *         -FL_FAIL 配置过程发生错误
  */
FL_ErrorStatus FL_AES_Init(AES_Type *AESx, FL_AES_InitTypeDef *AES_InitStructer)
{
    /* 入口合法性检查 */
    assert_param(IS_FL_AES_INSTANCE(AESx));
    assert_param(IS_FL_AES_KEYLENTH(AES_InitStructer->keyLength));
    assert_param(IS_FL_AES_CIPHERMODE(AES_InitStructer->cipherMode));
    assert_param(IS_FL_AES_OPERATIONMODE(AES_InitStructer->operationMode));
    assert_param(IS_FL_AES_DATATYPE(AES_InitStructer->dataType));
    if(FL_AES_IsEnabled(AESx) == 0)
    {
        /* 开启总线时钟 */
        FL_CMU_EnableGroup2BusClock(FL_CMU_GROUP2_BUSCLK_AES);
        /* key长度 */
        FL_AES_SetKeySize(AESx, AES_InitStructer->keyLength);
        /* 数据流处理模式 */
        FL_AES_SetCipherMode(AESx, AES_InitStructer->cipherMode);
        /* 操作模式 */
        FL_AES_SetOperationMode(AESx, AES_InitStructer->operationMode);
        /* 数据类型 */
        FL_AES_SetDataType(AESx, AES_InitStructer->dataType);
    }
    else
    {
        return FL_FAIL;
    }
    return FL_PASS;
}
/**
  * @brief  设置 AES_InitStruct 为默认配置
  *
  * @param  AES_InitStruct 指向需要将值设置为默认配置的结构体 @ref FL_AES_InitTypeDef 结构体
  *
  * @retval None
  */
void FL_AES_StructInit(FL_AES_InitTypeDef *AES_InitStructer)
{
    AES_InitStructer->keyLength      = FL_AES_KEY_LENGTH_128B;
    AES_InitStructer->cipherMode    = FL_AES_CIPHER_ECB;
    AES_InitStructer->operationMode = FL_AES_OPERATION_MODE_ENCRYPTION;
    AES_InitStructer->dataType      = FL_AES_DATA_TYPE_32B;
}

/**
  *@}
  */

#endif /* FL_AES_DRIVER_ENABLED */

/**
  *@}
  */

/**
  *@}
  */

/*********************** (C) COPYRIGHT Fudan Microelectronics *****END OF FILE************************/
