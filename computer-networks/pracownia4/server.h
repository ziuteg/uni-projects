#ifndef __SERVER_H__
#define __SERVER_H__

#include <iostream>
#include <cstring>
#include <string>
#include <fstream>
#include <sstream>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <vector>
#include <queue>
#include <list>

struct request {
  int fd;
  int port;

  std::string status;
  std::string content_type;
  int reply_length;

  std::string path;
  std::string host;
  std::string connection;

  request();
};

void init(int port);
request parse_request(char *buffer, int buffer_size);
void handle_requests();
void receive(int port);
void send_headers(request &req);
void send_data(request &req);
void check_type(request &req);
void find_file(request &req, std::string directory);
void respond(std::string directory);
void server(int port, std::string directory);

#endif
