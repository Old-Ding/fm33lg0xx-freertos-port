 /*    
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
#include "svd.h"

__WEAK void SVD_DelayUs(uint32_t u32Delay_us)
{
    /* 默认保持裸机行为；接入 RTOS 后由应用层覆盖，避免 SVD 模块依赖调度器。 */
    FL_DelayUs(u32Delay_us);
}

/**
  * @brief  将SVS引脚初始化为模拟端口
  * @param  void
  * @retval void
  */
static void SVD_SVS_GPIO_Init(void)
{
    FL_GPIO_InitTypeDef gpioInitStruct = {0};

    gpioInitStruct.pin        = SVD_SVS_PIN;
    gpioInitStruct.mode       = FL_GPIO_MODE_ANALOG;
    gpioInitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    gpioInitStruct.pull       = FL_DISABLE;
    gpioInitStruct.remapPin   = FL_DISABLE;

    FL_GPIO_Init(SVD_SVS_GPIO, &gpioInitStruct);
}


/**
  * @brief  SVD初始化
  * @param  eSVD_MonitroPower: 选择SVD监控电源
  *         u32WarnThreshold： 报警阈值档位
  *         u32RevVoltage：    参考基准
  * @retval void
  */
void SVD_Init(SVD_MONTIOR_POWER eSVD_MonitroPower, uint32_t u32WarnThreshold, uint32_t u32RevVoltage)
{
    FL_SVD_InitTypeDef    SVD_InitStruct;

    if(SVD_MONTIOR_SVS == eSVD_MonitroPower)
    {   /* SVS引脚初始化为模拟功能 */
        SVD_SVS_GPIO_Init();
    }

    /* 使能外部SVS通道 */
    SVD_InitStruct.SVSChannel       = (FL_FunState)(SVD_MONTIOR_SVS == eSVD_MonitroPower);
    /* 使能数字滤波 */
    SVD_InitStruct.digitalFilter    = FL_ENABLE;
    /* 参考基准输入选择 */
    SVD_InitStruct.referenceVoltage = u32RevVoltage;
    /* 工作模式配置 */
    SVD_InitStruct.workMode         = FL_SVD_WORK_MODE_CONTINUOUS;
    /* 间歇使能间隔配置 */
    SVD_InitStruct.enablePeriod     = FL_SVD_ENABLE_PERIOD_62P5MS;
    /* 报警阈值配置 */
    SVD_InitStruct.warningThreshold = u32WarnThreshold;
    /* 报警阈值设置 */
    FL_SVD_Init(SVD, &SVD_InitStruct);
    /* 使能SVD */
    FL_SVD_Enable(SVD);
    /* SVD使能后到稳定输出等待500us */
    SVD_DelayUs(500);
}

/**
  * @brief  SVD监测结果（SVDO标志位）轮询输出
            SVD实时电压监测标志，无滤波
  * @param  void
  * @retval None
  */
SVD_RESULT SVD_SVDO_POLL(void)
{
    /* SVD实时电压监测标志；软件避免写此寄存器 */
    return (SVD_RESULT)(SVD_ISR_SVDO_Msk == FL_SVD_GetCurrentPowerStatus(SVD));
}

/**
  * @brief  SVD监测结果（SVDR标志）轮询输出
            仅在数字滤波使能时有意义，即滤波后的锁存信号 
  * @param  void
  * @retval None
  */
SVD_RESULT SVD_SVDR_POLL(void)
{
    /* SVD内部滤波后的电压监测标志，仅在使能数字滤波时有意义；软件避免写此寄存器 */
    return (SVD_RESULT)(SVD_ISR_SVDR_Msk == FL_SVD_GetLatchedPowerStatus(SVD));
}

/**
  * @brief  确认SVD监测结果是否处于目标状态
  * @param  eSVDResultType：要确认的目状态SVD_BELOW_THRESHOLD或SVD_HIGHER_THRESHOLD
            u32Delay_us：   防抖时长，单位us
  * @retval bool   true： 当前SVD监测结果与目标状态一致
                   false：当前SVD监测结果与目标状态不一致
  */
bool SVD_Result_Confirmed(SVD_RESULT eSVDResultType, uint32_t u32Delay_us)
{
    if (0 == u32Delay_us)
    {
        u32Delay_us += 1;
    }
    /* 注意是否使能数字滤波，调用不同的结果轮询函数 */
    while ((SVD_SVDR_POLL() == eSVDResultType) && u32Delay_us)
    {
        if (--u32Delay_us)
        {
            SVD_DelayUs(1);
        }
    }
    
    return (bool)(0 == u32Delay_us);
}

