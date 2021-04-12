#include "main.h"

/* mac-адрес этого узла */

const uint8_t MAC_SRC[]  = {0x00, 0xa0, 0x71, 0x67, 0x2c, 0xf7};
const uint8_t  IP_ADDR[] = {10,  1,   2,  70};    /**< ip-адрес этого узла */
extern uint8_t MAC_BROADCAST[6];


#define TEST_MSG "1986VE1T"

void delay(uint32_t ticks)
{
    while(ticks --);
}


//	Тактирование ядра от HSE, 8МГц на демо-плате
void Clock_Init(void)
{
	/* Enable HSE (High Speed External) clock */
	RST_CLK_HSEconfig(RST_CLK_HSE_ON);
	while (RST_CLK_HSEstatus() != SUCCESS);

	/* Configures the CPU_PLL clock source */
	RST_CLK_CPU_PLLconfig(RST_CLK_CPU_PLLsrcHSEdiv1, RST_CLK_CPU_PLLmul9);

	/* Enables the CPU_PLL */
	RST_CLK_CPU_PLLcmd(ENABLE);
	while (RST_CLK_CPU_PLLstatus() == ERROR);

	/* Enables the RST_CLK_PCLK_EEPROM */
	RST_CLK_PCLKcmd(RST_CLK_PCLK_EEPROM, ENABLE);
	
    /* Sets the code latency value */
	EEPROM_SetLatency(EEPROM_Latency_5);

	/* Select the CPU_PLL output as input for CPU_C3_SEL */
	RST_CLK_CPU_PLLuse(ENABLE);
	
    /* Set CPUClk Prescaler */
	RST_CLK_CPUclkPrescaler(RST_CLK_CPUclkDIV1);

	/* Select the CPU clock source */
	RST_CLK_CPUclkSelection(RST_CLK_CPUclkCPU_C3);
    
    ETH_ClockDeInit();

	/*	Включение генератора HSE2 = 25МГц */
	RST_CLK_HSE2config(RST_CLK_HSE2_ON);
    while (RST_CLK_HSE2status() != SUCCESS);

	/* Тактирование PHY от HSE2 = 25МГц */
	ETH_PHY_ClockConfig(ETH_PHY_CLOCK_SOURCE_HSE2, ETH_PHY_HCLKdiv1);

	/* Без делителя */
	ETH_BRGInit(ETH_HCLKdiv1);

	/* Включение тактирования блока MAC */
	ETH_ClockCMD(ETH_CLK1, ENABLE);
}


void Ethernet_Init(const uint8_t* mac_addr)
{
    ETH_InitTypeDef eth_init;
	ETH_DeInit(MDR_ETHERNET1);
    delay(100000U);
	ETH_StructInit(&eth_init);
    memcpy(eth_init.ETH_MAC_Address, mac_addr, sizeof eth_init.ETH_MAC_Address);
 	ETH_Init(MDR_ETHERNET1, &eth_init);
    delay(100000U);
	ETH_PHYCmd(MDR_ETHERNET1, ENABLE);
    delay(10000000U);
    ETH_Start(MDR_ETHERNET1);
    delay(10000000U);
    while (MDR_ETHERNET1->PHY_Status & ETH_PHY_FLAG_LINK);
    delay(10000000U);
}


uint32_t eth_printf(const char* fmt, ...)
{
    static uint8_t tx_data[MTU_SIZE];
    uint32_t cnt;
    va_list args;
    cnt = 0U;
    va_start(args, fmt);
    cnt += vsprintf((char*) &tx_data[cnt + sizeof(uint32_t) + sizeof(mac_addrs)], fmt, args);
    va_end(args);
    *(uint32_t*)tx_data = cnt + sizeof '\0' + sizeof(mac_addrs);
    ETH_SendFrame(MDR_ETHERNET1, (uint32_t*)tx_data, *(uint32_t*)tx_data);
    return cnt;
}


void eth_receive(buffer* rx_buffer)
{
    rx_buffer->size_used = 0U;
    VE1T_ETH_STAT* eth_stat = (VE1T_ETH_STAT*) &MDR_ETHERNET1->ETH_STAT;
    if (MDR_ETHERNET1->ETH_R_Head == MDR_ETHERNET1->ETH_R_Tail)
        return;
    uint32_t status = ETH_ReceivedFrame(MDR_ETHERNET1, (uint32_t*) rx_buffer->data);
    ETH_StatusPacketReceptionBitFileds* fields = (ETH_StatusPacketReceptionBitFileds*) &status;
    rx_buffer->size_used = fields->Length - sizeof(uint32_t);
    if (eth_stat->R_COUNT)
        eth_stat->R_COUNT --;
}


uint32_t eth_send_data(buffer* tx_buffer)
{
    if (!tx_buffer->size_used)
        return 0U;
    if (MDR_ETHERNET1->PHY_Status & ETH_PHY_FLAG_LINK)
        return 0U;
    if (MDR_ETHERNET1->ETH_X_Head != MDR_ETHERNET1->ETH_X_Tail)
        return 0U;
    ETH_SendFrame(MDR_ETHERNET1, &tx_buffer->size_used, tx_buffer->size_used);
    return tx_buffer->size_used;
}


void eth_update_leds(void)
{        
    if (MDR_ETHERNET1->PHY_Status & ETH_PHY_FLAG_LINK)
        PORT_ResetBits(MDR_PORTF, PORT_Pin_13);
    else
        PORT_SetBits(MDR_PORTF, PORT_Pin_13);
    if (MDR_ETHERNET1->PHY_Status & ETH_PHY_FLAG_CARRIER_SENSE)
        PORT_ResetBits(MDR_PORTF, PORT_Pin_14);
    else
        PORT_SetBits(MDR_PORTF, PORT_Pin_14);
}


void Ports_Init(void)
{
    PORT_InitTypeDef gpio;
    RST_CLK_PCLKcmd (RST_CLK_PCLK_PORTF, ENABLE);
    PORT_StructInit (&gpio);
    gpio.PORT_Pin   = PORT_Pin_13 | PORT_Pin_14;
    gpio.PORT_OE    = PORT_OE_OUT;
    gpio.PORT_SPEED = PORT_SPEED_SLOW;
    gpio.PORT_MODE  = PORT_MODE_DIGITAL;
    PORT_Init(MDR_PORTF, &gpio);
}


int main(void)
{
	Clock_Init();
	Ethernet_Init(MAC_SRC);
    Ports_Init();
    
    mac_addrs maddr;
    mac_init_addr(&maddr, MAC_SRC, MAC_BROADCAST);
    
    buffer tx_buffer;
    init_buffer(&tx_buffer);
    
    buffer rx_buffer;
    init_buffer(&rx_buffer);
    
    eth_printf("%s:%u:\"%s\" %s", __FILE__, __LINE__, TEST_MSG, __DATE__);

    while (1U)
    {
        eth_update_leds();
        eth_receive(&rx_buffer);

        if (rx_buffer.size_used)
        {
            if (arp_receive(&rx_buffer, &maddr, *(uint32_t*)IP_ADDR))
            {
                arp_send(&tx_buffer, &rx_buffer, &maddr, *(uint32_t*)IP_ADDR);
            }
            else if (icmp_receive(&rx_buffer, *(uint32_t*)IP_ADDR))
            {
                icmp_send(&tx_buffer, &rx_buffer);
            }
        }
        if (tx_buffer.size_used)
        {
            if (eth_send_data(&tx_buffer))
            {
                tx_buffer.size_used = 0U;
            }
        }
    }
}
