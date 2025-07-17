#include "hbox_config.h"
#include "hbox.h"
#include "ch32v20x.h"
#include "core_riscv.h"
#include "time.h"

void hbox_tick_init (void)
{
    SysTick->SR = 0;
    SysTick->CNT = 0;
    SystemCoreClockUpdate();
    /*
     * 毫秒定时器
     */
    SysTick->CMP = (SystemCoreClock/1000)-1;
    SysTick->CTLR =0xF;

    NVIC_SetPriority(SysTicK_IRQn, 1);
    NVIC_EnableIRQ(SysTicK_IRQn);

}

static hdefaults_tick_t current_tick = 0;

hdefaults_tick_t hbox_tick_get (void)
{
    return current_tick;
}

void hbox_tick_inc (void)
{
    SysTick->SR = 0;
    SysTick->CTLR =0xF;
    current_tick++;
}

static int hbox_critical_cnt = 0;

void hbox_enter_critical()
{
    if (hbox_critical_cnt == 0)
    {
        /*
         * TODO:实现临界区,开关全局中断(MIE)将导致异常，暂时无法使用开关中断实现临界区
         * 临时解决方法为对有可能使用到全局锁的中断在此处开关中断(如软件中断等)
         */
    }
    hbox_critical_cnt++;
}

void hbox_exit_critical()
{
    hbox_critical_cnt--;
    if (hbox_critical_cnt == 0)
    {
        /*
         * TODO:实现临界区,开关全局中断(MIE)将导致异常,暂时无法使用开关中断实现临界区
         * 临时解决方法为对有可能使用到全局锁的中断在此处开关中断(如软件中断等)
         */


    }
}

void *hbox_malloc (size_t bytes)
{
    hbox_enter_critical();
    void *ptr = malloc (bytes);
    hbox_exit_critical();
    return ptr;
}

void hbox_free (void *ptr)
{
    hbox_enter_critical();
    free (ptr);
    hbox_exit_critical();
}

static int hbox_version_entry (int argc, const char *argv[])
{
    hshell_context_t *hshell_ctx = hshell_context_get_from_main_argv (argc, argv);
    hshell_printf (hshell_ctx, "version=0.0.0.1(ChipID:%08x)\r\n",DBGMCU_GetCHIPID());
    return 0;
}

HSHELL_COMMAND_EXPORT (hbox_version, hbox_version_entry, show hbox_version);


static int  cmd_reboot(int argc,const char *argv[])
{
    NVIC_SystemReset();
    return 0;
}

HSHELL_COMMAND_EXPORT(reboot,cmd_reboot,reboot system);

/*
 * 当前时间
 */
time_t time(time_t *timer)
{
    hgettimeofday_timeval_t tv= {0};
    hgettimeofday(&tv,NULL);
    if(timer!=NULL)
    {
        (*timer)=tv.tv_sec;
    }
    return tv.tv_sec;
}

static int cmd_datetime_entry(int argc,const char *argv[])
{
    hshell_context_t * hshell_ctx=hshell_context_get_from_main_argv(argc,argv);
    time_t time_now=time(NULL);
    const char *TZ="";
    {
        const char *tz_str=hgetenv("TZ");
        if(tz_str!=NULL)
        {
            TZ=tz_str;
        }
    }
    hshell_printf(hshell_ctx,"%s %s",TZ,asctime(localtime(&time_now)));
    return 0;
};
HSHELL_COMMAND_EXPORT(datetime,cmd_datetime_entry,show datetime);

/*
 * 设置当前时间
 */
void hbox_set_time(time_t new_time)
{
    hgettimeofday_timeval_t tv= {0};
    tv.tv_sec=new_time;
    hsettimeofday(&tv,NULL);
    RTC_SetCounter(new_time);
}

static int cmd_set_datetime_entry(int argc,const char *argv[])
{
    hshell_context_t * hshell_ctx=hshell_context_get_from_main_argv(argc,argv);
    if(argc <=1)
    {
        hshell_printf(hshell_ctx,"set_datetime [year] [month] [day] [hour] [minute] [second]\r\n");
    }
    else
    {
        time_t time_now=time(NULL);
        struct tm time_now_struct= {0};
        localtime_r(&time_now,&time_now_struct);
        if(argc >= 2)
        {
            time_now_struct.tm_year=atoi(argv[1])-1900;
        }
        if(argc >= 3)
        {
            time_now_struct.tm_mon=atoi(argv[2])-1;
        }
        if(argc >= 4)
        {
            time_now_struct.tm_mday=atoi(argv[3]);
        }
        if(argc >= 5)
        {
            time_now_struct.tm_hour=atoi(argv[4]);
        }
        if(argc >= 6)
        {
            time_now_struct.tm_min=atoi(argv[5]);
        }
        if(argc >= 7)
        {
            time_now_struct.tm_sec=atoi(argv[6]);
        }
        time_now=mktime(&time_now_struct);
        hbox_set_time(time_now);
    }
    return 0;
};
HSHELL_COMMAND_EXPORT(set_datetime,cmd_set_datetime_entry,set datetime.);

#include "malloc.h"
static int cmd_free_entry(int argc,const char *argv[])
{
    hshell_context_t * hshell_ctx=hshell_context_get_from_main_argv(argc,argv);
    struct mallinfo info=mallinfo();
    size_t system_total=0;
    {
        extern char _end[];
        extern char _heap_end[];
        system_total=(uintptr_t)_heap_end-(uintptr_t)_end;
    }
    size_t free_size=(system_total-info.arena)+info.fordblks;
    size_t max_used_size=info.arena;
    size_t used_size=info.uordblks;
    size_t total_size=free_size+used_size;
    hshell_printf(hshell_ctx,"total:%d bytes,max_used:%d bytes,used:%d bytes,free:%d bytes\r\n",total_size,max_used_size,used_size,free_size);
    return 0;
};
HSHELL_COMMAND_EXPORT(free,cmd_free_entry,show memory info);


