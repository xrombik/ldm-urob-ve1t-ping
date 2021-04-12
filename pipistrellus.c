#include "pipistrellus.h"


uint8_t MAC_BROADCAST[]      = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
const uint8_t MAC_ZERO[]     = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t  IP_ANY_ADDR[4] = {0, 0, 0, 0};
uint16_t IP_ANY_PORT   = 0x0000;


extern uint32_t eth_printf(const char* fmt, ...);


void init_buffer(buffer* buf)
{
    buf->size_alloc = sizeof buf->data;
    buf->size_used = 0U;
}

uint16_t get_checksum(const void *data, uint32_t count)
{
    /* Compute Internet Checksum for "count" bytes beginning at location "addr"  */
    uint32_t sum = 0U;

    while (count > 1U)
    {
        /*  This is the inner loop */
        sum += *((uint16_t*) data) ++;
        count -= 2U;
    }

    /*  Add left-over byte, if any */
    if (count > 0U)
        sum += * (uint8_t*) data;

    /*  Fold 32-bit sum to 16 bits */
    while (sum >> 16U)
       sum = (sum & 0xffff) + (sum >> 16U);

   return ~sum;
}


bool arp_send(buffer* tx_buffer, const buffer* rx_buffer, const mac_addrs* mac_addr, const uint32_t addr)
{
    tx_buffer->size_used = 0U;
    if (tx_buffer->size_alloc < sizeof(arp_frame))
        return false;
    if (rx_buffer->size_used < sizeof(arp_frame))
        return false;
    
    arp_frame* tx_arpf = (arp_frame*) tx_buffer->data;
    arp_frame* rx_arpf = (arp_frame*) rx_buffer->data;
    
    memcpy(tx_arpf->maddrs.sndr, mac_addr->sndr,       sizeof tx_arpf->maddrs.sndr);
    memcpy(tx_arpf->sndr_mac,    mac_addr->sndr,       sizeof tx_arpf->sndr_mac);
    memcpy(tx_arpf->maddrs.trgt, rx_arpf->maddrs.sndr, sizeof tx_arpf->maddrs.trgt);
    memcpy(tx_arpf->trgt_mac,    rx_arpf->sndr_mac,    sizeof tx_arpf->trgt_mac);
    
    tx_arpf->maddrs.type = rx_arpf->maddrs.type;
    tx_arpf->sndr_ip     = addr;
    tx_arpf->trgt_ip     = rx_arpf->sndr_ip;
    tx_arpf->opcode      = ARP_REP;
    tx_arpf->hw_type     = rx_arpf->hw_type;
    tx_arpf->hw_size     = rx_arpf->hw_size;
    tx_arpf->prot_size   = rx_arpf->prot_size;
    tx_arpf->prot_type   = rx_arpf->prot_type;
    
    tx_buffer->size_used = sizeof *tx_arpf;
    
    return true;
}


bool arp_receive(const buffer* rx_buffer, const mac_addrs* maddrs, uint32_t ip_addr)
{
    if (rx_buffer->size_used < sizeof(arp_frame))
        return false;
    arp_frame* arpf = (arp_frame*) rx_buffer->data;
    if (arpf->maddrs.type != ETH_TYPE_ARP)
        return false;
    if (arpf->opcode != ARP_ASQ)
        return false;
    return arpf->trgt_ip == ip_addr;
}


bool mac_set_addr(buffer* x_buffer, const mac_addrs* addr)
{
    if (x_buffer->size_alloc < sizeof *addr)
        return false;
    memcpy(x_buffer->data, addr, sizeof *addr);
    return true;
}


void mac_init_addr(mac_addrs* maddr, const uint8_t* src, const uint8_t* dst)
{
    memcpy(maddr->sndr, src, sizeof maddr->sndr);
    memcpy(maddr->trgt, dst, sizeof maddr->trgt);
    maddr->type = 0U;
}


