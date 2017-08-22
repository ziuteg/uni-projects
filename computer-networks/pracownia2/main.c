
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "router.h"
#include "utilities.h"

int main() {

	int interface_cnt;
	char ip_addr[20];
	int mask;
	int dist;

	scanf("%d\n", &interface_cnt);
	struct net_table adj_networks;

	adj_networks.entry = malloc(sizeof(struct net_data) * interface_cnt);
	adj_networks.size = interface_cnt;

	for (int i = 0; i < interface_cnt; i++) {
		scanf("%[^/] %*c %d %*s %d\n", ip_addr, &mask, &dist);
		struct net_data net_entry;
		
		int err = inet_pton(AF_INET, ip_addr, &(net_entry.addr));

		if (err == 0) {
			fprintf(stderr, "error: invalid address\n");
			return EXIT_FAILURE;
		}

		net_entry.net_addr = get_network_addr(net_entry.addr, mask);
		net_entry.broadcast_addr = get_broadcast_addr(net_entry.addr, mask);
		net_entry.mask = mask;
		net_entry.dist = dist;
		adj_networks.entry[i] = net_entry;
	}

	router(adj_networks);
	
	free(adj_networks.entry);
	return EXIT_SUCCESS;
}
