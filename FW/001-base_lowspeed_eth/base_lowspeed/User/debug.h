#ifndef __DEBUG_H__
#define __DEBUG_H__
#include "ch32v20x.h"
#include "core_riscv.h"
#include "hbox.h"
#include "hbox_shell.h"

/*
 * 此文件用于兼容原厂驱动,尽量不要使用本文件中的函数
 */

/*
 * 使用hprintf 代替printf
 */
#ifdef printf
#undef printf
#endif
#define printf hprintf

void Delay_Us(size_t delay_count);
void Delay_Ms(size_t delay_count);

#endif
