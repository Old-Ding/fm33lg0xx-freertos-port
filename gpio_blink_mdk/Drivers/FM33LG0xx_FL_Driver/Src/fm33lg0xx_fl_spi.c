 /**    
  *******************************************************************************************************
  * @file    fm33lg0xx_fl_spi.c
  * @author  FMSH Application Team
  * @brief   Src file of SPI FL Module
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

/** @addtogroup SPI
  * @{
  */

#ifdef FL_SPI_DRIVER_ENABLED

/* Private macros ------------------------------------------------------------*/
/** @addtogroup SPI_FL_Private_Macros
  * @{
  */
#define         IS_FL_SPI_INSTANCE(INTANCE)                     (((INTANCE) == SPI0)||\
                                                                 ((INTANCE) == SPI1)||\
                                                                 ((INTANCE) == SPI2))

#define         IS_FL_SPI_MODE(__VALUE__)                       (((__VALUE__) == FL_SPI_WORK_MODE_SLAVE)||\
                                                                 ((__VALUE__) == FL_SPI_WORK_MODE_MASTER))

#define         IS_FL_SPI_BITORDER(__VALUE__)                   (((__VALUE__) == FL_SPI_BIT_ORDER_MSB_FIRST)||\
                                                                 ((__VALUE__) == FL_SPI_BIT_ORDER_LSB_FIRST))

#define         IS_FL_SPI_DATAWIDT(__VALUE__)                   (((__VALUE__) == FL_SPI_DATA_WIDTH_8B)||\
                                                                 ((__VALUE__) == FL_SPI_DATA_WIDTH_16B)||\
                                                                 ((__VALUE__) == FL_SPI_DATA_WIDTH_24B)||\
                                                                 ((__VALUE__) == FL_SPI_DATA_WIDTH_32B))

#define         IS_FL_SPI_CLOCK_PHASE(__VALUE__)                (((__VALUE__) == FL_SPI_PHASE_EDGE1)||\
                                                                 ((__VALUE__) == FL_SPI_PHASE_EDGE2))

#define         IS_FL_SPI_CLOCK_POLARITY(__VALUE__)             (((__VALUE__) == FL_SPI_POLARITY_NORMAL)||\
                                                                 ((__VALUE__) == FL_SPI_POLARITY_INVERT))

#define         IS_FL_SPI_BAUDRATE(__VALUE__)                   (((__VALUE__) == FL_SPI_BAUDRATE_DIV2)||\
                                                                 ((__VALUE__) == FL_SPI_BAUDRATE_DIV4)||\
                                                                 ((__VALUE__) == FL_SPI_BAUDRATE_DIV8)||\
                                                                 ((__VALUE__) == FL_SPI_BAUDRATE_DIV16)||\
                                                                 ((__VALUE__) == FL_SPI_BAUDRATE_DIV32)||\
                                                                 ((__VALUE__) == FL_SPI_BAUDRATE_DIV64)||\
                                                                 ((__VALUE__) == FL_SPI_BAUDRATE_DIV128)||\
                                                                 ((__VALUE__) == FL_SPI_BAUDRATE_DIV256))


#define         IS_FL_SPI_TANSFERMODE(__VALUE__)                (((__VALUE__) == FL_SPI_TRANSFER_MODE_FULL_DUPLEX)||\
                                                                 ((__VALUE__) == FL_SPI_TRANSFER_MODE_HALF_DUPLEX))


