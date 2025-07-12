#include "ch32v20x.h"
#include "core_riscv.h"
#include "hbox.h"
#include "hbox_shell.h"

/*
 * RTC最小时间(2025-01-01 08:00:00 UTC+8:00)
 */
#define RTC_MIN_COUNTER 1735689600

static void rtc_init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    PWR_BackupAccessCmd(ENABLE);
    RCC_LSEConfig(RCC_LSE_ON);
    {
        hdefaults_tick_t start_tick=hdefaults_tick_get();
        while(hdefaults_tick_get()-start_tick < 100)
        {
            if(RCC_GetFlagStatus(RCC_FLAG_LSERDY)==SET)
            {
                break;
            }
        }
    }
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
    RCC_RTCCLKCmd(ENABLE);
    RTC_WaitForLastTask();
    RTC_WaitForSynchro();
    if(RTC_GetCounter() < RTC_MIN_COUNTER)
    {
        /*
         * 第一次初始化
         */
        RTC_WaitForLastTask();
        RTC_WaitForSynchro();
        RTC_EnterConfigMode();
        RTC_SetPrescaler(32767); /*32.768k时钟，预分频值=计数值(32768)-1*/
        RTC_WaitForLastTask();
        RTC_SetCounter(RTC_MIN_COUNTER);
        RTC_ExitConfigMode();
    }

    {
        /*
         * 利用RTC设置初始事件
         * 由于RTC只能到秒（虽然可以设置其它预分频，但寄存器只有32位），因此hgettimeofday内仍然采用默认实现。
         * 对于需要准确时间的情况，应当直接使用RTC硬件相关函数
         */
        hsettimeofday_timeval_t tv= {0};
        tv.tv_sec=RTC_GetCounter();
        hsettimeofday(&tv,NULL);
    }
}



static void  hbox_port_init(const hruntime_function_t *func)
{
    rtc_init();
}
HRUNTIME_INIT_EXPORT(port,0,hbox_port_init,NULL);

static void  hbox_port_loop(const hruntime_function_t *func)
{

}
HRUNTIME_LOOP_EXPORT(port,0,hbox_port_loop,NULL);
