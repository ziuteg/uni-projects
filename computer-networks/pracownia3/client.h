#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <iostream>
#include <string>
#include <assert.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

struct frame {
  bool received;
  uint8_t *data;
  int size;

  frame(int frame_size);
};

int initialize(int port);

void send_request(int sockfd, int port, int start, int length);

bool process_segment(uint8_t *buffer, size_t buffer_size);

void save_data_to_file();

int receive_data(int sockfd);

void download_data(int sockfd, int port, int size);

void transport(int port, std::string filename, int size);

#endif