bool mac_receive(const buffer* rx_buffer, const mac_addrs* maddr)
{
    if (rx_buffer->size_used < sizeof *maddr)
        return false;
    mac_addrs* rx_madrs = (mac_addrs*) rx_buffer->data;
    if (memcmp(rx_madrs->trgt, MAC_BROADCAST, sizeof rx_madrs->trgt) == 0)
        return true;
    if (memcmp(rx_madrs->trgt, MAC_ZERO, sizeof rx_madrs->trgt) == 0)
        if (memcmp(rx_madrs->sndr, MAC_ZERO, sizeof rx_madrs->sndr) == 0)
            return true;
    return memcmp(rx_madrs->trgt, maddr->sndr, sizeof rx_madrs->trgt) == 0;
}


bool mac_send(buffer* tx_buffer, const mac_addrs* addr)
{
    if (tx_buffer->size_alloc < sizeof *addr)
        return false;
    memcpy(tx_buffer->data, addr, sizeof *addr);
    return true;
}


bool icmp_receive(const buffer* rx_buffer, uint32_t ip_addr)
{
    if (rx_buffer->size_used < sizeof (icmp_frame))
        return false;
    icmp_frame* icmpf = (icmp_frame*) rx_buffer->data;
    if (icmpf->maddrs.type != ETH_TYPE_IPV4)
        return false;
    if (icmpf->proto != ICMP_PROTO)
        return false;
    if (icmpf->opcode != ICMP_TYPE_ASQ)
        return false;
    return icmpf->trgt_ip == ip_addr;
}


bool icmp_send(buffer* tx_buffer, const buffer* rx_buffer)
{
    if (rx_buffer->size_used < sizeof (icmp_frame))
        return false;
    if (tx_buffer->size_alloc < rx_buffer->size_used)
        return false;
    
    tx_buffer->size_used  = rx_buffer->size_used;

    icmp_frame* tx_icmpf = (icmp_frame*) tx_buffer->data;
    icmp_frame* rx_icmpf = (icmp_frame*) rx_buffer->data;

    memcpy(tx_icmpf->maddrs.trgt, rx_icmpf->maddrs.sndr, sizeof tx_icmpf->maddrs.trgt);
    memcpy(tx_icmpf->maddrs.sndr, rx_icmpf->maddrs.trgt, sizeof tx_icmpf->maddrs.sndr);
    tx_icmpf->maddrs.type = rx_icmpf->maddrs.type;
    tx_icmpf->dsc_ecn     = rx_icmpf->dsc_ecn; 
    tx_icmpf->verlen      = rx_icmpf->verlen;
    tx_icmpf->totlen      = rx_icmpf->totlen;
    tx_icmpf->id          = swap16(swap16(rx_icmpf->id) + 1U);
    tx_icmpf->flags       = rx_icmpf->flags;
    tx_icmpf->hdr_csum    = 0U;
    tx_icmpf->csum        = 0U;
    tx_icmpf->ttl         = rx_icmpf->ttl;
    tx_icmpf->proto       = rx_icmpf->proto;
    tx_icmpf->sndr_ip     = rx_icmpf->trgt_ip;
    tx_icmpf->trgt_ip     = rx_icmpf->sndr_ip;
    tx_icmpf->opcode      = ICMP_TYPE_REP;
    tx_icmpf->code        = rx_icmpf->code;
    tx_icmpf->be          = rx_icmpf->be;
    tx_icmpf->le          = rx_icmpf->le;
    
    memcpy(tx_icmpf->time, rx_icmpf->time, sizeof tx_icmpf->time);
    memcpy(tx_buffer->data + sizeof(*tx_icmpf), rx_buffer->data + sizeof(*rx_icmpf), tx_buffer->size_used - sizeof(*tx_icmpf));
    
    tx_icmpf->csum        = get_checksum(&tx_icmpf->opcode, tx_buffer->size_used - (uint32_t)((uint8_t*) &tx_icmpf->opcode - (uint8_t*)tx_icmpf));
    tx_icmpf->hdr_csum    = get_checksum(&tx_icmpf->verlen, (uint32_t) ((uint8_t*) &tx_icmpf->opcode - (uint8_t*) &tx_icmpf->verlen));
    
    return true;
}
