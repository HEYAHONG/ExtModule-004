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
     * ���붨ʱ��
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
    current_tick++;
}

static int hbox_critical_cnt = 0;

void hbox_enter_critical()
{
    if (hbox_critical_cnt == 0)
    {
        /*
         * �ر�MIE
         */
        uint32_t mstatus = __get_MSTATUS();
        mstatus &= ~((1UL << 3));
        __set_MSTATUS (mstatus);
    }
    hbox_critical_cnt++;
}

void hbox_exit_critical()
{
    hbox_critical_cnt--;
    if (hbox_critical_cnt == 0)
    {
        /*
         * ��MIE
         */
        uint32_t mstatus = __get_MSTATUS();
        mstatus |= ((1UL << 3));
        __set_MSTATUS (mstatus);
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
    hshell_printf (hshell_ctx, "0.0.0.1\r\n");
    return 0;
}

HSHELL_COMMAND_EXPORT (hbox_version, hbox_version_entry, show hbox_version);
