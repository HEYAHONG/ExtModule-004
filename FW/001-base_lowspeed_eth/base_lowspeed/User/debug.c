#include "debug.h"

void Delay_Us(size_t delay_count)
{
    __O size_t count=(SystemCoreClock/1000000)*delay_count;
    while(count--);
}

void Delay_Ms(size_t delay_count)
{
    hdefaults_tick_t tick_start=hdefaults_tick_get();
    while(hdefaults_tick_get()-tick_start < delay_count);
}


