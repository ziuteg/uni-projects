#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "utilities.h"

struct timespec delta_timespec(struct timespec t1, struct timespec t2) {

	struct timespec ts;
	ts.tv_sec = t1.tv_sec - t2.tv_sec;
	ts.tv_nsec = t1.tv_nsec - t2.tv_nsec;

	while (ts.tv_nsec > NANO_PER_SEC) {
        ts.tv_sec++;
        ts.tv_nsec -= NANO_PER_SEC;
    }
    while (ts.tv_nsec < 0) {
        ts.tv_sec--;
        ts.tv_nsec += NANO_PER_SEC;
    }

    return ts;
}

in_addr_t get_broadcast_addr(in_addr_t addr, uint8_t mask) {
	addr = ntohl(addr);
	addr |= ~(0xFFFFFFFF << (32 - mask));
	return htonl(addr);
}

in_addr_t get_network_addr(in_addr_t addr, uint8_t mask) {
	addr = ntohl(addr);
	addr &= 0xFFFFFFFF << (32 - mask);
	return htonl(addr);
}