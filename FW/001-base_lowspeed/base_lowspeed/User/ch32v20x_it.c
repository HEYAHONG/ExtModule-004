/********************************** (C) COPYRIGHT *******************************
 * File Name          : ch32v20x_it.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2023/12/29
 * Description        : Main Interrupt Service Routines.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#include "ch32v20x_it.h"
#include "hbox.h"
#include "hbox_shell.h"

void NMI_Handler (void) __attribute__ ((interrupt ("WCH-Interrupt-fast")));
void HardFault_Handler (void) __attribute__ ((interrupt ("WCH-Interrupt-fast")));
void SysTick_Handler (void) __attribute__ ((interrupt ("WCH-Interrupt-fast")));
void SW_Handler(void) __attribute__ ((interrupt ("WCH-Interrupt-fast")));
void USART2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

/*********************************************************************
 * @fn      NMI_Handler
 *
 * @brief   This function handles NMI exception.
 *
 * @return  none
 */
void NMI_Handler (void)
{
    while (1)
    {
    }
}

/*********************************************************************
 * @fn      HardFault_Handler
 *
 * @brief   This function handles Hard Fault exception.
 *
 * @return  none
 */
void HardFault_Handler (void)
{
    NVIC_SystemReset();
    while (1)
    {
    }
}

void SysTick_Handler (void)
{
    hbox_tick_inc();
}

void SW_Handler(void)
{

}

void USART2_IRQHandler(void)
{
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        USART_ClearITPendingBit(USART2,USART_IT_RXNE);
        uint8_t data=USART_ReceiveData(USART2);
        hbox_shell_console_input(data);
    }
    if(USART_GetITStatus(USART2, USART_IT_TXE) != RESET)
    {
        USART_ClearITPendingBit(USART2,USART_IT_TXE);
        uint8_t data=0;
        if(hbox_shell_console_output(&data))
        {
            USART_SendData(USART2,data);
        }
        else
        {
            /*
             * 停止发送
             */
            USART_ITConfig(USART2,USART_IT_TXE,DISABLE);
        }
    }
}
