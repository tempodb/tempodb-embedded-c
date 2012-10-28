#include "tempodb.h"

#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <inttypes.h>

static int tempodb_send(struct tempodb_config *config, const char *command);
static int tempodb_read_response(struct tempodb_config *config, char *buffer, const int buffer_size);

static int tempodb_write(struct tempodb_config *config, const char *query_buffer, char *response_buffer, const ssize_t response_buffer_size);
static int tempodb_write_by_path(struct tempodb_config *config, const char *path, const float value, char *response_buffer, const ssize_t response_buffer_size);
static int tempodb_bulk_update(struct tempodb_config *config, const struct tempodb_bulk_update *updates, ssize_t update_count, char *response_buffer, const ssize_t response_buffer_size, const char *path);

static struct sockaddr_in * tempodb_addr(void);
static int tempodb_create_socket(void);
static char * tempodb_getip(char *host);

struct tempodb_config {
  char *access_key;
  char *access_secret;
  int sock;
};

struct tempodb_config * tempodb_create(const char *key, const char *secret)
{
  struct tempodb_config *config = (struct tempodb_config *)malloc(sizeof(struct tempodb_config));
  config->access_key = (char *)malloc(sizeof(char) * (ACCESS_KEY_SIZE + 1));
  config->access_secret = (char *)malloc(sizeof(char) * (ACCESS_KEY_SIZE + 1));
  strncpy(config->access_key, key, ACCESS_KEY_SIZE + 1);
  strncpy(config->access_secret, secret, ACCESS_KEY_SIZE + 1);

  config->sock = tempodb_create_socket();

  return config;
}

void tempodb_destroy(struct tempodb_config *config)
{
  free(config->access_key);
  free(config->access_secret);
  free(config);
}

static struct sockaddr_in * tempodb_addr(void) {
  struct sockaddr_in *remote;
  char *ip = tempodb_getip(DOMAIN);

  remote = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in *));
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

void tempodb_build_query(struct tempodb_config *config, char *buffer, const size_t buffer_size, const char *verb, const char *path, const char *payload) {
  char access_credentials[ACCESS_KEY_SIZE*2 + 2];
  char *encoded_credentials;
  snprintf(access_credentials, strlen(config->access_key) + strlen(config->access_secret) + 2, "%s:%s", config->access_key, config->access_secret);
  encoded_credentials = encode_base64(strlen(access_credentials), (unsigned char *)access_credentials);

  snprintf(buffer, buffer_size, "%s %s HTTP/1.0\r\nAuthorization: Basic %s\r\nUser-Agent: tempodb-embedded-c/1.0.0\r\nHost: %s\r\nContent-Length: %lu\r\nContent-Type: application/json\r\n\r\n%s", verb, path, encoded_credentials, DOMAIN, (unsigned long)strlen(payload), payload);
  free(encoded_credentials);
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

static int tempodb_read_response(struct tempodb_config *config, char *buffer, const int buffer_size) {
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

int tempodb_send(struct tempodb_config *config, const char *query) {
  int sent = 0;
  int sent_part;

  while (sent < strlen(query)) {
    sent_part = send(config->sock, query + sent, strlen(query) - sent, 0);
    if (sent_part == -1) {
      perror("Can't sent query");
      return -1;
    }
    sent += sent_part;
  }
  return 0;
}

int tempodb_bulk_increment(struct tempodb_config *config, const struct tempodb_bulk_update *updates, ssize_t update_count, char *response_buffer, const ssize_t response_buffer_size) {
  return tempodb_bulk_update(config, updates, update_count, response_buffer, response_buffer_size, "/v1/increment");
}

int tempodb_bulk_write(struct tempodb_config *config, const struct tempodb_bulk_update *updates, ssize_t update_count, char *response_buffer, const ssize_t response_buffer_size) {
  return tempodb_bulk_update(config, updates, update_count, response_buffer, response_buffer_size, "/v1/data");
}

int tempodb_bulk_update(struct tempodb_config *config, const struct tempodb_bulk_update *updates, ssize_t update_count, char *response_buffer, const ssize_t response_buffer_size, const char *path) {
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

  tempodb_build_query(config, query_buffer, 512, TEMPODB_POST, path, body_buffer);

  status = tempodb_write(config, query_buffer, response_buffer, response_buffer_size);

  free(query_buffer);
  return status;
}

int tempodb_increment_by_id(struct tempodb_config *config, const char *series_id, const float value, char *response_buffer, const ssize_t response_buffer_size) {
  char path[255];
  snprintf(path, 255, "/v1/series/id/%s/increment", series_id);

  return tempodb_write_by_path(config, path, value, response_buffer, response_buffer_size);
}

int tempodb_increment_by_key(struct tempodb_config *config, const char *series_key, const float value, char *response_buffer, const ssize_t response_buffer_size) {
  char path[255];
  snprintf(path, 255, "/v1/series/key/%s/increment", series_key);

  return tempodb_write_by_path(config, path, value, response_buffer, response_buffer_size);
}

int tempodb_write_by_key(struct tempodb_config *config, const char *series_key, const float value, char *response_buffer, const ssize_t response_buffer_size) {
  char path[255];
  snprintf(path, 255, "/v1/series/key/%s/data", series_key);

  return tempodb_write_by_path(config, path, value, response_buffer, response_buffer_size);
}

int tempodb_write_by_id(struct tempodb_config *config, const char *series_id, const float value, char *response_buffer, const ssize_t response_buffer_size) {
  char path[255];
  snprintf(path, 255, "/v1/series/id/%s/data", series_id);

  return tempodb_write_by_path(config, path, value, response_buffer, response_buffer_size);
}

int tempodb_write_by_path(struct tempodb_config *config, const char *path, const float value, char *response_buffer, const ssize_t response_buffer_size) {
  char *query_buffer = (char *)malloc(512);
  char body_buffer[255];
  int status;

  snprintf(body_buffer, 255, "[{\"v\":%f}]", value);
  tempodb_build_query(config, query_buffer, 512, TEMPODB_POST, path, body_buffer);

  status = tempodb_write(config, query_buffer, response_buffer, response_buffer_size);

  free(query_buffer);
  return status;
}

int tempodb_write(struct tempodb_config *config, const char *query_buffer, char *response_buffer, const ssize_t response_buffer_size) {
  int status = tempodb_send(config, query_buffer);
  if (status != -1) {
    status = tempodb_read_response(config, response_buffer, response_buffer_size);
  }
  return status;
}
