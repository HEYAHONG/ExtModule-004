/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2021/06/06
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#include "debug.h"

/* Global typedef */

/* Global define */

/* Global Variable */

/*
 * 全局环境变量
 */
static char *global_environ[]=
{
    /*
     * 语言设置
     */
    (char *)"LANG=zh_CN.UTF-8",
    /*
     * 时区设置(调用tzset生效,设置完成后，C库的相关时间函数的时区即生效)
     */
    (char *)"TZ=CST-8:00:00",
    NULL
};

/*
 * 设定环境变量指针
 */
char **environ=global_environ;

/*
 * 栈溢出钩子
 */
void vApplicationStackOverflowHook( TaskHandle_t xTask,char * pcTaskName )
{
    while(true);
}

/*
 * 空闲钩子
 */
void vApplicationIdleHook( void )
{
    if(hwatchdog_is_valid())
    {
        HWATCHDOG_FEED();
    }
    else
    {
        hruntime_loop_enable_softwatchdog(false);
    }
}

/*
 * hbox任务
 */
static void hbox_task_entry(void *usr)
{
    hcpprt_init();
    while(1)
    {
        hcpprt_loop();
        vTaskDelay(1);
    }    
}

/*
 * 主任务
 */
static void main_task_entry(void *usr)
{
    while(true)
    {
        vTaskDelay(1);
    }
}

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
    {
        /*
         * 启用Flash增强模式,若要进行Flash操作，请先关闭增强模式
         */
        FLASH_Unlock();
        FLASH_Unlock_Fast();
        FLASH_Enhance_Mode(ENABLE);
        FLASH_Lock_Fast();
        FLASH_Lock();
    }
    NVIC_PriorityGroupConfig (NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();

    {
        {
            /*
             * 初始化时间为2020-01-01 00:00:00
             * 主要用于初始化随机数
             */
            hsettimeofday_timeval_t tv= {0};
            tv.tv_sec=1577808000;
            hsettimeofday(&tv,NULL);
        }
        /*
         * 初始化随机数(由于CH32V208没有真随机数发生器（TRNG）,采用初始化伪随机数种子)
         */
        uint8_t temp[4]= {0};
        hgetrandom(temp,sizeof(temp),0); //读取一次随机数,进行内部初始化
        srand(hcrc_crc32_fast_calculate((const uint8_t *)HBOX_HW_UID_BASE,HBOX_HW_UID_LENGTH)); //初始化随机数种子
    }
    //设置时区
    tzset();
    
    hruntime_init_lowlevel();

    xTaskCreate( hbox_task_entry, "hbox_task",4096/sizeof(StackType_t), NULL, 1, NULL );

    xTaskCreate( main_task_entry, "main_task",4096/sizeof(StackType_t), NULL, 2, NULL );

    vTaskStartScheduler();

    /*
     * 一般不会到这里
     */
    while(true);
    
}

/*********************************************************************
 * @fn      _sbrk
 *
 * @brief   Change the spatial position of data segment.
 *
 * @return  size: Data length
 */
__attribute__ ((used)) void *_sbrk (ptrdiff_t incr)
{
    extern char _end[];
    extern char _heap_end[];
    static char *curbrk = _end;

    if ((curbrk + incr < _end) || (curbrk + incr > _heap_end))
        return NULL - 1;

    curbrk += incr;
    return curbrk - incr;
}

