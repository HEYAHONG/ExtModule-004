#include "hbox_config.h"
#include "hbox.h"
#include "ch32v20x.h"
#include "core_riscv.h"

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
