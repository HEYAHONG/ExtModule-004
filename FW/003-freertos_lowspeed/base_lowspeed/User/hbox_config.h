#ifndef __HBOX_CONFIG_H__
#define __HBOX_CONFIG_H__

/*
 * 启用FreeRTOS内核
 */
#define FREERTOS_KERNEL              1
#define FREERTOS_KERNEL_MEMMANG_HEAP 3 

/*
 * 不使用原子支持
 */
#define HCPPRT_NO_ATOMIC 1

/*
 * 启用C++初始化
 */
#define HCPPRT_USE_CTORS 1


/*
 * 启用初始化段
 */
#define HRUNTIME_USING_INIT_SECTION 1

/*
 * 启用循环段
 */
#define HRUNTIME_USING_LOOP_SECTION 1

/*
 * 启用符号段
 */
#define HRUNTIME_USING_SYMBOL_SECTION 1


#ifdef __cplusplus
extern "C" {
#endif
#include "time.h"

void hbox_set_time(time_t new_time);

#ifdef __cplusplus
}
#endif

/*
 * CH32V208支持96位唯一编码，可分为3个32位无符号数
 */
#ifndef HBOX_HW_UID_BASE
#define HBOX_HW_UID_BASE        ((uint8_t *)0x1FFFF7E8)
#define HBOX_HW_UID_LENGTH      (12)
#endif

#endif  // __HBOX_CONFIG_H__
