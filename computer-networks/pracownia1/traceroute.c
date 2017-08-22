
#include "traceroute.h"
#include "utilities.h"

int init(int *sockfd) {
	*sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (*sockfd < 0) {
		fprintf(stderr, "socket error: %s\n", strerror(errno)); 
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int icmp_send(int sockfd, char* addr, int ttl, int pid, int seq) {

	// Konstruujemy komunikat ICMP do wysłania
	struct icmphdr icmp_header;
	icmp_header.type = ICMP_ECHO;
	icmp_header.code = 0;
	icmp_header.un.echo.id = pid;
	icmp_header.un.echo.sequence = seq;
	icmp_header.checksum = 0;
	icmp_header.checksum = compute_icmp_checksum (
		(u_int16_t*)&icmp_header, sizeof(icmp_header));

	// Wpisujemy adres odbiorcy do struktury adresowej:
	struct sockaddr_in recipient;
	bzero (&recipient, sizeof(recipient));
	recipient.sin_family = AF_INET;
	inet_pton(AF_INET, addr, &recipient.sin_addr);

	// Pole TTL jest w nagłówku IP → brak bezpośredniego dostępu.
	// Zmiana wywołaniem:
	setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(int));

	ssize_t bytes_sent = sendto (
		sockfd,
		&icmp_header,
		sizeof(icmp_header),
		0,
		(struct sockaddr*)&recipient,
		sizeof(recipient)
	);

	if (bytes_sent < 0) {
		fprintf(stderr, "send error: %s\n", strerror(errno)); 
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int icmp_receive(int sockfd, int pid, int ttl, struct packet_display* display) {

	int echoreply = 0;
	struct timespec ts_end;
	struct timespec ts_start;
	clock_gettime(CLOCK_MONOTONIC, &ts_start);
	ts_end = ts_start;
	ts_end.tv_sec += WAIT;

	for (int pck_cnt = 0; pck_cnt < ECHO_REQUESTS; pck_cnt++) {

		display[pck_cnt].timeout = 0;

		fd_set descriptors;
		FD_ZERO (&descriptors);
		FD_SET (sockfd, &descriptors);
		
		// Czekanie maksymalnie WAIT sekund na pakiety w gnieździe sockfd
		struct timespec ts_current;
		clock_gettime(CLOCK_MONOTONIC, &ts_current);
		struct timespec ts = delta_timespec(ts_end, ts_current); // pozostały czas na odbiór pakietów

		int ready = pselect(sockfd+1, &descriptors, NULL, NULL, &ts, NULL);

		if (ready < 0) {
			fprintf(stderr, "select error: %s\n", strerror(errno));
			return -1;
		}
		else if (ready == 0) { // timeout
			while (pck_cnt < ECHO_REQUESTS) {
				display[pck_cnt].timeout = 1;
				pck_cnt++;
			}
			return echoreply;
		}

		// Oblicz czas w jakim odebrano pakiet
		clock_gettime(CLOCK_MONOTONIC, &ts_current);
		ts = delta_timespec(ts_current, ts_start); // czas w jakim odebrano pakiet

		// Odbiór pakietów
		struct sockaddr_in 	sender;	
		socklen_t 			sender_len = sizeof(sender);
		u_int8_t 			buffer[IP_MAXPACKET];

		ssize_t packet_len = recvfrom (
			sockfd,
			buffer,
			IP_MAXPACKET,
			MSG_DONTWAIT,
			(struct sockaddr*)&sender,
			&sender_len
		);

		if (packet_len < 0) {
			fprintf(stderr, "recvfrom error: %s\n", strerror(errno)); 
			return -1;
		}

		// Odczyt nagłówka ICMP		
		struct iphdr* 	ip_header 	= (struct iphdr*) buffer;
		u_int8_t* 		icmp_packet = buffer + 4 * ip_header->ihl;
		struct icmphdr* icmp_header = (struct icmphdr*) icmp_packet;

		u_int8_t *ptr = buffer;
		ptr += 4 * ip_header->ihl + 8;
		struct iphdr*	prev_ip_header = (struct iphdr*) ptr;
		ptr += prev_ip_header->ihl * 4;
		struct icmphdr* prev_icmp_header = (struct icmphdr*) ptr;

		// TTL = j został odrzucony przez j-ty router
		if (icmp_header->type == ICMP_TIME_EXCEEDED) {
			if (prev_icmp_header->un.echo.id != pid ||
				prev_icmp_header->un.echo.sequence < ttl * ECHO_REQUESTS) {
				pck_cnt--;
				continue;
			}
			display[pck_cnt].sender = sender;
			display[pck_cnt].rtt = ts.tv_nsec;
		}

		// Odpowiedź komputera docelowego
		if (icmp_header->type == ICMP_ECHOREPLY) {
			if (icmp_header->un.echo.id != pid ||
				icmp_header->un.echo.sequence < ttl * ECHO_REQUESTS) {
				continue;
			}
			display[pck_cnt].sender = sender;
			display[pck_cnt].rtt = ts.tv_nsec;
			echoreply = 1;
		}
	}
	return echoreply;
}

void trace_print(struct packet_display* display) {

	int timeout_cnt = 0;
	for (int i = 0; i < ECHO_REQUESTS; i++) {
		if (display[i].timeout) timeout_cnt++;
	}

	if (timeout_cnt == ECHO_REQUESTS) {
		printf("*\n");
		return;
	}

	int *matched = (int*) calloc(ECHO_REQUESTS, sizeof(int));
	
	for (int i = 0; i < ECHO_REQUESTS; i++) {
		time_t sum_ns = 0; int cnt = 0;;
		
		if (matched[i] == 0) {
			for (int j = 0; j < ECHO_REQUESTS; j++) {
				if (!matched[j] &&
					(display[i].sender.sin_addr.s_addr ==
					display[j].sender.sin_addr.s_addr)) {

					sum_ns += display[j].rtt;
					timeout_cnt += display[j].timeout;
					matched[j] = 1;
					cnt++;
				}
			}

			struct sockaddr_in sender = display[i].sender;
			char sender_ip_str[20]; 
			inet_ntop(AF_INET, &(sender.sin_addr),
			 	sender_ip_str, sizeof(sender_ip_str));
			
			if (display[i].timeout) {
				printf ("(%s)  ???  ", sender_ip_str);
			}
			else {
				time_t avg_ns = sum_ns / cnt;
				printf("(%s)  %ld.%ld ms  ", sender_ip_str, avg_ns / MICRO_PER_SEC,
					(avg_ns / MILI_PER_SEC) % MILI_PER_SEC);
			}
		}
	}
	printf("\n");
	free(matched);
	return;
}

int traceroute(char* addr) {

	int fd, pid;

	if (init(&fd))
		return EXIT_FAILURE;

	pid = getpid();

	printf("traceroute to %s\n", addr);

	for (int ttl = 1; ttl <= MAX_TTL; ttl++) {
		struct packet_display disp[3];
		printf("%2d\t", ttl);

		for (int i = 0; i < ECHO_REQUESTS; i++) {
			int seq = ttl * ECHO_REQUESTS + i;
			icmp_send(fd, addr, ttl, pid, seq);
		}

 		int echo = icmp_receive(fd, pid, ttl, disp);
		trace_print(disp);

		if (echo) break;
	}

	return EXIT_SUCCESS;
}
