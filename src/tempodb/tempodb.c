#include "tempodb.h"

#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <inttypes.h>

static char access_key[ACCESS_KEY_SIZE + 1] = {0};
static char access_secret[ACCESS_KEY_SIZE + 1] = {0};
static int sock;
static struct sockaddr_in *addr;
static char *ip;
static int tempodb_send(const char *command);
static int tempodb_read_response(char *buffer, const int buffer_size);

static int tempodb_write(const char *query_buffer, char *response_buffer, const ssize_t response_buffer_size);
static int tempodb_write_by_path(const char *path, const float value, char *response_buffer, const ssize_t response_buffer_size);
static int tempodb_bulk_update(const struct tempodb_bulk_update *updates, ssize_t update_count, char *response_buffer, const ssize_t response_buffer_size, const char *path);

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

  snprintf(buffer, buffer_size, "%s %s HTTP/1.0\r\nAuthorization: Basic %s\r\nUser-Agent: tempodb-embedded-c/1.0.0\r\nHost: %s\r\nContent-Length: %lu\r\nContent-Type: application/json\r\n\r\n%s", verb, path, encoded_credentials, DOMAIN, (unsigned long)strlen(payload), payload);
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

static int tempodb_read_response(char *buffer, const int buffer_size) {
  size_t bytes_read = 0;
  size_t bytes_read_part;
  int status = 0;

  size_t remaining_buffer_size = buffer_size - 1;
  char *remaining_buffer = buffer;

  memset(buffer, 0, buffer_size);

  while ((bytes_read_part = recv(sock, remaining_buffer, remaining_buffer_size, 0)) > 0) {
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

int tempodb_send(const char *query) {
  int sent = 0;
  int sent_part;

  while (sent < strlen(query)) {
    sent_part = send(sock, query + sent, strlen(query) - sent, 0);
    if (sent_part == -1) {
      perror("Can't sent query");
      return -1;
    }
    sent += sent_part;
  }
  return 0;
}

int tempodb_bulk_increment(const struct tempodb_bulk_update *updates, ssize_t update_count, char *response_buffer, const ssize_t response_buffer_size) {
  return tempodb_bulk_update(updates, update_count, response_buffer, response_buffer_size, "/v1/increment");
}

int tempodb_bulk_write(const struct tempodb_bulk_update *updates, ssize_t update_count, char *response_buffer, const ssize_t response_buffer_size) {
  return tempodb_bulk_update(updates, update_count, response_buffer, response_buffer_size, "/v1/data");
}

int tempodb_bulk_update(const struct tempodb_bulk_update *updates, ssize_t update_count, char *response_buffer, const ssize_t response_buffer_size, const char *path) {
  char *query_buffer = (char *)malloc(512);
  char body_buffer[255];
  char *body_buffer_head = body_buffer;
  ssize_t body_buffer_size_remaining = 254;
  int chars_printed;
  int i;
  int status;
  char *type;

  sprintf(body_buffer_head, "{\"data\":[");
  body_buffer_head += strlen(body_buffer_head);

  /* two closing characters */
  body_buffer_size_remaining -= strlen(body_buffer_head) - 2;

  for (i = 0; i < update_count; i++) {
    switch(updates[i].type) {
      case TEMPODB_ID:
        type = "id";
        break;
      case TEMPODB_KEY:
        type = "key";
        break;
      default:
        perror("Invalid update type");
        type = "";
        break;
    }
    chars_printed = snprintf(body_buffer_head, body_buffer_size_remaining, "{\"%s\":\"%s\",\"v\":%f},", type, updates[i].series, (float)updates[i].value);
    body_buffer_size_remaining -= chars_printed;
    body_buffer_head += chars_printed;
  }

  snprintf(body_buffer_head, body_buffer_size_remaining, "]}");

  tempodb_build_query(query_buffer, 512, TEMPODB_POST, path, body_buffer);

  status = tempodb_write(query_buffer, response_buffer, response_buffer_size);

  free(query_buffer);
  return status;
}

int tempodb_increment_by_id(const char *series_id, const float value, char *response_buffer, const ssize_t response_buffer_size) {
  char path[255];
  snprintf(path, 255, "/v1/series/id/%s/increment", series_id);

  return tempodb_write_by_path(path, value, response_buffer, response_buffer_size);
}

int tempodb_increment_by_key(const char *series_key, const float value, char *response_buffer, const ssize_t response_buffer_size) {
  char path[255];
  snprintf(path, 255, "/v1/series/key/%s/increment", series_key);

  return tempodb_write_by_path(path, value, response_buffer, response_buffer_size);
}

int tempodb_write_by_key(const char *series_key, const float value, char *response_buffer, const ssize_t response_buffer_size) {
  char path[255];
  snprintf(path, 255, "/v1/series/key/%s/data", series_key);

  return tempodb_write_by_path(path, value, response_buffer, response_buffer_size);
}

int tempodb_write_by_id(const char *series_id, const float value, char *response_buffer, const ssize_t response_buffer_size) {
  char path[255];
  snprintf(path, 255, "/v1/series/id/%s/data", series_id);

  return tempodb_write_by_path(path, value, response_buffer, response_buffer_size);
}

int tempodb_write_by_path(const char *path, const float value, char *response_buffer, const ssize_t response_buffer_size) {
  char *query_buffer = (char *)malloc(512);
  char body_buffer[255];
  int status;

  snprintf(body_buffer, 255, "[{\"v\":%f}]", value);
  tempodb_build_query(query_buffer, 512, TEMPODB_POST, path, body_buffer);

  status = tempodb_write(query_buffer, response_buffer, response_buffer_size);

  free(query_buffer);
  return status;
}

int tempodb_write(const char *query_buffer, char *response_buffer, const ssize_t response_buffer_size) {
  int status = tempodb_send(query_buffer);
  if (status != -1) {
    status = tempodb_read_response(response_buffer, response_buffer_size);
  }
  return status;
}
