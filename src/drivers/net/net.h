#ifndef NET_H
#define NET_H

#include "types.h"

#define ETH_ALEN 6
#define IP_ALEN 4

#define ETH_P_IP 0x0800
#define ETH_P_ARP 0x0806

#define IP_PROTO_ICMP 1
#define IP_PROTO_TCP 6
#define IP_PROTO_UDP 17

typedef struct {
	uchar dst[ETH_ALEN];
	uchar src[ETH_ALEN];
	ushort type;
} eth_t;

typedef struct {
	uchar val_ihl;
	uchar tos;
	ushort total_len;
	ushort id;
	ushort flags_frag;
	uchar ttl;
	uchar protocol;
	ushort checksum;
	uchar src[IP_ALEN];
	uchar dst[IP_ALEN];
} ip_t;

typedef struct {
	ushort src_port;
	ushort dst_port;
	ushort length;
	ushort checksum;
} udp_t;

typedef struct {
	uchar hw_type[2];
	uchar proto_type[2];
	uchar hw_size;
	uchar proto_size;
	ushort opcode;
	uchar sender_mac[ETH_ALEN];
	uchar sender_ip[IP_ALEN];
	uchar target_mac[ETH_ALEN];
	uchar target_ip[IP_ALEN];
} arp_packet_t;

/* Network funcs */
void net_init (uchar *mac, uchar *ip, uchar *gateway, uchar *mask);
void net_poll ();
void net_process_packet (uchar *data, ushort len);

/* ARP funcs */
void arp_request (uchar *target_ip);
void arp_reply (uchar *target_mac, uchar *target_ip);
void arp_cache_add (uchar *ip, uchar *mac);
bool arp_cache_lookup (uchar *ip, uchar *mac);

/* IP funcs */
void ip_send (uchar *dst_ip, uchar protocol, uchar *data, ushort len);

/* UDP funcs */
void udp_send (uchar *dst_ip, ushort dst_port, ushort src_port, uchar *data, ushort len);
bool udp_receive (uchar *buf, ushort *len, uchar *src_ip, ushort *src_port);

/* ICMP funcs */
void icmp_reply (uchar *src_ip, uchar *data, ushort len);

#endif /* NET_H */
