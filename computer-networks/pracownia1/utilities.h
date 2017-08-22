
#ifndef __UTILITIES_H__
#define __UTILITIES_H__

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

#define NANO_PER_SEC 1000000000
#define MICRO_PER_SEC 1000000
#define MILI_PER_SEC 1000

struct timespec delta_timespec(struct timespec t1, struct timespec t2);

u_int16_t compute_icmp_checksum (const void *buff, int length);

#endif
