#include "tempodb.h"
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

static int tempodb_create_socket(void);
static struct sockaddr_in * tempodb_addr(void);
static char * tempodb_getip(char *host);

struct tempodb_platform_config {
  int sock;
};

tempodb_platform_config * tempodb_platform_create(void) {
  tempodb_platform_config *config = (tempodb_platform_config *)malloc(sizeof(tempodb_platform_config));
  config->sock = tempodb_create_socket();
  return config;
}

int tempodb_platform_destroy(tempodb_platform_config *config)
{
  free(config);
  return 0;
}

int tempodb_platform_read_response(tempodb_platform_config *config, char *buffer, const int buffer_size) {
  size_t bytes_read = 0;
  size_t bytes_read_part;
  int status = 0;

  size_t remaining_buffer_size = buffer_size - 1;
  char *remaining_buffer = buffer;

  memset(buffer, 0, buffer_size);

  while ((bytes_read_part = recv(config->sock, remaining_buffer, remaining_buffer_size, 0)) > 0) {
    if (bytes_read_part == -1) {
      status = -1;
      perror("Can't read from socket");
    }
    bytes_read += bytes_read_part;
    remaining_buffer_size -= bytes_read_part;
    remaining_buffer += bytes_read_part;
  }
  return status;
}

int tempodb_platform_send(tempodb_platform_config *config, const char *query) {
  int sent = 0;
  int sent_part;

  while (sent < strlen(query)) {
    sent_part = send(config->sock, query + sent, strlen(query) - sent, 0);
    if (sent_part == -1) {
      perror("Can't send query");
      return -1;
    }
    sent += sent_part;
  }
  return 0;
}

static int tempodb_create_socket(void) {
  struct sockaddr_in *addr = tempodb_addr();
  int sock;
  if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
    perror("Can't create TCP socket");
    exit(1);
  }
  if(connect(sock, (struct sockaddr *)addr, sizeof(struct sockaddr)) < 0){
    perror("Could not connect");
    exit(1);
  }
  free(addr);
  return sock;
}

static struct sockaddr_in * tempodb_addr(void) {
  struct sockaddr_in *remote;
  char *ip = tempodb_getip(DOMAIN);

  remote = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
  remote->sin_family = AF_INET;
  remote->sin_port = htons(80);
  remote->sin_addr.s_addr = inet_addr(ip);

  if (remote->sin_addr.s_addr == INADDR_NONE) {
    fprintf(stderr, "%s is not a valid IP address\n", ip);
    exit(1);
  }

  free(ip);
  return remote;
}

static char * tempodb_getip(char *host)
{
  struct hostent *hent;
  int iplen = 16;
  char *ip = (char *)malloc(iplen+1);
  memset(ip, 0, iplen+1);
  if((hent = gethostbyname(host)) == NULL)
  {
    herror("Can't get IP");
    exit(1);
  }
  if(inet_ntop(AF_INET, (void *)hent->h_addr_list[0], ip, iplen) == NULL)
  {
    perror("Can't resolve host");
    exit(1);
  }
  return ip;
}

