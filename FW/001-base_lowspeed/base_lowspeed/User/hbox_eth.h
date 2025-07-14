#ifndef __HBOX_ETH_H__
#define __HBOX_ETH_H__
#include "eth_driver.h"
#include "hbox.h"
#ifdef __cplusplus
extern "C"
{
#endif

bool hbox_eth_is_linked(void);
typedef enum
{
    HBOX_ETH_EVENT_LINKED_STATE_CHANGE=1,
    HBOX_ETH_EVENT_RECEIVE_DATA,/*包含两个额外的参数(分别是const uint8_t *、size_t)*/
} hbox_eth_event_t;

typedef void (*hbox_eth_event_callback_t)(hbox_eth_event_t evt,...);
void hbox_eth_set_callback(hbox_eth_event_callback_t cb);
bool hbox_eth_transmit(const uint8_t *data,size_t data_length);

#ifdef __cplusplus
}
#endif


#endif
