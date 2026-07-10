#include "arch/sys_arch.h"

uint32_t sys_now(void)
{
    return hdefaults_tick_get();
}


