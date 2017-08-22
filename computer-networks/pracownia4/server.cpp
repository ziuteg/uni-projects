/**
 * Łukasz Siudek
 * 283493
 **/

#include <iostream>
#include <cstring>
#include <string>
#include <list>
#include <fstream>
#include <sstream>
#include <queue>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <chrono>
#include <thread>
#include "server.h"
#include "utils.h"

#define BACKLOG 10
#define CLIENT_TIMEOUT 2

namespace {
  const std::string STATUS_OK = "200 OK";
  const std::string STATUS_MP = "301 Moved Permanently";
  const std::string STATUS_FR = "403 Forbidden";
  const std::string STATUS_NF = "404 Not Found";
  const std::string STATUS_NI = "501 Not Implemented";

  const std::string FR_REPLY = "<html><body><h1>403 Forbidden</h1></body></html>";
  const std::string NF_REPLY = "<html><body><h1>404 Not Found</h1></body></html>";
  const std::string NI_REPLY = "<html><body><h1>501 Not Implemented</h1></body></html>";

  const std::vector <std::string> allowed_extensions =
    {"txt", "html", "css", "jpg", "jpeg", "png", "pdf"};
  const std::string OTHER_EXT = "application/octet-stream";

  int sockfd;
  std::ifstream file;
  std::list <int> connections;
  std::queue<request> requests;
}

request::request() {
  status = STATUS_OK;
}

void init(int port) {
  sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  if (sockfd < 0)
    throw std::runtime_error("Socket error");

  struct sockaddr_in server_address;
	std::memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family      = AF_INET;
	server_address.sin_port        = htons(port);
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sockfd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0)
		throw std::runtime_error("Bind error");

  if (listen(sockfd, BACKLOG) < 0)
    throw std::runtime_error("Listen error");
}

request parse_request(char *buffer, int buffer_size) {
  std::string str(buffer, buffer_size);
  std::stringstream ss(str);
  request req;
  size_t substr_begin;

  substr_begin = str.find("GET");
  if (substr_begin == std::string::npos)
    throw std::runtime_error("Parse error: no 'get' header");
  ss.seekg(substr_begin + sizeof("GET"), ss.beg);
  ss >> req.path;

  substr_begin = str.find("Host:");
  if (substr_begin == std::string::npos)
    throw std::runtime_error("Parse error: no 'host' header");
  ss.seekg(substr_begin + sizeof("Host:"), ss.beg);
  std::getline(ss, req.host, ':'); // oddzielanie adresu od portu

  substr_begin = str.find("Connection:");
  if (substr_begin == std::string::npos)
    throw std::runtime_error("Parse error: no 'connection' header");
  ss.seekg(substr_begin + sizeof("Connection:"), ss.beg);
  ss >> req.connection;

  return req;
}

void handle_requests() {
  fd_set descriptors;
  struct timeval timeout;
  std::queue <int> connections_to_remove;

  for (auto fd : connections) {
    FD_ZERO (&descriptors);
  	FD_SET (fd, &descriptors);
    timeout.tv_sec = CLIENT_TIMEOUT;
    timeout.tv_usec = 0;

    int ready = select(fd+1, &descriptors, NULL, NULL, &timeout);
    if (ready < 0)
      throw std::runtime_error("Select error");
    if (ready == 0) {
      connections_to_remove.push(fd);
      continue;
    }

    const int buffer_size = 1024;
    char buffer[buffer_size];
    int bytes_recvd = recv(fd, buffer, buffer_size, 0);
    if (bytes_recvd < 0) {
      throw std::runtime_error("Recv error");
    }
    if (bytes_recvd == 0) {
      connections_to_remove.push(fd);
      continue;
    }
    try {
      request req = parse_request(buffer, bytes_recvd);
      req.fd = fd;
      requests.push(req);
    }
    catch (const std::exception &e){
      std::cerr << e.what() << std::endl;
      request req;
      req.fd = fd;
      req.path = "*";
      req.status = STATUS_NI;
      requests.push(req);
    }
  }
  while (!connections_to_remove.empty()) {
    int fd = connections_to_remove.front();
    connections_to_remove.pop();
    connections.remove(fd);
    close(fd);
  }
}

void receive(int port) {
  struct sockaddr_in server_address;
	std::memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family      = AF_INET;
	server_address.sin_port        = htons(port);
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);

  while (true) {
    socklen_t sin_size = sizeof(server_address);
    int new_fd = accept(sockfd, (struct sockaddr*)&server_address, &sin_size);
    if (new_fd < 0) {
      if (errno == EWOULDBLOCK || errno == EAGAIN)
        break;
      throw std::runtime_error("Accept error");
    }
    connections.push_back(new_fd);
  }
  handle_requests();
}

void send_headers(request &req) {
  std::string str;
  std::stringstream ss;
  ss << "HTTP/1.1 " << req.status << "\r\n"
     << "Content-Length: " << req.reply_length << "\r\n"
     << "Content-Type: " << req.content_type << "\r\n"
     << "\r\n";
  str = ss.str();
  std::cout << "--HEADER--------\n";
  std::cout << str << std::endl;
  int bytes_sent = send(req.fd, str.c_str(), str.size(), 0);
  if (bytes_sent < 0)
    throw std::runtime_error("Send error");
}

void send_data(request &req) {
  if (req.status == STATUS_OK || req.status == STATUS_MP) {
    const int buffer_size = 16384;
    char buffer[buffer_size];
    while (!file.eof()) {
      std::cout << "SENDING\n";
      file.read(buffer, sizeof buffer);
      int bytes_read = file.gcount();
      int bytes_sent = send(req.fd, buffer, bytes_read, 0);
      if (bytes_sent < 0)
        throw std::runtime_error("Send error");
    }
    file.close();
  }
  else {
    std::string buffer;
    if (req.status == STATUS_FR)
      buffer = FR_REPLY;
    if (req.status == STATUS_NF)
      buffer = NF_REPLY;
    if (req.status == STATUS_NI)
      buffer = NI_REPLY;
    int bytes_sent = send(req.fd, buffer.c_str(), req.reply_length, 0);
    if (bytes_sent < 0)
      throw std::runtime_error("Send error");
  }
}

void check_type(request &req) {
  for (auto str : allowed_extensions)
    if (req.content_type.find(str))
      return;
  req.content_type = OTHER_EXT;
}

void find_file(request &req, std::string directory) {
  if (req.status == STATUS_NI) {
    req.content_type = "text/html";
    req.reply_length = NI_REPLY.size();
    return;
  }
  std::string path = directory + req.host + req.path;
  std::cout << "PATH: " << path << std::endl;
  if (is_directory(path)) {
    std::cout << "directory" << '\n';
    path += "/index.html";
    req.status = STATUS_MP;
  }
  if (is_file(path)) {
    std::cout << "is file" << '\n';
    file.open(path, std::ios::in | std::ios::binary);
    if (!file.good())
      throw std::runtime_error("File open error");
    req.reply_length = file_size(file);
    req.content_type = get_extension(path);
    std::cout << req.reply_length << " " << req.content_type << std::endl;
    check_type(req);
  }
  else {
    req.status = STATUS_NF;
    req.content_type = "text/html";
    req.reply_length = NF_REPLY.size();
  }
}

void respond(std::string directory) {
  while (!requests.empty()) {
    auto req = requests.front();
    requests.pop();
    if (req.connection == "close") {
      connections.remove(req.fd);
      continue;
    }
    find_file(req, directory);
    send_headers(req);
    send_data(req);
  }
}

void server(int port, std::string directory) {
  init(port);
  while (true) {
    receive(port);
    respond(directory);
    if (connections.empty()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }
}