/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup SPI_FL_EF_Init
  * @{
  */

/**
  * @brief  复位对应SPI寄存器.
  * @param  SPIx
  * @retval ErrorStatus枚举值:
  *         -FL_PASS 外设寄存器值恢复复位值
  *         -FL_FAIL 未成功执行
  */
FL_ErrorStatus FL_SPI_DeInit(SPI_Type *SPIx)
{
    assert_param(IS_FL_SPI_INSTANCE(SPIx));
    /* 使能外设复位 */
    FL_RMU_EnablePeripheralReset(RMU);
    if(SPIx == SPI0)
    {
        /* 复位SPI外设寄存器 */
        FL_RMU_EnableResetAPBPeripheral(RMU, FL_RMU_RSTAPB_SPI0);
        FL_RMU_DisableResetAPBPeripheral(RMU, FL_RMU_RSTAPB_SPI0);
        /* 关闭外设总线始时钟和工作时钟 */
        FL_CMU_DisableGroup3BusClock(FL_CMU_GROUP3_BUSCLK_SPI0);
    }
    else
        if(SPIx == SPI1)
        {
            /* 复位SPI外设寄存器 */
            FL_RMU_EnableResetAPBPeripheral(RMU, FL_RMU_RSTAPB_SPI1);
            FL_RMU_DisableResetAPBPeripheral(RMU, FL_RMU_RSTAPB_SPI1);
            /* 关闭外设总线始时钟和工作时钟 */
            FL_CMU_DisableGroup3BusClock(FL_CMU_GROUP3_BUSCLK_SPI1);
        }
        else
            if(SPIx == SPI2)
            {
                /* 复位SPI外设寄存器 */
                FL_RMU_EnableResetAPBPeripheral(RMU, FL_RMU_RSTAPB_SPI2);
                FL_RMU_DisableResetAPBPeripheral(RMU, FL_RMU_RSTAPB_SPI2);
                /* 关闭外设总线始时钟和工作时钟 */
                FL_CMU_DisableGroup3BusClock(FL_CMU_GROUP3_BUSCLK_SPI2);
            }
            else
            {
                FL_RMU_DisablePeripheralReset(RMU);
                return FL_FAIL;
            }
    /* 锁定外设复位 */
    FL_RMU_DisablePeripheralReset(RMU);
    return FL_PASS;
}
/**
  * @brief  根据 SPI_InitStruct 的配置信息初始化对应外设入口地址的寄存器值.
  * @param  SPIx SPIx
  * @param  SPI_InitStruct 指向一个 @ref FL_SPI_InitTypeDef 结构体
  *         其中包含了外设的相关配置信息.
  * @retval ErrorStatus枚举值
  *         -FL_FAIL 配置过程发生错误
  *         -FL_PASS SPI配置成功
  */
FL_ErrorStatus FL_SPI_Init(SPI_Type *SPIx, FL_SPI_InitTypeDef *initStruct)
{
    assert_param(IS_FL_SPI_INSTANCE(SPIx));
    assert_param(IS_FL_SPI_MODE(initStruct->mode));
    assert_param(IS_FL_SPI_BITORDER(initStruct->bitOrder));
    assert_param(IS_FL_SPI_DATAWIDT(initStruct->dataWidth));
    assert_param(IS_FL_SPI_BAUDRATE(initStruct->baudRate));
    assert_param(IS_FL_SPI_CLOCK_PHASE(initStruct->clockPhase));
    assert_param(IS_FL_SPI_CLOCK_POLARITY(initStruct->clockPolarity));
    if(SPIx == SPI0)
    {
        /* 外设总线始时钟 */
        FL_CMU_EnableGroup3BusClock(FL_CMU_GROUP3_BUSCLK_SPI0);
    }
    else
        if(SPIx == SPI1)
        {
            /* 外设总线始时钟 */
            FL_CMU_EnableGroup3BusClock(FL_CMU_GROUP3_BUSCLK_SPI1);
        }
        else
            if(SPIx == SPI2)
            {
                /* 外设总线始时钟 */
                FL_CMU_EnableGroup3BusClock(FL_CMU_GROUP3_BUSCLK_SPI2);
            }
            else
            {
                return FL_FAIL;
            }
    /* 选择NSS脚控制模式 */
    if(initStruct->softControl == FL_ENABLE)
    {
        FL_SPI_EnableSSNSoftControl(SPIx);
    }
    else
    {
        FL_SPI_DisableSSNSoftControl(SPIx);
    }
    /* 外设工作主从模式 */
    FL_SPI_SetWorkMode(SPIx, initStruct->mode);
    /* 总线通讯速率 */
    FL_SPI_SetClockDivision(SPIx, initStruct->baudRate);
    /* 数据bit方向 */
    FL_SPI_SetBitOrder(SPIx, initStruct->bitOrder);
    /* 总线数据位宽 */
    FL_SPI_SetDataWidth(SPIx, initStruct->dataWidth);
    /* 时钟相位 */
    FL_SPI_SetClockPhase(SPIx, initStruct->clockPhase);
    /* 传输模式 双工半双工  */
    FL_SPI_SetTransferMode(SPIx, initStruct->transferMode);
    /* 时钟极性 */
    FL_SPI_SetClockPolarity(SPIx, initStruct->clockPolarity);
    /* 使能 外设*/
    FL_SPI_Enable(SPIx);
    return FL_PASS;
}
/**
  * @brief  设置 SPI_InitStruct 为默认配置
  * @param  SPI_InitStruct 指向需要将值设置为默认配置的结构体 @ref FL_SPI_InitTypeDef 结构体
  *
  * @retval None
  */

void FL_SPI_StructInit(FL_SPI_InitTypeDef *initStruct)
{
    initStruct->softControl         =   FL_DISABLE;
    initStruct->mode                =   FL_SPI_WORK_MODE_MASTER;
    initStruct->baudRate            =   FL_SPI_CLK_DIV8;
    initStruct->bitOrder            =   FL_SPI_BIT_ORDER_MSB_FIRST;
    initStruct->dataWidth           =   FL_SPI_DATA_WIDTH_8B;
    initStruct->clockPolarity       =   FL_SPI_POLARITY_NORMAL;
    initStruct->clockPhase          =   FL_SPI_PHASE_EDGE1;
    initStruct->transferMode        =   FL_SPI_TRANSFER_MODE_FULL_DUPLEX;
}

/**
  * @}
  */

#endif /* FL_SPI_DRIVER_ENABLED */

/**
  * @}
  */

/**
  * @}
  */

/*************************(C) COPYRIGHT Fudan Microelectronics **** END OF FILE*************************/

