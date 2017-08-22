/*
 * Autor: Lukasz Siudek
 * Indeks: 283493
 */

#include <iostream>
#include <string>
#include <assert.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <cstring>
#include "client.h"

#define ARGUMENT_COUNT 4

static int port;
static std::string filename;
static int size;


void usage() {
	std::cerr << "Usage: transport <port> <file_name> <size>" << std::endl;
}


bool parse_input(int argc, char *argv[]) {

	if (argc != ARGUMENT_COUNT) return false;

	port = std::stoi(argv[1]);
	filename = std::string(argv[2]);
	size = std::stoi(argv[3]);

	if (port >= (1 << 16)) return false;
	if (port < 0) return false;
	if (size < 0) return false;

	return true;
}


int main(int argc, char *argv[]) {
	try {
		if (!parse_input(argc, argv)) {
			std::cerr << "Error: invalid input" << std::endl;
			usage();
			return 0;
		}
		transport(port, filename, size);
	}
	catch (const std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
	catch (...) {
		std::cerr << "Error: exception" << std::endl;
	}
	return 0;
}
