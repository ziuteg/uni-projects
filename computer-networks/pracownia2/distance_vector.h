#ifndef __DISTANCE_VECTOR_H__
#define __DISTANCE_VECTOR_H__

#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "utilities.h"

/* struktura przechowujaca elementy wektora odleglosci
   w postaci listy jednokierunkowej */
struct dv_node {
	struct dv_node* next;

	in_addr_t net_addr;
	uint8_t mask;
	uint32_t dist;

	in_addr_t via;
	int connected_directly;
	int unreachable;
	time_t timeout;
};

void dv_update(struct datagram data, in_addr_t sender_addr, in_addr_t sender_net_addr, struct net_table networks, struct dv_node *dist_vector);

struct dv_node *dv(struct net_data entry);

struct dv_node *dv_sent(struct datagram datagr, struct dv_node *sender);

struct dv_node *dv_init(struct net_table networks);

struct dv_node *dv_find(struct dv_node *node, in_addr_t addr);

void dv_insert(struct dv_node *node, struct dv_node *new_node);

void dv_remove(struct dv_node **node, in_addr_t addr);

void dv_free(struct dv_node *node);

#endif