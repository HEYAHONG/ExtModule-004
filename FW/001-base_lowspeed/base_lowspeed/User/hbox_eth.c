#include "hbox_eth.h"
#include "hbox_shell.h"
#include "lwip/init.h"
#include "lwip/timeouts.h"
#include "netif/ethernet.h"
#include "netif/etharp.h"
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "lwip/dhcp6.h"
#include "lwip/priv/tcp_priv.h"
#include "lwip/ethip6.h"
#include "lwip/apps/sntp.h"

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
    netif->name[0]='e';
    netif->name[1]='m';
    netif->output=etharp_output;
    netif->output_ip6=ethip6_output;
    netif->linkoutput=ethernetif_lowlevel_output;
    netif->hwaddr_len=ETHARP_HWADDR_LEN;
    netif->mtu=MAX_ETH_PAYLOAD;
    netif->mtu6=MAX_ETH_PAYLOAD;
    WCHNET_GetMacAddr(netif->hwaddr);
    netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP | NETIF_FLAG_ETHERNET | NETIF_FLAG_IGMP | NETIF_FLAG_MLD6;
#if LWIP_IPV6
    //创建IPV6 Linklocal地址（可用于没有分配到ip时的本地通信）
    netif_create_ip6_linklocal_address(netif,1);
#endif

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
#if     LWIP_IPV6_DHCP6
        if(ERR_OK==dhcp6_enable_stateless(&lwip_netif))
        {
            console_printf("eth:start dhcpv6(stateless)!");
        }
