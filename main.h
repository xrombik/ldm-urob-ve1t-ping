#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>

#include <MDR32F9Qx_port.h>
#include <MDR32F9Qx_rst_clk.h>
#include <MDR32F9Qx_eeprom.h>
#include <MDR32F9Qx_eth.h>

#include "pipistrellus.h"

#pragma pack(push, 1)

typedef struct 
{
    uint16_t
    R_EMPTY     :1,     /** */
    R_AEMPTY    :1,     /** */
    R_HALF      :1,     /** */
    R_AFULL     :1,     /** */
    R_FULL      :1,     /** */
    R_COUNT     :3,     /** */
    X_EMPTY     :1,     /** */
    X_AEMPTY    :1,     /** */
    X_HALF      :1,     /** */
    X_AFULL     :1,     /** */
    X_FULL      :1,     /** */
                :3;     /** */
} VE1T_ETH_STAT;

#pragma pack(pop)

void eth_update_leds(void);
