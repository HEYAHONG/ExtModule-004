#include "hbox_eth.h"

static void  hbox_eth_init(const hruntime_function_t *func)
{
    uint8_t macaddr[6]= {0};
    WCHNET_GetMacAddr(macaddr);
    {
        char buff[32]= {0};
        hbase16_encode(buff,sizeof(buff),macaddr,sizeof(macaddr));
        console_printf("eth mac:%s",buff);
    }
    ETH_Init(macaddr);
}
HRUNTIME_INIT_EXPORT(eth,16,hbox_eth_init,NULL);

static void  hbox_eth_loop(const hruntime_function_t *func)
{
    WCHNET_MainTask();
}
HRUNTIME_LOOP_EXPORT(eth,16,hbox_eth_loop,NULL);
