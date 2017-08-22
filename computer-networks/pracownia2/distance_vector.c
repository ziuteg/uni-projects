#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "utilities.h"
#include "distance_vector.h"


/* sprawdzamy, czy siec jest już w wektorze odleglosci
   jesli tak, to aktualizujemy wynik
   jesli nie, to dodajemy do wektora */
void dv_update(struct datagram data, in_addr_t sender_addr, in_addr_t sender_net_addr, struct net_table networks, struct dv_node *dist_vector) {

	if (data.dist >= INF) return;

	struct dv_node *node = dv_find(dist_vector, data.net_addr);
	struct dv_node *sender_node = dv_find(dist_vector, sender_net_addr);
	
	/* aktualizujemy timeout sieci z ktorej otrzymalismy pakiet */
	for (size_t i = 0; i < networks.size; i++) {
		struct net_data entry = networks.entry[i];


		if (entry.net_addr == sender_net_addr) { 
			if (sender_node->via != 0) {
				if (sender_node->dist >= entry.dist) {
					sender_node->unreachable = 0;
					sender_node->timeout = TIMEOUT;
					sender_node->via = 0;
					sender_node->dist = entry.dist;
				}
			}
			else {
				sender_node->dist = entry.dist;
				sender_node->unreachable = 0;
				sender_node->timeout = TIMEOUT;
			}
		}
	}

	if (node == NULL) {
		/* sieci nie ma w wektorze */
		printf("\t ____________\n");
		printf("\t| NEW NETWORK\n");
		char str[20];
		inet_ntop(AF_INET, &(data.net_addr), str, sizeof(str));
		printf("\t| addr: %s\n\t| mask: %x\n\t| dist: %u\n", str, data.mask, data.dist);
		struct dv_node *new_node = dv_sent(data, sender_node);
		new_node->via = sender_addr;
		dv_insert(dist_vector, new_node);
		printf("oppa\n");
	}
	else {
		/* siec jest w wektorze */
		uint32_t new_dist = data.dist + sender_node->dist;
		if (new_dist < node->dist || node->unreachable) {
			if (new_dist < INF) {
				node->dist = new_dist;
				node->via = sender_addr;
				node->timeout = TIMEOUT;
				node->unreachable = 0;
				/* czy update dotyczy bezposrednio polaczonej sieci */
				//node->connected_directly = 0;
			}
		}
		else if (new_dist == node->dist && new_dist < INF) {
			node->timeout = TIMEOUT;
		}
	}
}


/* tworzy element listy na podstawie struktury net_data */
struct dv_node *dv(struct net_data entry) {
	struct dv_node *node = malloc(sizeof(struct dv_node));
	node->net_addr = entry.net_addr;
	node->mask = entry.mask;
	node->dist = entry.dist;
	node->connected_directly = 1;
	node->via = 0;
	node->unreachable = 0;
	node->timeout = TIMEOUT;
	return node; 
}

/* tworzy element listy na podstawie struktury datagram */
struct dv_node *dv_sent(struct datagram datagr, struct dv_node *sender_node) {
	struct dv_node *node = malloc(sizeof(struct dv_node));
	node->net_addr = datagr.net_addr;
	node->mask = datagr.mask;
	node->dist = datagr.dist + sender_node->dist;
	node->connected_directly = 0;
	node->unreachable = node->dist >= INF ? 1 : 0;
	node->timeout = TIMEOUT;
	return node; 
}

/* funkcja tworzy wstepna liste wektora odleglosci na podstawie
   tablicy polaczonych bezposrednio sieci */
struct dv_node *dv_init(struct net_table networks) {
	struct dv_node *head = dv(networks.entry[0]);
	for (size_t i = 1; i < networks.size; i++) {
		struct dv_node *new_node = dv(networks.entry[i]);
		new_node->connected_directly = 1;
		dv_insert(head, new_node);
	}
	return head;
}

/* sprawdza, czy adres sieci znajduje sie w wektorze odleglosci */
struct dv_node *dv_find(struct dv_node *node, in_addr_t addr) {
	while (node != NULL) {
		if (node->net_addr == addr)
			return node;
		node = node->next;
	}
	return node;
}

/* wstawia element na koniec listy */
void dv_insert(struct dv_node *node, struct dv_node *new_node) {
	while (node->next != NULL)
		node = node->next;
	node->next = new_node;
}

/* usuwa element o podanym adresie sieci */
void dv_remove(struct dv_node **head_ptr, in_addr_t addr) {
	struct dv_node *node = (*head_ptr);
	if (node == NULL)
		return;
	struct dv_node *rm_node = dv_find(node, addr);
	if (node == rm_node) {
		(*head_ptr) = rm_node->next;
		dv_free(rm_node);
		return;
	}
	while (node->next != rm_node)
		node = node->next;
	node->next = rm_node->next;
	free(rm_node);
}

/* zwalnia wszystkie elementy listy */
void dv_free(struct dv_node *node) {
	if (node == NULL)
		return;
	if (node->next != NULL)
		dv_free(node->next);
	free(node);
}