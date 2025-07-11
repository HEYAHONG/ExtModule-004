#include "hbox_config.h"
#include "hbox_shell.h"
/*
 * 采用串口2作为调试串口
 */
static bool is_usart2_init_ok=false;
static hringbuf_t * console_rx_buffer_get(void);
int hbox_shell_putchar(int ch)
{
    if(ch>0)
    {
        if(is_usart2_init_ok)
        {
            while(RESET==USART_GetFlagStatus(USART2,USART_FLAG_TXE));
            USART_SendData(USART2,(uint16_t)ch);
        }

    }
    return ch;
}

void hbox_shell_console_input(uint8_t data)
{
    hringbuf_t *rbuf=console_rx_buffer_get();
    if(rbuf!=NULL)
    {
        hringbuf_input(rbuf,&data,sizeof(data));
    }
}
int hbox_shell_getchar(void)
{
    int ch=EOF;
    {
        hringbuf_t *rbuf=console_rx_buffer_get();
        if(rbuf!=NULL)
        {
            uint8_t data=0;
            if(hringbuf_get_length(rbuf)>0)
            {
                hringbuf_output(rbuf,&data,sizeof(data));
                ch=data;
            }
        }
    }
    return ch;
}


static uint8_t console_rx_buffer[256]= {0};
static hringbuf_t * console_rx_buffer_get(void)
{
    hringbuf_t * buffer=hringbuf_get(console_rx_buffer,sizeof(console_rx_buffer));
    hringbuf_set_lock(buffer,NULL,NULL,NULL);
    return buffer;
}

static void  hbox_shell_console_init(void)
{
    {
        //IO口
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
        GPIO_InitTypeDef GPIO_InitStructure= {0};
        GPIO_InitStructure.GPIO_Pin=GPIO_Pin_2 | GPIO_Pin_3;
        GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
        GPIO_Init(GPIOA,&GPIO_InitStructure);
    }

    {
        //USART2
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
        USART_InitTypeDef USART_InitStructure= {0};
        USART_InitStructure.USART_BaudRate=115200;
        USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
        USART_InitStructure.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;
        USART_InitStructure.USART_Parity=USART_Parity_No;
        USART_InitStructure.USART_StopBits=USART_StopBits_1;
        USART_InitStructure.USART_WordLength=USART_WordLength_8b;
        USART_Init(USART2,&USART_InitStructure);
        USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
        NVIC_InitTypeDef  NVIC_InitStructure = {0};
        NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
        USART_Cmd(USART2,ENABLE);
    }
    is_usart2_init_ok=true;
}
static void putchar_cb(char c)
{
    hbox_shell_putchar((uint8_t)c);
}

static void  hbox_shell_init(const hruntime_function_t *func)
{
    hbox_shell_console_init();
    hshell_command_name_shortcut_set(NULL,true);
    HSHELL_COMMANDS_REGISTER(NULL);
    hprintf_set_callback(putchar_cb);
    console_printf("console init!");
}
HRUNTIME_INIT_EXPORT(shell,0,hbox_shell_init,NULL);

static void  hbox_shell_loop(const hruntime_function_t *func)
{
    while(hshell_loop(NULL)==0);
}
HRUNTIME_LOOP_EXPORT(shell,0,hbox_shell_loop,NULL);

