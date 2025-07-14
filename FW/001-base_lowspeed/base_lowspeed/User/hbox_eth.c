#include "hbox_eth.h"
#include "hbox_shell.h"
extern volatile uint8_t LinkSta;
bool hbox_eth_is_linked(void)
{
    return LinkSta!=0;
}

static hbox_eth_event_callback_t hbox_eth_event_cb=NULL;
void hbox_eth_set_callback(hbox_eth_event_callback_t cb)
{
    hbox_eth_event_cb=cb;
}

static volatile bool is_linked_state_change=false;
void __hbox_update_linked_state(void)
{
    is_linked_state_change=true;
}

static void hbox_update_linked_state(void)
{
    is_linked_state_change=false;
    console_printf("eth:%s",(hbox_eth_is_linked()?"up":"down"));
    /*
     * 以太网连接状态改变
     */
    hbox_eth_event_callback_t cb=hbox_eth_event_cb;
    if(cb!=NULL)
    {
        cb(HBOX_ETH_EVENT_LINKED_STATE_CHANGE);
    }
}

void __hbox_update_receive_data(const uint8_t* data,size_t data_length)
{
    hbox_eth_event_callback_t cb=hbox_eth_event_cb;
    if(cb!=NULL)
    {
        cb(HBOX_ETH_EVENT_RECEIVE_DATA,data,data_length);
    }
}

bool hbox_eth_transmit(const uint8_t *data,size_t data_length)
{
    return ETH_SUCCESS==MACRAW_Tx((uint8_t *)data,data_length);
}

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
    if(is_linked_state_change)
    {
        hbox_update_linked_state();
    }
    WCHNET_MainTask();
}
HRUNTIME_LOOP_EXPORT(eth,16,hbox_eth_loop,NULL);
