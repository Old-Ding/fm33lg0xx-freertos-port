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
#include "gpio.h"
#include "adc.h"
#include "svd.h"
#include "iwdt.h"
#include "rmu.h"
#include "uart.h"
#include "taskadc.h"
#include "taskgpio.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#define MONITOR_TASK_STACK_WORDS       128U
#define GPIO_TASK_STACK_WORDS          192U
#define ADC_TASK_STACK_WORDS           192U
#define MONITOR_TASK_PRIORITY          ( tskIDLE_PRIORITY + 1U )
#define GPIO_TASK_PRIORITY             ( tskIDLE_PRIORITY + 2U )
#define ADC_TASK_PRIORITY              ( tskIDLE_PRIORITY + 2U )
#define MONITOR_INTERVAL_TICKS         pdMS_TO_TICKS(500U)
#define ADC_SEMAPHORE_MAX_COUNT        10U
#define ADC_SEMAPHORE_INITIAL_COUNT    0U
#define SVD_BUSY_DELAY_LOOP_CYCLES     4U

#define FREERTOS_FAULT_NONE            0U
#define FREERTOS_FAULT_MALLOC          1U
#define FREERTOS_FAULT_STACK           2U
#define FREERTOS_FAULT_SCHEDULER       3U
#define FREERTOS_FAULT_SYNC_OBJECT     4U
#define FREERTOS_FAULT_TASK_CREATE     5U
#define FREERTOS_FAULT_ASSERT          6U

volatile uint32_t g_freertosFaultCode = FREERTOS_FAULT_NONE;
volatile uint32_t g_monitorTaskLoopCount = 0U;
volatile BaseType_t g_monitorTaskCreateStatus = pdFALSE;
volatile BaseType_t g_gpioTaskCreateStatus = pdFALSE;
volatile BaseType_t g_adcTaskCreateStatus = pdFALSE;
volatile UBaseType_t g_monitorTaskStackHighWaterMark = 0U;
volatile TaskHandle_t g_stackOverflowTaskHandle = NULL;
char * volatile g_stackOverflowTaskName = NULL;
const char * volatile g_freertosAssertFile = NULL;
volatile uint32_t g_freertosAssertLine = 0U;

SemaphoreHandle_t g_gpioSemaphore = NULL;
SemaphoreHandle_t g_adcSemaphore = NULL;

static void SVD_BusyDelayUsNoSysTick(uint32_t u32Delay_us);
static void MonitorTask(void *pvParameters);
static BaseType_t CreateDemoSyncObjects(void);
static BaseType_t CreateDemoTasks(void);

/**
  * @brief  HardFault 中断服务函数 请保留 
  * @param  void
  * @retval None
  */
void HardFault_Handler(void)
{
    /* 软复位MCU */
    FL_RMU_SetSoftReset(RMU);
}

/**
  * @brief  掉电监控，用于执行掉电事件
  * @param  
  * @retval 
  */
static void PowerDownMonitroing(void)
{
    /* 确认SVD监测结果是否低于阈值 */
    if (true == SVD_Result_Confirmed(SVD_BELOW_THRESHOLD, 1U))
    {   /* 防抖处理 */
        if (true == SVD_Result_Confirmed(SVD_BELOW_THRESHOLD, 2000U/*us*/))
        {
            /* 电压下降到报警阈值，处理掉电事件 */
            /* 用户程序1 */
            /* ...... */
            /* 注意：用户需确保在BOR复位之前执行完毕 */
            
            /* 确认SVD监测结果是否高于阈值，如否则持续等待 */
            while(false == SVD_Result_Confirmed(SVD_HIGHER_THRESHOLD, 20U/*us*/));
            /* 由于电源有下降到报警阈值，软件复位后重新初始化 */
            FL_RMU_SetSoftReset(RMU);
        }
    }
}

static void SVD_BusyDelayUsNoSysTick(uint32_t u32Delay_us)
{
    volatile uint32_t u32LoopCount;
    uint32_t u32ClocksPerUs;

    u32ClocksPerUs = SystemCoreClock / 1000000U;
    if(0U == u32ClocksPerUs)
    {
        u32ClocksPerUs = 1U;
    }

    u32LoopCount = (u32ClocksPerUs * u32Delay_us) / SVD_BUSY_DELAY_LOOP_CYCLES;
    if(0U == u32LoopCount)
    {
        u32LoopCount = u32Delay_us;
    }

    while(u32LoopCount > 0U)
    {
        __NOP();
        u32LoopCount--;
    }
}

