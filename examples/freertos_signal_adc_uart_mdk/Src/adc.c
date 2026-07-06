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
#include "adc.h"
#include "FreeRTOS.h"
#include "task.h"

/**
  * @brief  ADC初始化
  * @param  void
  * @retval void
  */
void ADC_Init(void)
{
    FL_GPIO_InitTypeDef         GPIO_InitStruct={0};
    FL_ADC_CommonInitTypeDef    ADC_CommonInitStruct;
    FL_ADC_InitTypeDef          ADC_InitStruct ;

    /* 配置引脚为模拟功能 */
    /* ADC ADC_1 引脚 PD1 */
    GPIO_InitStruct.pin            = FL_GPIO_PIN_1;
    GPIO_InitStruct.mode           = FL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.outputType     = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull           = FL_DISABLE;
    GPIO_InitStruct.remapPin       = FL_DISABLE;
    GPIO_InitStruct.analogSwitch   = FL_DISABLE;
    FL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* ADC 时钟设置 */
    ADC_CommonInitStruct.clockSource                = FL_CMU_ADC_CLK_SOURCE_RCHF;
    ADC_CommonInitStruct.clockPrescaler             = FL_ADC_CLK_PSC_DIV8;
    ADC_CommonInitStruct.referenceSource            = FL_ADC_REF_SOURCE_VDDA;
    ADC_CommonInitStruct.bitWidth                   = FL_ADC_BIT_WIDTH_12B;
    FL_ADC_CommonInit(&ADC_CommonInitStruct);

    /* ADC 寄存器设置 */
    ADC_InitStruct.conversionMode               = FL_ADC_CONV_MODE_SINGLE;                /* 单次模式 */
    ADC_InitStruct.autoMode                     = FL_ADC_SINGLE_CONV_MODE_AUTO;           /* 自动 */
    ADC_InitStruct.waitMode                     = FL_ENABLE;                              /* 等待 */ 
    ADC_InitStruct.overrunMode                  = FL_ENABLE;                              /* 覆盖上次数据 */
    ADC_InitStruct.scanDirection                = FL_ADC_SEQ_SCAN_DIR_FORWARD;            /* 通道正序扫描 */
    ADC_InitStruct.externalTrigConv             = FL_ADC_TRIGGER_EDGE_NONE;               /* 禁止触发信号 */
    ADC_InitStruct.triggerSource                = FL_ADC_TRGI_LUT0;
    ADC_InitStruct.fastChannelTime              = FL_ADC_FAST_CH_SAMPLING_TIME_2_ADCCLK;  /* 快速通道采样时间 */
    ADC_InitStruct.lowChannelTime               = FL_ADC_SLOW_CH_SAMPLING_TIME_192_ADCCLK;/* 慢速通道采样时间 */
    ADC_InitStruct.oversamplingMode             = FL_ENABLE;                              /* 过采样关闭 */
    ADC_InitStruct.overSampingMultiplier        = FL_ADC_OVERSAMPLING_MUL_8X;             /* 8倍过采样 */
    ADC_InitStruct.oversamplingShift            = FL_ADC_OVERSAMPLING_SHIFT_3B;           /* 数据右移, /8 */

    FL_ADC_Init(ADC, &ADC_InitStruct);

}

/**
  * @brief  VREF1P2采样
  * @param  void
  * @retval ADCRdresult: 采样值
  */
static uint32_t GetVREF1P2Sample_POLL(void)
{
    uint16_t ADCRdresult;
    uint8_t i=0;
    FL_VREF_EnableVREFBuffer(VREF);       /* 使能VREF BUFFER */
    FL_ADC_EnableSequencerChannel(ADC, FL_ADC_INTERNAL_VREF1P2);/* 通道选择VREF */
            
    FL_ADC_ClearFlag_EndOfConversion(ADC);/* 清标志 */
    FL_ADC_Enable(ADC);                   /* 启动ADC */
    FL_ADC_EnableSWConversion(ADC);       /* 开始转换 */
    /* 等待转换完成 */
    while (FL_ADC_IsActiveFlag_EndOfConversion(ADC) == FL_RESET)
    {
        if(i>=5)
        {
            break;
        }
        i++;
        vTaskDelay(1);
    }
    FL_ADC_ClearFlag_EndOfConversion(ADC);/* 清标志 */
    ADCRdresult =FL_ADC_ReadConversionData(ADC);//获取采样值

    FL_ADC_Disable(ADC);                  /* 关闭ADC */
    FL_ADC_DisableSequencerChannel(ADC, FL_ADC_INTERNAL_VREF1P2);/* 通道关闭VREF */
    FL_VREF_DisableVREFBuffer(VREF);      /* 关闭VREF BUFFER */
    /* 转换结果 */ 
    return ADCRdresult;
}

/**
  * @brief  单端输入采样
  * @param  channel: 采样通道
  * @retval ADCRdresult: 采样值
  */
static uint32_t GetSingleChannelSample_POLL(uint32_t channel)
{
    uint16_t ADCRdresult;
    uint8_t i=0;
    FL_ADC_EnableSequencerChannel(ADC, channel); /* 通道选择ADC_1 */

    FL_ADC_ClearFlag_EndOfConversion(ADC);       /* 清标志 */
    FL_ADC_Enable(ADC);                          /* 启动ADC */
    FL_ADC_EnableSWConversion(ADC);              /* 开始转换 */
    /* 等待转换完成 */
    while (FL_ADC_IsActiveFlag_EndOfConversion(ADC) == FL_RESET)
    {
        if(i>=5)
        {
            break;
        }
        i++;
        vTaskDelay(1);
    }
    FL_ADC_ClearFlag_EndOfConversion(ADC);       /* 清标志 */
    ADCRdresult =FL_ADC_ReadConversionData(ADC); /* 获取采样值 */

    FL_ADC_Disable(ADC);                         /* 关闭ADC */
    FL_ADC_DisableSequencerChannel(ADC, channel);/* 通道 */ 
    /* 转换结果 */
    return ADCRdresult;
}

/**
  * @brief  单端输入采样
  * @param  channel: 采样通道
  * @retval GetChannelVoltage: 通道电压值
  */
uint32_t GetSingleChannelVoltage_POLL(uint32_t channel)
{
    uint32_t Get122VSample,GetChannelVoltage;
    uint64_t GetVSample; 

    Get122VSample = GetVREF1P2Sample_POLL();
    GetVSample =GetSingleChannelSample_POLL(channel);
    if(0U == Get122VSample)
    {
        /* VREF采样无效时停止换算，避免把硬件采样异常扩散成除零故障。 */
        return 0U;
    }
    GetChannelVoltage =  (GetVSample *3000*(ADC_VREF))/(Get122VSample*4095);
    /* 转换结果 */ 
    return GetChannelVoltage;
}

