#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define ROUTER_PORT 54321
#define ROUND_DELAY 3
#define INF 10
#define TIMEOUT 5

#define NANO_PER_SEC 1000000000
#define MICRO_PER_SEC 1000000
#define MILI_PER_SEC 1000

struct datagram {
	in_addr_t net_addr;
	uint8_t mask;
	uint32_t dist;
} __attribute__ ((__packed__));


/* informacje o bezpośrednio połączonej sieci */
struct net_data {
	in_addr_t addr;
	in_addr_t net_addr;
	in_addr_t broadcast_addr;
	uint8_t mask;
	uint32_t dist;
};

/* strukura przechowująca informacje o bezpośrednio
   połączonych sieciach */
struct net_table {
	struct net_data* entry;
	size_t size;
};

struct timespec delta_timespec(struct timespec t1, struct timespec t2);

in_addr_t get_broadcast_addr(in_addr_t addr, uint8_t mask);

in_addr_t get_network_addr(in_addr_t addr, uint8_t mask);

#endif