/** 
 * 	Zadanie: traceroute
 * 	Autor: Lukasz Siudek
 * 	Indeks: 283493
 **/

#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "traceroute.h"

int parse_input(int argc, char **argv) {

	if (argc != 2) {
		fprintf(stderr, "parse error: Expected one argument\n");
		return EXIT_FAILURE;
	}

	struct sockaddr_in sa;

	int ret = inet_pton(AF_INET, argv[1], &(sa.sin_addr));

	if (ret == 0) {
		fprintf(stderr, "parse error: Invalid address\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int main(int argc, char **argv) {

	if (parse_input(argc, argv)) {
		fprintf(stderr, "Usage: %s [IP Address]\n", argv[0]);
    	return EXIT_FAILURE;
	}

	traceroute(argv[1]);

	return EXIT_SUCCESS;
}
