#ifndef __HBOX_SHELL_H__
#define __HBOX_SHELL_H__
#include "ch32v20x.h"
#include "core_riscv.h"
#include "hbox.h"
#ifdef  __cplusplus
extern "C"
{
#endif

#define console_printf(fmt,...) hprintf("[%08X] " fmt "\r\n" ,hdefaults_tick_get(),##__VA_ARGS__)

void hbox_shell_console_input(uint8_t data);
bool hbox_shell_console_output(uint8_t* data);

#ifdef __cplusplus
}
#endif

#endif


