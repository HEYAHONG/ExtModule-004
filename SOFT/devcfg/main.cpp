#include "HCPPBox.h"
#include "hrc.h"
#include H3RDPARTY_ARGTABLE3_HEADER
#include "string"
#include "vector"

#define UDP_DEVCFG_PORT 4

static int zone_id=0;
static void list_net_interfaces(void);
static void main_arg_parse(int argc,char *argv[])
{
    struct arg_lit  *nologo=NULL;
    struct arg_lit  *list_if=NULL;
    struct arg_int  *scope_id=NULL;
    void *argtable[]=
    {
        nologo=arg_lit0("N","nologo","hide logo"),
        list_if=arg_lit0("L","list","list net interfaces"),
        scope_id=arg_int0("Z","zone","scope_id","zone id"),
        arg_end(20)
    };
    if(arg_nullcheck(argtable)!=0)
    {
        hfputs("arg_nullcheck error!\r\n",stderr);
        hexit(-1);
    }

    if(arg_parse(argc,argv,argtable)>0)
    {
        hfputs("arg_parse error!\r\n",stderr);
        hfputs("Usage:\r\n",stderr);
        arg_print_glossary(stderr,argtable,"  %-25s %s\n");
        hexit(-1);
    }

    if(nologo->count==0)
    {
        {
            const uint8_t * banner=RCGetHandle("banner");
            if(banner!=NULL)
            {
                hprintf("%s",(const char *)banner);
            }
        }

    }

    if(list_if->count > 0)
    {
        list_net_interfaces();
        hexit(0);
    }

    if(scope_id->count>0)
    {
        zone_id=scope_id->ival[0];
    }
    else
    {
        hfputs("Zone must be specified!\r\n",stderr);
        hfputs("Usage:\r\n",stderr);
        arg_print_glossary(stderr,argtable,"  %-25s %s\n");
        hexit(-1);
    }

    arg_freetable(argtable,sizeof(argtable)/sizeof(argtable[0]));

}

#if defined(HDEFAULTS_OS_WINDOWS)
#include "iphlpapi.h"
#endif

static void list_net_interfaces(void)
{
#if defined(HDEFAULTS_OS_WINDOWS)
    {
        IP_ADAPTER_ADDRESSES * addr=new IP_ADAPTER_ADDRESSES[4096];
        ULONG addr_size=sizeof(IP_ADAPTER_ADDRESSES)*4096;
        if(ERROR_SUCCESS==GetAdaptersAddresses(AF_INET6,0,NULL,addr,&addr_size))
        {
            hprintf("-ZoneID-\t----Name----\r\n");
            PIP_ADAPTER_ADDRESSES current_addr=addr;
            while(current_addr!=NULL)
            {
                if((current_addr->IfType == IF_TYPE_ETHERNET_CSMACD || current_addr->IfType == IF_TYPE_IEEE80211) && current_addr->Ipv6Enabled)
                {
                    char ifname[4096]= {0};
                    if(hlocale_charset_is_utf8())
                    {
                        hunicode_char_t buff[4096]= {0};
                        hunicode_char_from_wchar_string(buff,(sizeof(buff)/sizeof(buff[0]))-1,current_addr->FriendlyName);
                        hunicode_char_string_to_utf8(ifname,sizeof(ifname)-1,buff);
                    }
                    else
                    {
                        hunicode_char_t buff[4096]= {0};
                        hunicode_char_from_wchar_string(buff,(sizeof(buff)/sizeof(buff[0]))-1,current_addr->FriendlyName);
                        hgb2312_string_from_unicode(ifname,sizeof(ifname)-1,buff,hunicode_char_string_length(buff));
                    }
                    hprintf("%-08d\t%s\r\n",current_addr->Ipv6IfIndex,ifname);
                }
                current_addr=current_addr->Next;
            }
        }
        delete [] addr;
    }
#endif
}


std::vector<HCPPSocketAddressIPV6> dev_list;

#define DEVCFG_HELLO_MESSAGE "hellodevcfg"
static void detect_device(void)
{
    HCPPSocketInit();
    int udp_fd=socket(AF_INET6,SOCK_DGRAM,0);
    if(udp_fd==INVALID_SOCKET)
    {
        hfputs("udp:socker error!\r\n",stderr);
        goto cleanup;
    }
    {
        //设定接收超时1s
        struct timeval  tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        setsockopt(udp_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));
    }
    {
        /*
         * 在本地链路广播消息
         */
        const char *buffer=DEVCFG_HELLO_MESSAGE;
        HCPPSocketAddressIPV6 boardcast_addr= {0};
        boardcast_addr.sin6_family=AF_INET6;
        inet_pton(AF_INET6,"FF02::1",&boardcast_addr.sin6_addr);
        boardcast_addr.sin6_port=htons(UDP_DEVCFG_PORT);
        boardcast_addr.sin6_scope_id=zone_id;
        size_t len=strlen(buffer);
        if(len!=sendto(udp_fd,buffer,len,0,(HCPPSocketAddress *)&boardcast_addr,sizeof(boardcast_addr)))
        {
            hfputs("udp:sendto error!\r\n",stderr);
            goto cleanup;
        }
    }
    {
        hdefaults_tick_t tick_start=hdefaults_tick_get();
        while(hdefaults_tick_get()-tick_start < 3000)
        {
            char buffer[4096]= {0};
            HCPPSocketAddressIPV6 addr= {0};
            socklen_t addr_len=sizeof(addr);
            if(recvfrom(udp_fd,buffer,sizeof(buffer)-1,0,(HCPPSocketAddress *)&addr,&addr_len) > 0)
            {
                if(addr_len==sizeof(addr) && strcmp(DEVCFG_HELLO_MESSAGE,buffer)==0)
                {
                    dev_list.push_back(addr);
                    char ip_str[256]= {0};
                    inet_ntop(AF_INET6,&addr.sin6_addr,ip_str,sizeof(ip_str));
                    hprintf("%s%%%d found\r\n",ip_str,addr.sin6_scope_id);
                }
            }
        }
    }

cleanup:
    if(udp_fd!=INVALID_SOCKET)
    {
        closesocket(udp_fd);
    }
}

int main(int argc,char *argv[])
{
    /*
     * 处理参数
     */
    main_arg_parse(argc,argv);

    /*
     * 检测设备
     */
    detect_device();



    return 0;
}
