
#ifndef __TRACEROUTE_H__
#define __TRACEROUTE_H__

#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_TTL 30
#define ECHO_REQUESTS 3
#define WAIT 1

struct packet_display {
	struct sockaddr_in 	sender; // address
	time_t rtt; 				// roud trip time (ns)
	int timeout; 				// timeout
};

int init();

int traceroute(char *addr);

int icmp_send(int sockfd, char* addr, int ttl, int pid, int seq);

int icmp_receive(int sockfd, int pid, int ttl, struct packet_display* display);

void trace_print(struct packet_display* display);

#endif
