#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/** \file  */

#define swap16(x) ((uint16_t) ((((x) >> 8U) & 0xff) | (((x) & 0xff) << 8U)))  
#define ETH_TYPE_ARP    swap16  (0x0806)   /**< Значение поля mac_addrs::type - arp-протокол */
#define ETH_TYPE_IPV4   swap16  (0x0800)   /**< Значение поля mac_addrs::type - ipv4-протокол */
#define ARP_ASQ         swap16  (  0x01)   /**< Значение поля arp_frame::opcode - запрос */
#define ARP_REP         swap16  (  0x02)   /**< Значение поля arp_frame::opcode - ответ */
#define ICMP_VERLEN                0x45    /**< Версия и длина */
#define ICMP_PROTO                    1    /**< Значение поля icmp_frame::proto */
#define ICMP_TYPE_ASQ                 8    /**< Значение поля icmp_frame::type - запрос */
#define ICMP_TYPE_REP                 0    /**< Значение поля icmp_frame::type - ответ */
#define ICMP_FLAGS_DF  swap16   (0x4000)   /**< Значение поля icmp_frame::flags - ответ */    
#define UDP_PROTO                    17    /**< Значение поля udp_frame::proto */
#define MTU_SIZE                   1500    /**< */

#pragma pack(push, 1)

/** Буфер для выделения области */
struct buffer
{
    uint32_t size_alloc;     /**< Размер области в байтах выделенный */
    uint32_t size_used;      /**< Размер области в байтах занятый */
    uint8_t  data[MTU_SIZE]; /**< Указатель на начало занимаемой области */
    buffer (void)
    {
        size_alloc = sizeof data;
        size_used = 0U;
    }
};


/** Буфер для размещения внутри области */
struct udp_buffer
{
    uint32_t size_used;
    uint8_t* data;
};


/** mac-адреса */
typedef struct
{
    uint8_t trgt[6]; /**< mac-адрес получателя */
    uint8_t sndr[6]; /**< mac-адрес отправителя */
  uint16_t type;  /**< тип протокола */
} mac_addrs;


/** udp-адрес */
typedef struct
{
  uint16_t port;
  uint32_t addr;
} udp_addr;


/** Формат icmp-запроса и icmp-ответа для выполнения пинга */
typedef struct
{
  mac_addrs maddrs;    /**< mac-адреса получателя и отправителя */
  uint8_t   verlen;    /**< версия и длина */
  uint8_t   dsc_ecn;   /**< Differentiated Services Codepoint: Default (0)
                            Explicit Congestion Notification: Not ECN-Capable Transport (0) */
  uint16_t  totlen;    /**< Total Length. Длина icmp-данных  + sizeof(icmp_frame) */
  uint16_t  id;        /**< Identification, случайное значение? */
  uint16_t  flags;     /**< Флаги, все установлены в 0 */
  uint8_t   ttl;       /**< Время жизни */
  uint8_t   proto;     /**< Протокол */
  uint16_t  hdr_csum;  /**< Header checksum */
  uint32_t  sndr_ip;   /**< ip-адрес отправителя */
  uint32_t  trgt_ip;   /**< ip-адрес получателя */
  uint8_t   opcode;    /**< тип icmp-запроса */
  uint8_t   code;      /**< Code, всегда 0 */
  uint16_t  csum;      /**< Контрольная сумма */
  uint16_t  be;        /**< Identifier BE. В ответе повторяют значение из запроса */
  uint16_t  le;        /**< Identifier LE. В ответе повторяют значение из запроса */
  uint8_t   time[8];   /**< Отметка времени. В ответе повторяют значение из запроса */
                       /**< Здесь начинаются icmp-данные. В ответе повторяют значение из запроса */
} icmp_frame;


/** Формат arp-запроса и arp-ответа */
typedef struct
{
  mac_addrs maddrs;       /**< mac-адреса получателя и отправителя */
  uint16_t  hw_type;      /**<  */
  uint16_t  prot_type;    /**<  */
  uint8_t   hw_size;      /**<  */
  uint8_t   prot_size;    /**<  */
  uint16_t  opcode;       /**< код операции, одно из ARP_ASQ, ARP_REP */
  uint8_t   sndr_mac[6];  /**< mac-адрес отправителя */
  uint32_t  sndr_ip;      /**< ip-адрес отправителя */
  uint8_t   trgt_mac[6];  /**< mac-адрес получателя */
  uint32_t  trgt_ip;      /**< ip-адрес получателя */
} arp_frame;


