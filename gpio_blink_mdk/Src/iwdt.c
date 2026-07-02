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
#include "main.h"

/**
  * @brief  IWDT初始化
  * @param  void
  * @retval FL_FAIL: 初始化失败
  *         FL_PASS: 初始化成功
  */
FL_ErrorStatus IWDT_Init(uint32_t u32OverFlowPeriod)
{
    FL_IWDT_InitTypeDef IWDT_InitStruct;

    /* 默认不使用窗口 */
    IWDT_InitStruct.iwdtWindows    = 0;

    /*最长溢出时间*/
    IWDT_InitStruct.overflowPeriod = u32OverFlowPeriod;

    return FL_IWDT_Init(IWDT, &IWDT_InitStruct);
}


/**
  * @brief  IWDT清除计数
  * @param  void
  * @retval void
  */
void IWDT_Clr(void)
{
    FL_IWDT_ReloadCounter(IWDT);
}

/**
  * @brief  IWDT休眠下是否计数
  * @param  eIWDTCounterType：FL_DISABLE: 休眠下不冻结计数
  *                           FL_ENABLE : 休眠下冻结计数
  * @retval void
  */
void IWDT_SLP_Freeze(bool eIWDTCounterType)
{
    if(FL_DISABLE == eIWDTCounterType)
    {   /* 休眠下不冻结IWDT计数 */
        FL_IWDT_DisableFreezeWhileSleep(IWDT);
    }
    else if(FL_ENABLE == eIWDTCounterType)
    {   /* 休眠下冻结IWDT计数 */
        FL_IWDT_EnableFreezeWhileSleep(IWDT);
    }

}
