
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "router.h"
#include "distance_vector.h"
#include "utilities.h"


int router_init(int *sockfd) {

	*sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (*sockfd < 0) {
		fprintf(stderr, "socket error: %s\n", strerror(errno)); 
		return EXIT_FAILURE;
	}

	/* umozliwiamy broadcast z danego gniazda */
	int broadcast_enable = 1;
	if (setsockopt(*sockfd, SOL_SOCKET, SO_BROADCAST,
		&broadcast_enable, sizeof(broadcast_enable))) {
		fprintf(stderr, "setsockopt error: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	struct sockaddr_in server_address;
	bzero (&server_address, sizeof(server_address));

	server_address.sin_family 		= AF_INET;
	server_address.sin_port 		= htons(ROUTER_PORT);
	server_address.sin_addr.s_addr 	= htonl(INADDR_ANY);

	/* przypisanie adresu do socketa */
	if (bind (*sockfd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
		fprintf(stderr, "bind error: %s\n", strerror(errno)); 
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

static unsigned int rnd = 1;

void router_print(struct dv_node *node) {

	printf("=== ROUND %d =================\n", rnd++);
	while (node != NULL) {
		char addr_str[20];
		char via_str[20];
		inet_ntop(AF_INET, &(node->net_addr), addr_str, sizeof(addr_str));
		inet_ntop(AF_INET, &(node->via), via_str, sizeof(via_str));

		printf("%s/%d ", addr_str, node->mask);

		printf("TIMEOUT %ld | ", node->timeout);
		if (node->unreachable)
			printf("unreachable ");
		else
			printf("distance %d ", node->dist);

		if (node->connected_directly && node->via == 0)
			printf("connected directly\n");
		else
			printf("via %s\n", via_str);

		node = node->next;
	}
}

/* 	
	Wysyla wszystkie elementy swojego aktualnego wektora
	odleglosci.
*/
int router_broadcast(int sockfd, struct net_table networks, struct dv_node *dist_vector) {

	for (size_t i = 0; i < networks.size; i++) {
		struct dv_node *node = dist_vector;
		while (node != NULL) {

			struct net_data entry = networks.entry[i];
			struct datagram datagr;

			datagr.net_addr = node->net_addr;
			datagr.mask = node->mask;
			datagr.dist = (node->unreachable ? 0xFFFFFFFF : node->dist);

			struct sockaddr_in server_address;
			bzero (&server_address, sizeof(server_address));

			server_address.sin_family 		= AF_INET;
			server_address.sin_port 		= htons(ROUTER_PORT);
			server_address.sin_addr.s_addr 	= entry.net_addr;

			if (sendto(sockfd, &datagr, sizeof(datagr), 0, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) {
				//fprintf(stderr, "sendto error: %s\n", strerror(errno)); 
				/* sieć nieosiągalna */
				struct dv_node *server_node = dv_find(dist_vector, entry.net_addr);
				if (server_node->connected_directly == 1)
					server_node->unreachable = 1;
			}
			node = node->next;
		}
	}
	return EXIT_SUCCESS;
}

/* liczy timeout
   jesli czas przekroczony i reachable, to ustawia unreachable
   jesli czas przekroczony i unreachable, to usuwa siec z dv */
void router_update(struct dv_node **dist_vector) {
	printf("hi\n");
	struct dv_node *node = (*dist_vector);
	while (node != NULL) {
		printf("fuck\n");

		if (node->timeout == 0) {
			if (node->unreachable == 1 && node->connected_directly == 0) {
				printf("a\n");
				struct dv_node *tmp = node->next;
				printf("b\n");
				dv_remove(dist_vector, node->net_addr);
				printf("c\n");
				node = tmp;
				continue;
			} else {
				if (node->connected_directly) node->via = 0;
				node->unreachable = 1;
				node->timeout = TIMEOUT;
			}
		}
		else node->timeout--;

		node = node->next;
	}
}


in_addr_t get_sender_network_addr(struct net_table networks, struct sockaddr_in sender) {
	in_addr_t sender_addr = sender.sin_addr.s_addr;
	for (size_t i = 0; i < networks.size; i++) {
		struct net_data entry = networks.entry[i];
		in_addr_t sender_net_addr = get_network_addr(sender_addr, entry.mask);
		if (entry.addr == sender_addr) // pakiet od siebie
			return 0;
		if (entry.net_addr == sender_net_addr) // wlasciwa siec
			return sender_net_addr;
	}
	return 0; // nie znaleziono sieci
}

int router_receive(int sockfd, struct net_table networks, struct dv_node *dist_vector) {

	fd_set descriptors;
	FD_ZERO (&descriptors);
	FD_SET (sockfd, &descriptors);

	struct timespec ts_end;
	struct timespec ts_start;
	clock_gettime(CLOCK_MONOTONIC, &ts_start);
	ts_end = ts_start;
	ts_end.tv_sec += ROUND_DELAY;

	for (;;) {

		// Czekanie maksymalnie ROUND_DELAY sekund na pakiety w gnieździe sockfd
		struct timespec ts_current;
		clock_gettime(CLOCK_MONOTONIC, &ts_current);
		struct timespec ts = delta_timespec(ts_end, ts_current); // pozostały czas na odbiór pakietów

		int ready = pselect(sockfd+1, &descriptors, NULL, NULL, &ts, NULL);

		if (ready < 0) {
			fprintf(stderr, "select error: %s\n", strerror(errno));
			return EXIT_FAILURE;
		}
		else if (ready == 0) {
			return EXIT_SUCCESS;
		}

		struct sockaddr_in 	sender;	
		socklen_t 			sender_len = sizeof(sender);

		uint8_t buffer[IP_MAXPACKET+1];

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
			return EXIT_FAILURE;
		}
		/* spr. czy odebrano wlasciwy pakiet */
		in_addr_t sender_addr = sender.sin_addr.s_addr;
		in_addr_t sender_net_addr = get_sender_network_addr(networks, sender);
		if (sender_net_addr == 0 || packet_len != 9) {
			continue;
		}
		/* zaktualizuj wektor odleglosci */
		struct datagram *datagr = (struct datagram *) buffer;
		dv_update(*datagr, sender_addr, sender_net_addr, networks, dist_vector);
	}

	return EXIT_SUCCESS;
}


int router(struct net_table networks) {

	int sockfd;
	if (router_init(&sockfd)) 
		return EXIT_FAILURE;

	/* inicjalizujemy liste wektora odleglosci */
	struct dv_node *dist_vector = dv_init(networks);

	for (;;) {
		router_print(dist_vector);
		router_broadcast(sockfd, networks, dist_vector);
		//printf("RECEIVE\n");
		router_receive(sockfd, networks, dist_vector);
		printf("UPDATE\n");
		router_update(&dist_vector);
	}

	dv_free(dist_vector);

	return EXIT_SUCCESS;
}