#ifndef __ROUTER_H__
#define __ROUTER_H__

#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "utilities.h"
#include "distance_vector.h"

int router_init(int *sockfd);

void router_print(struct dv_node *node);

int router_broadcast(int sockfd, struct net_table networks, struct dv_node *dist_vector);

void router_update(struct dv_node **dist_vector);

in_addr_t get_sender_network_addr(struct net_table networks, struct sockaddr_in sender);

int router_receive(int sockfd, struct net_table networks, struct dv_node *dist_vector);

int router(struct net_table networks);

#endif
