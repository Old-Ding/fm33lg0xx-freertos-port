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
#include "uart.h"
#include <stdio.h>
#if 1
#pragma import(__use_no_semihosting)                          
    struct __FILE 
    { 
        int handle; 
    };
    FILE __stdout;       

    void _sys_exit(int x) 
    { 
        x = x;
    } 

    int fputc(int ch, FILE *f)
    {
        UART0->TXBUF = ch;
        while (!(UART0->ISR & (0x1 << 0)));
        UART0->ISR |= (0x1 << 0);
    
        return ch;
    }
#endif

/**
  * @brief  Debug输出串口配置
  * @param  void
  * @retval void
  */
void DebugInit(void)
{
    FL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    FL_UART_InitTypeDef UART_InitStruct = {0};  

    GPIO_InitStruct.pin = FL_GPIO_PIN_2|FL_GPIO_PIN_3;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);  

    UART_InitStruct.clockSrc = FL_CMU_UART1_CLK_SOURCE_APBCLK;
    UART_InitStruct.baudRate = 115200;
    UART_InitStruct.dataWidth = FL_UART_DATA_WIDTH_8B;
    UART_InitStruct.stopBits = FL_UART_STOP_BIT_WIDTH_1B;
    UART_InitStruct.parity = FL_UART_PARITY_EVEN;
    UART_InitStruct.transferDirection = FL_UART_DIRECTION_TX;       
    FL_UART_Init(UART0, &UART_InitStruct);    
}



