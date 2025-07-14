#ifndef __LWIP_SYS_ARCH_H__
#define __LWIP_SYS_ARCH_H__

#include "ch32v20x.h"
#include "hbox.h"

#define SYS_MBOX_NULL 0
#define SYS_SEM_NULL  0

#define LWIP_RAND  rand

uint32_t sys_now(void);

#endif