#endif
    }
    else
    {
        /*
         * 启动DHCP
         */
        dhcp_stop(&lwip_netif);
#if     LWIP_IPV6_DHCP6
        dhcp6_disable(&lwip_netif);
#endif
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


static const char *sntp_server_name[]=
{
    "ntp.hyhsystem.cn",
    "ntp.ntsc.ac.cn",
    "0.cn.pool.ntp.org",
    "0.pool.ntp.org",
    "1.cn.pool.ntp.org",
    "1.pool.ntp.org",
    "2.cn.pool.ntp.org",
    "2.pool.ntp.org",
    "3.cn.pool.ntp.org",
    "3.pool.ntp.org",
};
static void sntp_lowlevel_init(void)
{
#if SNTP_SERVER_DNS
    for(size_t i=0; i<SNTP_MAX_SERVERS; i++)
    {
        if(i<sizeof(sntp_server_name)/sizeof(sntp_server_name[0]))
        {
            sntp_setservername(i,sntp_server_name[i]);
        }
    }
#endif
    //启动sntp
    sntp_init();
}

static void  hbox_eth_init(const hruntime_function_t *func)
{
    lwip_lowlevel_init();
    sntp_lowlevel_init();
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
}
HRUNTIME_LOOP_EXPORT(eth,16,hbox_eth_loop,NULL);

static int cmd_ifconfig_entry(int argc,const char *argv[])
{
    hshell_context_t * hshell_ctx=hshell_context_get_from_main_argv(argc,argv);
    if(argc <= 1)
    {
        for(uint8_t index=1; index < 254; index++)
        {
            struct netif *interface=netif_get_by_index(index);
            if(interface!=NULL)
            {
                {
                    char ifname[NETIF_NAMESIZE]= {0};
                    netif_index_to_name(index,ifname);
#if LWIP_IPV6 && LWIP_ND6_ALLOW_RA_UPDATES
                    hshell_printf(hshell_ctx,"%s: flags=%d mtu %d mtu6 %d\r\n",ifname,(int)interface->flags,(int)interface->mtu,(int)interface->mtu6);
#else
                    hshell_printf(hshell_ctx,"%s: flags=%d mtu %d\r\n",(int)interface->flags,(int)interface->mtu);
#endif
                }
#if             LWIP_IPV4
                {
                    char ip_str[32]= {0};
                    ipaddr_ntoa_r(&interface->ip_addr,ip_str,sizeof(ip_str));
                    char gw_str[32]= {0};
                    ipaddr_ntoa_r(&interface->gw,gw_str,sizeof(gw_str));
                    char netmask_str[32]= {0};
                    ipaddr_ntoa_r(&interface->netmask,netmask_str,sizeof(netmask_str));
                    hshell_printf(hshell_ctx,"\tinet %s netmask %s gateway %s\r\n",ip_str,netmask_str,gw_str);
                }
#endif
#if             LWIP_IPV6
                {
                    for(size_t i=0; i<LWIP_IPV6_NUM_ADDRESSES; i++)
                    {
                        if(ip6_addr_isvalid(netif_ip6_addr_state(interface,i)))
                        {
                            {
                                char addr_str[96]= {0};
                                ipaddr_ntoa_r(&interface->ip6_addr[i],addr_str,sizeof(addr_str));
                                hshell_printf(hshell_ctx,"\tinet6 %s \r\n",addr_str);
                            }
                        }
                    }
                }
#endif
                hshell_printf(hshell_ctx,"\tether %02X:%02X:%02X:%02X:%02X:%02X \r\n",(int)interface->hwaddr[0],(int)interface->hwaddr[1],(int)interface->hwaddr[2],(int)interface->hwaddr[3],(int)interface->hwaddr[4],(int)interface->hwaddr[5]);
            }
        }
    }
    else
    {
        {
            struct netif *interface=netif_find(argv[1]);
            if(interface!=NULL)
            {
                {
                    char ifname[NETIF_NAMESIZE]= {0};
                    netif_index_to_name(netif_get_index(interface),ifname);
#if LWIP_IPV6 && LWIP_ND6_ALLOW_RA_UPDATES
                    hshell_printf(hshell_ctx,"%s: flags=%d mtu %d mtu6 %d\r\n",ifname,(int)interface->flags,(int)interface->mtu,(int)interface->mtu6);
#else
                    hshell_printf(hshell_ctx,"%s: flags=%d mtu %d\r\n",(int)interface->flags,(int)interface->mtu);
#endif
                }
#if             LWIP_IPV4
                {
                    char ip_str[32]= {0};
                    ipaddr_ntoa_r(&interface->ip_addr,ip_str,sizeof(ip_str));
                    char gw_str[32]= {0};
                    ipaddr_ntoa_r(&interface->gw,gw_str,sizeof(gw_str));
                    char netmask_str[32]= {0};
                    ipaddr_ntoa_r(&interface->netmask,netmask_str,sizeof(netmask_str));
                    hshell_printf(hshell_ctx,"\tinet %s netmask %s gateway %s\r\n",ip_str,netmask_str,gw_str);
                }
#endif
#if             LWIP_IPV6
                {
                    for(size_t i=0; i<LWIP_IPV6_NUM_ADDRESSES; i++)
                    {
                        if(ip6_addr_isvalid(netif_ip6_addr_state(interface,i)))
                        {
                            {
                                char addr_str[96]= {0};
                                ipaddr_ntoa_r(&interface->ip6_addr[i],addr_str,sizeof(addr_str));
                                hshell_printf(hshell_ctx,"\tinet6 %s \r\n",addr_str);
                            }
                        }
                    }
                }
#endif
                hshell_printf(hshell_ctx,"\tether %02X:%02X:%02X:%02X:%02X:%02X \r\n",(int)interface->hwaddr[0],(int)interface->hwaddr[1],(int)interface->hwaddr[2],(int)interface->hwaddr[3],(int)interface->hwaddr[4],(int)interface->hwaddr[5]);
            }
            else
            {
                hshell_printf(hshell_ctx,"No such device %s\r\n",argv[1]);
            }
        }

    }
    return 0;
};
HSHELL_COMMAND_EXPORT(ifconfig,cmd_ifconfig_entry,configure a network interface);
