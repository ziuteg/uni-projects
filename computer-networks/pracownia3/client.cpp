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
#include <algorithm>
#include <sstream>
#include <fstream>
#include <vector>
#include <unistd.h>

#include "client.h"

#define SEGMENT_SIZE 1000
#define WINDOW_SIZE 100

static const char *server_addr = "156.17.4.30";

static std::vector < frame > window;
static int last_segment_received;

static std::ofstream file;
int file_size;

frame::frame(int frame_size) {
  data = new uint8_t[frame_size];
  received = false;
  size = 0;
}

int initialize(int port) {
	int sockfd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);

	if (sockfd < 0)
		throw std::runtime_error("Socket error");

	struct sockaddr_in server_address;
	bzero(&server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port);
	inet_pton(AF_INET, server_addr, &server_address.sin_addr);

	int connected = connect(
		sockfd,
		(struct sockaddr*) &server_address,
		sizeof(server_address)
	);

	if (connected < 0)
		throw std::runtime_error("Connect error");

	return sockfd;
}


void send_request(int sockfd, int port, int start, int length) {
	start = (last_segment_received) * SEGMENT_SIZE;
	for (int i = 0; i < WINDOW_SIZE && start < file_size; i++, start += SEGMENT_SIZE) {

		if (window[(last_segment_received + i) % WINDOW_SIZE].received)
			continue;

		length = std::min(SEGMENT_SIZE, file_size - start);

		std::string request = "GET " + std::to_string(start) + " " + std::to_string(length) + "\n";

		struct sockaddr_in recipient;
		bzero(&recipient, sizeof(recipient));
		recipient.sin_family = AF_INET;
		recipient.sin_port = htons(port);
		inet_pton(AF_INET, server_addr, &recipient.sin_addr);

		int send = sendto(
			sockfd,
			request.c_str(),
			strlen(request.c_str()),
			0,
			(struct sockaddr*) &recipient,
			sizeof(recipient)
		);

		if (send < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			throw std::runtime_error("Sendto error");
		}
	}
}


bool process_segment(uint8_t *buffer, size_t buffer_size) {
	std::string line;
	int start;
	int length;

	std::stringstream ss(std::string((char*)buffer, buffer_size));
	std::string str;
	ss >> str >> start >> length;
	char nl[2];
	ss.read(nl, 1);

	int index = start / SEGMENT_SIZE;

	if (index < last_segment_received)
		return false;
	if (index > last_segment_received + WINDOW_SIZE)
		return false;

	frame &segment = window[index % WINDOW_SIZE];

	if (segment.received)
		return false;

	segment.received = true;
	segment.size = length;
	ss.read((char*)segment.data, length);

	return true;
}


void save_data_to_file() {
	int index = last_segment_received % WINDOW_SIZE;
	while (window[index].received) {

		file.write((char*)window[index].data, window[index].size);

		window[index].received = false;
		index = (index + 1) % WINDOW_SIZE;
		last_segment_received++;
	}
}

int receive_data(int sockfd) {
	int cnt = 0;
	while (true) {
		int buffer_size = SEGMENT_SIZE + 100;
		uint8_t buffer[buffer_size];
		struct sockaddr_in sender;
		socklen_t sender_size = sizeof(sender);

		int recv_bytes = recvfrom(
			sockfd,
			buffer,
			buffer_size,
			0,
			(struct sockaddr*) &sender,
			&sender_size
		);

		if (recv_bytes < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			throw std::runtime_error("Recvfrom error");
		}
		if (process_segment(buffer, recv_bytes))
			cnt++;

		save_data_to_file();
	}
	return cnt;
}


void download_data(int sockfd, int port, int size) {

	int bytes_received = 0;

	while (bytes_received < size) {
		int start = bytes_received;
		int length = std::min(SEGMENT_SIZE, size - bytes_received);
		send_request(sockfd, port, start, length);
		usleep(10);

		int cnt = receive_data(sockfd);

		if (cnt > 0) {
			bytes_received += SEGMENT_SIZE * cnt;
			std::cout << "downloaded: " << std::min(size, bytes_received) << "/" << size << std::endl;
		}
	}
}


void transport(int port, std::string filename, int size) {

	file.open(filename, std::ios::out | std::ios::binary);
	int sockfd =  initialize(port);
	file_size = size;

	for (int i = 0; i < WINDOW_SIZE; i++)
		window.push_back(frame(SEGMENT_SIZE));

	download_data(sockfd, port, size);
	file.close();
}
