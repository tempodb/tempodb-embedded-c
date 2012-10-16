#include "tempodb.h"

#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

static char access_key[ACCESS_KEY_SIZE + 1] = {0};
static char access_secret[ACCESS_KEY_SIZE + 1] = {0};
static int sock;
static struct sockaddr_in *addr;
static char *ip;
static void tempodb_send(const char *command);
static void tempodb_read_response(char *buffer, const int buffer_size);
static void tempodb_write(const char *query_buffer, char *response_buffer, const ssize_t response_buffer_size);
static void tempodb_write_by_path(const char *path, const float value, char *response_buffer, const ssize_t response_buffer_size);

static struct sockaddr_in * tempodb_addr(void);
static int tempodb_create_socket(struct sockaddr_in *addr);
static char * tempodb_getip(char *host);

void tempodb_create(const char *key, const char *secret)
{
  strncpy(access_key, key, ACCESS_KEY_SIZE);
  strncpy(access_secret, secret, ACCESS_KEY_SIZE);

  addr = tempodb_addr();
  sock = tempodb_create_socket(addr);
}

void tempodb_destroy(void)
{
  free(addr);
  free(ip);
}

static struct sockaddr_in * tempodb_addr(void) {
  int addr_result;
  struct sockaddr_in *remote;
  ip = tempodb_getip(DOMAIN);

  remote = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in *));
  remote->sin_family = AF_INET;
  remote->sin_port = htons(80);
  remote->sin_addr.s_addr = *ip;
  addr_result = inet_pton(AF_INET, ip, (void *)(&(remote->sin_addr.s_addr)));
  if( addr_result < 0)
  {
    perror("Can't set remote->sin_addr.s_addr");
    exit(1);
  }else if(addr_result == 0)
  {
    fprintf(stderr, "%s is not a valid IP address\n", ip);
    exit(1);
  }
  return remote;
}

void tempodb_build_query(char *buffer, const size_t buffer_size, const char *verb, const char *path, const char *payload) {
  char access_credentials[ACCESS_KEY_SIZE*2 + 2];
  char *encoded_credentials;
  snprintf(access_credentials, strlen(access_key) + strlen(access_secret) + 2, "%s:%s", access_key, access_secret);
  encoded_credentials = encode_base64(strlen(access_credentials), (unsigned char *)access_credentials);

  snprintf(buffer, buffer_size, "%s %s HTTP/1.0\r\nAuthorization: Basic %s\r\nUser-Agent: tempodb-embedded-c/1.0.0\r\nHost: %s\r\nContent-Length: %ld\r\nContent-Type: application/json\r\n\r\n%s", verb, path, encoded_credentials, DOMAIN, strlen(payload), payload);
  free(encoded_credentials);
}

static int tempodb_create_socket(struct sockaddr_in *addr) {
  int sock;
  if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
    perror("Can't create TCP socket");
    exit(1);
  }
  if(connect(sock, (struct sockaddr *)addr, sizeof(struct sockaddr)) < 0){
    perror("Could not connect");
    exit(1);
  }
  return sock;
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

static void tempodb_read_response(char *buffer, const int buffer_size) {
  size_t bytes_read = 0;
  size_t bytes_read_part;

  size_t remaining_buffer_size = buffer_size - 1;
  char *remaining_buffer = buffer;

  memset(buffer, 0, buffer_size);

  while ((bytes_read_part = recv(sock, remaining_buffer, remaining_buffer_size, 0)) > 0) {
    bytes_read += bytes_read_part;
    remaining_buffer_size -= bytes_read_part;
    remaining_buffer += bytes_read_part;
  }
}

static void tempodb_send(const char *query) {
  int sent = 0;
  int sent_part;

  while (sent < strlen(query)) {
    sent_part = send(sock, query + sent, strlen(query) - sent, 0);
    if (sent_part == -1) {
      perror("Can't sent query");
      exit(1);
    }
    sent += sent_part;
  }
}

void tempodb_bulk_write(const struct tempodb_bulk_update *updates, ssize_t update_count, char *response_buffer, const ssize_t response_buffer_size) {

  char *query_buffer = (char *)malloc(512);
  char body_buffer[255];
  char *body_buffer_head = body_buffer;
  ssize_t body_buffer_size_remaining = 254;
  int chars_printed;
  int i;

  sprintf(body_buffer_head, "{\"data\":[");
  body_buffer_head += strlen(body_buffer_head);

  /* two closing characters */
  body_buffer_size_remaining -= strlen(body_buffer_head) - 2;

  for (i = 0; i < update_count; i++) {
    chars_printed = snprintf(body_buffer_head, body_buffer_size_remaining, "{\"%s\":\"%s\",\"v\":%f},", updates[i].id_or_key, updates[i].series, (float)updates[i].value);
    body_buffer_size_remaining -= chars_printed;
    body_buffer_head += chars_printed;
  }

  snprintf(body_buffer_head, body_buffer_size_remaining, "]}");

  tempodb_build_query(query_buffer, 512, POST, "/v1/data", body_buffer);

  tempodb_write(query_buffer, response_buffer, response_buffer_size);

  free(query_buffer);
}

void tempodb_increment_by_id(const char *series_id, const float value, char *response_buffer, const ssize_t response_buffer_size) {
  char path[255];
  snprintf(path, 255, "/v1/series/id/%s/increment", series_id);

  tempodb_write_by_path(path, value, response_buffer, response_buffer_size);
}

void tempodb_increment_by_key(const char *series_key, const float value, char *response_buffer, const ssize_t response_buffer_size) {
  char path[255];
  snprintf(path, 255, "/v1/series/key/%s/increment", series_key);

  tempodb_write_by_path(path, value, response_buffer, response_buffer_size);
}

void tempodb_write_by_key(const char *series_key, const float value, char *response_buffer, const ssize_t response_buffer_size) {
  char path[255];
  snprintf(path, 255, "/v1/series/key/%s/data", series_key);

  tempodb_write_by_path(path, value, response_buffer, response_buffer_size);
}

void tempodb_write_by_id(const char *series_id, const float value, char *response_buffer, const ssize_t response_buffer_size) {
  char path[255];
  snprintf(path, 255, "/v1/series/id/%s/data", series_id);

  tempodb_write_by_path(path, value, response_buffer, response_buffer_size);
}

void tempodb_write_by_path(const char *path, const float value, char *response_buffer, const ssize_t response_buffer_size) {
  char *query_buffer = (char *)malloc(512);
  char body_buffer[255];

  snprintf(body_buffer, 255, "[{\"v\":%f}]", value);
  tempodb_build_query(query_buffer, 512, POST, path, body_buffer);

  tempodb_write(query_buffer, response_buffer, response_buffer_size);

  free(query_buffer);
}

void tempodb_write(const char *query_buffer, char *response_buffer, const ssize_t response_buffer_size) {
  tempodb_send(query_buffer);
  tempodb_read_response(response_buffer, response_buffer_size);
}
