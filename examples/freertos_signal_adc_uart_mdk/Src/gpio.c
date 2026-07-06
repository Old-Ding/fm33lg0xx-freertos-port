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
#include "gpio.h"
#include "fm33lg0xx_fl.h"
#include "taskgpio.h"

/**
  * @brief  配置GPIO为输出
  * @param  void
  * @retval void
  */
void GPIO_Init(void)
{
    FL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.pin             = LED0_PIN;
    GPIO_InitStruct.mode            = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.outputType      = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull            = FL_DISABLE;
    GPIO_InitStruct.remapPin        = FL_DISABLE;
    GPIO_InitStruct.analogSwitch    = FL_DISABLE;
    FL_GPIO_Init(LED0_GPIO, &GPIO_InitStruct);
}

void GPIO_IRQHandler(void)
{
    if(FL_GPIO_IsActiveFlag_EXTI(GPIO, GPIO_TRIGGER_EXTI_LINE))
    {
        FL_GPIO_ClearFlag_EXTI(GPIO, GPIO_TRIGGER_EXTI_LINE);
        GPIO_IRQCallBack();
    }
}

void GPIO_InterruptInit(void)
{
    FL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    FL_CMU_EnableEXTIOnSleep();
    FL_CMU_EnableGroup3OperationClock(FL_CMU_GROUP3_OPCLK_EXTI);

    GPIO_InitStruct.pin             = GPIO_TRIGGER_PIN;
    GPIO_InitStruct.mode            = FL_GPIO_MODE_INPUT;
    GPIO_InitStruct.outputType      = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull            = FL_DISABLE;
    GPIO_InitStruct.remapPin        = FL_DISABLE;
    GPIO_InitStruct.analogSwitch    = FL_DISABLE;
    FL_GPIO_Init(GPIO_TRIGGER_PORT, &GPIO_InitStruct);

    FL_GPIO_SetTriggerEdge0(GPIO,
                            GPIO_TRIGGER_EXTI_LINE,
                            FL_GPIO_EXTI_TRIGGER_EDGE_DISABLE);
    FL_GPIO_SetExtiLine7(GPIO, FL_GPIO_EXTI_LINE_7_PB12);
    FL_GPIO_EnableDigitalFilter(GPIO, GPIO_TRIGGER_EXTI_LINE);
    FL_GPIO_SetTriggerEdge0(GPIO,
                            GPIO_TRIGGER_EXTI_LINE,
                            FL_GPIO_EXTI_TRIGGER_EDGE_FALLING);
    FL_GPIO_ClearFlag_EXTI(GPIO, GPIO_TRIGGER_EXTI_LINE);

    NVIC_DisableIRQ(GPIO_IRQn);
    NVIC_SetPriority(GPIO_IRQn, 2U);
    NVIC_EnableIRQ(GPIO_IRQn);
}
