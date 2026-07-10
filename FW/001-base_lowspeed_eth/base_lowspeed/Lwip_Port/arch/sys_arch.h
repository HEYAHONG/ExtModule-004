#ifndef __LWIP_SYS_ARCH_H__
#define __LWIP_SYS_ARCH_H__

#include "ch32v20x.h"
#include "hbox.h"
#include "hbox_shell.h"

#define SYS_MBOX_NULL 0
#define SYS_SEM_NULL  0

#define LWIP_RAND  rand

uint32_t sys_now(void);

/*
 * 用于sntp
 */
#define SNTP_SERVER_DNS 1
#define SNTP_GET_SYSTEM_TIME(sec, us) do {  hgettimeofday_timeval_t tv= {0}; hgettimeofday(&tv,NULL); (sec)=tv.tv_sec; (us)=tv.tv_usec;} while(0)
#define SNTP_SET_SYSTEM_TIME(sec) do { hbox_set_time(sec); console_printf("ntp:set system time!"); } while(0)

#endif