/** Формат udp-заголовка */
typedef struct
{
    mac_addrs   maddrs;    /**< mac-адреса получателя и отправителя */
	uint8_t     verlen;
	uint8_t     tos;
	uint16_t    length;
	uint16_t    id;
	uint16_t    offset;
	uint8_t     ttl;       /**< ip поле, время жизни */
	uint8_t     proto;     /**< ip поле, код протокола */
	uint16_t    xsum;      /**< ip поле, контрольная сумма */
	uint32_t    sndr_addr; /**< ip поле, ip-адрес отправителя */
	uint32_t    trgt_addr; /**< ip поле, ip-адрес получателя */
	uint16_t    sndr_port; /**< udp поле, ip-порт отправителя */
	uint16_t    trgt_port; /**< udp поле, ip-порт получателя */
	uint16_t    lendg;	   /**< udp поле, длинна udp-датаграммы */
	uint16_t    xsumd;	   /**< udp поле, котрольная сумма udp-датаграммы */
                           /**< Здесь начинается пользовательская датаграмма */
} udp_frame;


/** Псевдо ip-заголовок для подсчёта контрольной суммы */
typedef struct
{
	udp_addr source_address;
	udp_addr dest_address;
	uint8_t place_holder;
	uint8_t proto;
	uint16_t length;
} ip_pseudoheader;

#pragma pack(pop)

/** */
uint16_t get_checksum(const void *data, uint32_t len);


/** Заполняет структуру 
 \param[out] maddr Заполняемая структура
 \param[in] src mac-адрес отправителя
 \param[in] dst mac-адрес получателя */
void mac_init_addr(mac_addrs* maddrs, const uint8_t* src, const uint8_t* dst);


/** Заполняет поля mac-адресов в буфере
 \param[out] x_buffer Буфер для размещения mac-адресов
 \param[in] maddrs mac-адреса
 \return true - если адрес размещён, false - если не размещён (недостаточно места) */
bool mac_set_addr(buffer* x_buffer, const mac_addrs* maddrs);


/** Проверяет, что пакет адресован текущему узлу на mac-уровне
 \param[in] buffer Буфер содержащий данные приятые "с провода"
 \param[in] maddrs mac-адрес текущего узла 
 \return true - если пакет адресован текущему узлу и false - если иначе */
bool mac_receive(const buffer* rx_buffer, const mac_addrs* maddrs);


/** Проверяет, что icmp-запрос предназначен для текущего узла
 \param[in] buffer Буфер содержащий данные приятые "с провода"
 \param[in] ip_addr ip-адрес текущего узла 
 \return true если icmp-запрос адресован текущему узлу и false если иначе */
bool icmp_receive(const buffer* rx_buffer, uint32_t ip_addr);


/** Заполняет буфер ответом на icmp-запрос
 \param[out] tx_buffer Буфер в котором будет размещён ответ на icmp-запрос 
 \param[in] rx_buffer Буфер в котором размещён icmp-запрос
 \return true - если ответ размещён, false - если иначе (недостаточно места) */
bool icmp_send(buffer* tx_buffer, const buffer* rx_buffer);


/** Проверяет, что arp-запрос предназначен для текущего узла
 \param[in] rx_buffer Буфер в котором содержится arp-запрос
 \param[in] maddr mac-адреса
 \param[in] ip_addr ip-адрес текущего узла 
 \return true - если arp-запрос адресован текущему узлу и false - если иначе */
bool arp_receive(const buffer* rx_buffer, const mac_addrs* maddr, uint32_t ip_addr);


/** */
bool arp_send(buffer* tx_buffer, const buffer* rx_buffer, const mac_addrs* mac_addr, const uint32_t ip_addr);


/** */
void udp_init_addr(udp_addr* udp_addr, const uint8_t* addr, const uint16_t port);


/** Размещает буфер содержащий udp-датаграмму
 \param[in] x_buffer Буфер в котором содержится пакет с udp-датаграммой
 \param[out] udp_buffer Размещаемый udp-буфер
 \return true - если буфер размещён, false - если иначе */
bool udp_get_data(const buffer* x_buffer, buffer* udp_buffer);


/** */
bool udp_get_src(const buffer* x_buffer, udp_addr* addr);


/** */
bool udp_get_dst(const buffer* x_buffer, udp_addr* addr);


/** */
bool udp_cmp_src(const buffer* x_buffer, const udp_addr* addr);


/** */
bool udp_cmp_dst(const buffer* x_buffer, const udp_addr* addr);


/** */
bool udp_receive(const buffer* rx_buffer, const udp_addr* trgt, const udp_addr* sndr, uint32_t netmask);


/** */
bool udp_set_src(buffer* tx_buffer, udp_addr* addr);


/** */
bool udp_set_dst(buffer* tx_buffer, udp_addr* addr);


/** */
bool udp_set_xsum(buffer* tx_buffer);


/** */
bool udp_send(buffer* tx_buffer, const udp_addr* src, const udp_addr* dst);
