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

u_int16_t compute_icmp_checksum (const void *buff, int length)
{
	u_int32_t sum;
	const u_int16_t* ptr = buff;
	assert (length % 2 == 0);
	for (sum = 0; length > 0; length -= 2)
		sum += *ptr++;
	sum = (sum >> 16) + (sum & 0xffff);
	return (u_int16_t)(~(sum + (sum >> 16)));
}
