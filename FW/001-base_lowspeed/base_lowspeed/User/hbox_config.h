#ifndef __HBOX_CONFIG_H__
#define __HBOX_CONFIG_H__

#define HDEFAULTS_TICK_GET hbox_tick_get
#define HDEFAULTS_MUTEX_LOCK hbox_enter_critical
#define HDEFAULTS_MUTEX_UNLOCK hbox_exit_critical
#define HDEFAULTS_MALLOC hbox_malloc
#define HDEFAULTS_FREE hbox_free

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


/*
 * 使用自定义的putchar
 */
#define HPUTCHAR hbox_shell_putchar

/*
 * 使用自定义的getchar
 */
#define HGETCHAR hbox_shell_getchar


#ifdef __cplusplus
extern "C" {
#endif
#include "time.h"

void hbox_tick_init (void);
void hbox_tick_inc (void);
void hbox_set_time(time_t new_time);

#ifdef __cplusplus
}
#endif

#endif  // __HBOX_CONFIG_H__