void SVD_DelayUs(uint32_t u32Delay_us)
{
    if(taskSCHEDULER_NOT_STARTED == xTaskGetSchedulerState())
    {
        FL_DelayUs(u32Delay_us);
    }
    else
    {
        /* 调度器运行后 SysTick 归 FreeRTOS，SVD 去抖只能用不改 SysTick 的短延时。 */
        SVD_BusyDelayUsNoSysTick(u32Delay_us);
    }
}

static void MonitorTask(void *pvParameters)
{
    (void)pvParameters;

    while(1)
    {
        IWDT_Clr();
        PowerDownMonitroing();
        LED0_TOG();
        g_monitorTaskLoopCount++;
        g_monitorTaskStackHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
        vTaskDelay(MONITOR_INTERVAL_TICKS);
    }
}

static BaseType_t CreateDemoSyncObjects(void)
{
    g_gpioSemaphore = xSemaphoreCreateBinary();
    g_adcSemaphore = xSemaphoreCreateCounting(ADC_SEMAPHORE_MAX_COUNT,
                                              ADC_SEMAPHORE_INITIAL_COUNT);

    if((NULL == g_gpioSemaphore) || (NULL == g_adcSemaphore))
    {
        g_freertosFaultCode = FREERTOS_FAULT_SYNC_OBJECT;
        return pdFAIL;
    }

    return pdPASS;
}

static BaseType_t CreateDemoTasks(void)
{
    g_monitorTaskCreateStatus = xTaskCreate(MonitorTask,
                                            "monitor",
                                            MONITOR_TASK_STACK_WORDS,
                                            NULL,
                                            MONITOR_TASK_PRIORITY,
                                            NULL);

    g_gpioTaskCreateStatus = xTaskCreate(GPIOTask,
                                         "gpio_evt",
                                         GPIO_TASK_STACK_WORDS,
                                         NULL,
                                         GPIO_TASK_PRIORITY,
                                         NULL);

    g_adcTaskCreateStatus = xTaskCreate(AdcTask,
                                        "adc_sample",
                                        ADC_TASK_STACK_WORDS,
                                        NULL,
                                        ADC_TASK_PRIORITY,
                                        NULL);

    if((pdPASS != g_monitorTaskCreateStatus) ||
       (pdPASS != g_gpioTaskCreateStatus) ||
       (pdPASS != g_adcTaskCreateStatus))
    {
        g_freertosFaultCode = FREERTOS_FAULT_TASK_CREATE;
        return pdFAIL;
    }

    return pdPASS;
}

void vApplicationMallocFailedHook(void)
{
    g_freertosFaultCode = FREERTOS_FAULT_MALLOC;
    taskDISABLE_INTERRUPTS();

    while(1)
    {
    }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    g_stackOverflowTaskHandle = xTask;
    g_stackOverflowTaskName = pcTaskName;
    g_freertosFaultCode = FREERTOS_FAULT_STACK;
    taskDISABLE_INTERRUPTS();

    while(1)
    {
    }
}

void FreeRTOS_AssertFailed(const char *pcFile, uint32_t ulLine)
{
    g_freertosAssertFile = pcFile;
    g_freertosAssertLine = ulLine;
    g_freertosFaultCode = FREERTOS_FAULT_ASSERT;
    __disable_irq();

    while(1)
    {
    }
}

int main(void)
{
    /* 使能IWDT */
    IWDT_Init(FL_IWDT_PERIOD_2000MS);

    /* 延时函数初始化 */
    FL_Init();

    /* 使能SVD,阈值4.16V(falling)~4.284V(rising) */
    SVD_Init(SVD_MONTIOR_VDD,FL_SVD_WARNING_THRESHOLD_GROUP11,FL_SVD_REFERENCE_1P0V);

    /* 确认SVD监测结果是否高于阈值，如否则持续等待 */
    while(false == SVD_Result_Confirmed(SVD_HIGHER_THRESHOLD, 2000U/*us*/));
    
    /* 使能BOR为2.0V */
    RMU_BOR_Init(FL_RMU_BOR_THRESHOLD_2P00V);

    /* RTC调校初值配置 */
    MF_Clock_Init();

    /* LED初始化 */
    GPIO_Init();

    /* ADC和串口先完成硬件初始化，任务启动后只处理事件和观测输出。 */
    ADC_Init();
    DebugInit();

    if((pdPASS == CreateDemoSyncObjects()) && (pdPASS == CreateDemoTasks()))
    {
        /* 信号量和任务已经就绪后再开外部中断，ISR只负责释放事件。 */
        GPIO_InterruptInit();
        printf("FM33LG0xx FreeRTOS signal ADC UART demo start\r\n");
        vTaskStartScheduler();
    }

    if(FREERTOS_FAULT_NONE == g_freertosFaultCode)
    {
        g_freertosFaultCode = FREERTOS_FAULT_SCHEDULER;
    }

    while(1)
    {
    }
}


