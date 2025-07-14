#include "hbox_eth.h"
#include "hbox_shell.h"
#include "lwip/init.h"
#include "lwip/timeouts.h"
#include "netif/ethernet.h"
#include "netif/etharp.h"
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "lwip/priv/tcp_priv.h"
#include "lwip/ethip6.h"

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

static err_t ethernetif_lowlevel_output(struct netif *netif,struct pbuf *p)
{
    if(!hbox_eth_is_linked())
    {
        return ERR_ISCONN;
    }
    for(struct pbuf *q=p; q!=NULL; q=q->next)
    {
        hdefaults_tick_t tick_start=hdefaults_tick_get();
        bool is_ok=false;
        while(hdefaults_tick_get()-tick_start < 3000)
        {
            if((is_ok=hbox_eth_transmit(q->payload,q->len)))
            {
                break;
            }
        }
        if(!is_ok)
        {
            return ERR_TIMEOUT;
        }

        /*
         * 等待发送完成
         */
        while( DMATxDescToSet->Status & ETH_DMATxDesc_OWN )
        {

        }
    }
    return ERR_OK;
}

static err_t ethernetif_init(struct netif *netif)
{
    if(netif==NULL)
    {
        return ERR_VAL;
    }

#if LWIP_NETIF_HOSTNAME
    netif->hostname="ExtModule-004";
#endif
    netif->name[0]='E';
    netif->name[1]='M';
    netif->output=etharp_output;
    netif->output_ip6=ethip6_output;
    netif->linkoutput=ethernetif_lowlevel_output;
    netif->hwaddr_len=ETHARP_HWADDR_LEN;
    netif->mtu=ETH_MAX_PACKET_SIZE;
    netif->mtu6=ETH_MAX_PACKET_SIZE;
    WCHNET_GetMacAddr(netif->hwaddr);
    netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

    /*
     * 初始化以太网
     */
    {
        char buff[32]= {0};
        hbase16_encode(buff,sizeof(buff),netif->hwaddr,ETHARP_HWADDR_LEN);
        console_printf("eth mac:%s",buff);
    }
    ETH_Init(netif->hwaddr);

    return ERR_OK;
}

static struct netif lwip_netif= {0};
static void lwip_lowlevel_init(void)
{
    lwip_init();
    struct ip4_addr ipaddr= {0};
    struct ip4_addr gw= {0};
    struct ip4_addr netmask= {0};
    if(netif_add(&lwip_netif,&ipaddr,&netmask,&gw,NULL,ethernetif_init,ethernet_input)!=NULL)
    {
        netif_set_default(&lwip_netif);
    }

}

static void hbox_update_linked_state(void)
{
    is_linked_state_change=false;
    console_printf("eth:%s",(hbox_eth_is_linked()?"up":"down"));
    if(hbox_eth_is_linked())
    {
        netif_set_up(&lwip_netif);
        if(ERR_OK==dhcp_start(&lwip_netif))
        {
            console_printf("eth:start dhcp!");
        }
    }
    else
    {
        /*
         * 启动DHCP
         */
        dhcp_stop(&lwip_netif);
        netif_set_down(&lwip_netif);
    }
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
    if(lwip_netif.input!=NULL)
    {
        struct pbuf *p=pbuf_alloc(PBUF_RAW,data_length,PBUF_POOL);
        if(p!=NULL)
        {
            size_t index=0;
            for(struct pbuf *q=p; q!=NULL; q=q->next)
            {
                memcpy(q->payload,&data[index],q->len);
                index+=q->len;
            }
        }
        if(ERR_OK!=lwip_netif.input(p,&lwip_netif))
        {
            pbuf_free(p);
        }
    }
}

bool hbox_eth_transmit(const uint8_t *data,size_t data_length)
{
    return ETH_SUCCESS==MACRAW_Tx((uint8_t *)data,data_length);
}

static void lwip_net_info(void)
{
    static bool is_dhcp_ok=false;
    if(netif_is_up(&lwip_netif))
    {
        //打印DHCP IP
        bool is_dhcp_ok_temp=(0!=dhcp_supplied_address(&lwip_netif));
        if(is_dhcp_ok_temp!=is_dhcp_ok && is_dhcp_ok_temp==true)
        {
            is_dhcp_ok=is_dhcp_ok_temp;
            {
                char buff[32]= {0};
                ipaddr_ntoa_r(&lwip_netif.ip_addr,buff,sizeof(buff));
                console_printf("eth:ip=%s",buff);
            }
            {
                char buff[32]= {0};
                ipaddr_ntoa_r(&lwip_netif.gw,buff,sizeof(buff));
                console_printf("eth:gateway=%s",buff);
            }
            {
                char buff[32]= {0};
                ipaddr_ntoa_r(&lwip_netif.netmask,buff,sizeof(buff));
                console_printf("eth:netmask=%s",buff);
            }
        }
    }
    else
    {
        is_dhcp_ok=false;
    }
}

static void  hbox_eth_init(const hruntime_function_t *func)
{
    lwip_lowlevel_init();
}
HRUNTIME_INIT_EXPORT(eth,16,hbox_eth_init,NULL);

static void  hbox_eth_loop(const hruntime_function_t *func)
{
    if(is_linked_state_change)
    {
        hbox_update_linked_state();
    }
    WCHNET_MainTask();
    sys_check_timeouts();
    lwip_net_info();
}
HRUNTIME_LOOP_EXPORT(eth,16,hbox_eth_loop,NULL);
