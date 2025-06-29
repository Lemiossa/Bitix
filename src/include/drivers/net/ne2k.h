#ifdef __net__
#ifndef NE2K_H
#define NE2K_H

#include "types.h"

#define NE2K_IO_BASE  0x300
#define NE2K_IRQ      10

#define ETH_ALEN 6
#define ETH_HLEN 14

#define ETH_P_IP 0x0800
#define ETH_P_ARP 0x0806

extern bool ne2k_packet_receied;

void ne2k_reset();
void ne2k_init(uchar *mac_out);
void ne2k_send_packet(uchar *data, ushort len);
bool ne2k_receive_packet(uchar *buf, ushort *len);
void ne2k_irq_handler();

#endif /* NE2K_H */
#endif
