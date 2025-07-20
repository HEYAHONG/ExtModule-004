#include "udp_devcfg.h"
#include "ch32v20x.h"
#include "hbox.h"
#include "hbox_shell.h"
#include "lwip/ip.h"
#include "lwip/udp.h"
#include "lwip/ip_addr.h"


struct udp_pcb * devcfg_pcb=NULL;
void udp_devcfg_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    if(pcb==NULL || p==NULL || addr==NULL)
    {
        if(p!=NULL)
        {
            pbuf_free(p);
        }
    }

    size_t length=p->tot_len;
    uint8_t *data=(uint8_t *)p->payload;
    if(length > 0 && data!=NULL)
    {
        {
            /*
             * 显示接收的数据包信息
             */
            char header[16+1]= {0};
            size_t header_length=length;
            if(header_length>8)
            {
                header_length=8;
            }
            hbase16_encode(header,sizeof(header)-1,data,header_length);
            char ip_str[96]= {0};
            ipaddr_ntoa_r(addr,ip_str,sizeof(ip_str)-1);
            console_printf("devcfg:addr=%s,port=%d,length=%d,header=%s",ip_str,(int)port,(int)length,header);
        }
        /*
         * 将请求发送回去
         */
        udp_sendto(pcb,p,addr,port);
    }

    /*
     * 释放数据包
     */
    pbuf_free(p);
}
static void  hdevcfg_init(const hruntime_function_t *func)
{
    devcfg_pcb=udp_new_ip_type(IPADDR_TYPE_ANY);
    if(devcfg_pcb==NULL)
    {
        console_printf("devcfg:udp_new failed!");
        return;
    }
    /*
     * 绑定端口号
     */
    ip_addr_t addr;
    ip_addr_set_any(true,&addr);
    if(udp_bind(devcfg_pcb,&addr,UDP_DEVCFG_PORT)!=ERR_OK)
    {
        console_printf("devcfg:udp_bind failed!");
        udp_remove(devcfg_pcb);
        devcfg_pcb=NULL;
        return;
    }
    /*
     * 设定接收回调
     */
    udp_recv(devcfg_pcb,udp_devcfg_recv,NULL);
    console_printf("devcfg:init ok!");
}
HRUNTIME_INIT_EXPORT(devcfg,24,hdevcfg_init,NULL);


static void  hdevcfg_loop(const hruntime_function_t *func)
{

}
HRUNTIME_LOOP_EXPORT(devcfg,24,hdevcfg_loop,NULL);


