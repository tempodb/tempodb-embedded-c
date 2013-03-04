#include "tempodb.h"

#include <string.h>
#include <stdio.h>
#include <inttypes.h>

static int tempodb_write(tempodb_config *config, const char *query_buffer, char *response_buffer, const ssize_t response_buffer_size);
static int tempodb_write_by_path(tempodb_config *config, const char *path, const float value, char *response_buffer, const ssize_t response_buffer_size);
static int tempodb_send_bulk_update(tempodb_config *config, const tempodb_bulk_update *updates, ssize_t update_count, char *response_buffer, const ssize_t response_buffer_size, const char *path);

struct tempodb_config {
  char *access_key;
  char *access_secret;
  tempodb_platform_config *platform_config;
};

tempodb_config * tempodb_create(const char *key, const char *secret)
{
  tempodb_config *config = (tempodb_config *)malloc(sizeof(tempodb_config));
  config->access_key = (char *)malloc(sizeof(char) * (ACCESS_KEY_SIZE + 1));
  config->access_secret = (char *)malloc(sizeof(char) * (ACCESS_KEY_SIZE + 1));
  strncpy(config->access_key, key, ACCESS_KEY_SIZE + 1);
  strncpy(config->access_secret, secret, ACCESS_KEY_SIZE + 1);

  config->platform_config = tempodb_platform_create();

  return config;
}

void tempodb_destroy(tempodb_config *config)
{
  tempodb_platform_destroy(config->platform_config);
  free(config->access_key);
  free(config->access_secret);
  free(config);
}

void tempodb_build_query(tempodb_config *config, char *buffer, const size_t buffer_size, const char *verb, const char *path, const char *payload) {
  char access_credentials[ACCESS_KEY_SIZE*2 + 2];
  char *encoded_credentials;
  snprintf(access_credentials, strlen(config->access_key) + strlen(config->access_secret) + 2, "%s:%s", config->access_key, config->access_secret);
  encoded_credentials = encode_base64(strlen(access_credentials), (unsigned char *)access_credentials);

  snprintf(buffer, buffer_size, "%s %s HTTP/1.0\r\nAuthorization: Basic %s\r\nUser-Agent: tempodb-embedded-c/1.0.0\r\nHost: %s\r\nContent-Length: %lu\r\nContent-Type: application/json\r\n\r\n%s", verb, path, encoded_credentials, DOMAIN, (unsigned long)strlen(payload), payload);
  free(encoded_credentials);
}

int tempodb_bulk_increment(tempodb_config *config, const tempodb_bulk_update *updates, ssize_t update_count, char *response_buffer, const ssize_t response_buffer_size) {
  return tempodb_send_bulk_update(config, updates, update_count, response_buffer, response_buffer_size, "/v1/increment");
}

int tempodb_bulk_write(tempodb_config *config, const tempodb_bulk_update *updates, ssize_t update_count, char *response_buffer, const ssize_t response_buffer_size) {
  return tempodb_send_bulk_update(config, updates, update_count, response_buffer, response_buffer_size, "/v1/data");
}

int tempodb_send_bulk_update(tempodb_config *config, const tempodb_bulk_update *updates, ssize_t update_count, char *response_buffer, const ssize_t response_buffer_size, const char *path) {
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

int tempodb_increment_by_id(tempodb_config *config, const char *series_id, const float value, char *response_buffer, const ssize_t response_buffer_size) {
  char path[255];
  snprintf(path, 255, "/v1/series/id/%s/increment", series_id);

  return tempodb_write_by_path(config, path, value, response_buffer, response_buffer_size);
}

int tempodb_increment_by_key(tempodb_config *config, const char *series_key, const float value, char *response_buffer, const ssize_t response_buffer_size) {
  char path[255];
  snprintf(path, 255, "/v1/series/key/%s/increment", series_key);

  return tempodb_write_by_path(config, path, value, response_buffer, response_buffer_size);
}

int tempodb_write_by_key(tempodb_config *config, const char *series_key, const float value, char *response_buffer, const ssize_t response_buffer_size) {
  char path[255];
  snprintf(path, 255, "/v1/series/key/%s/data", series_key);

  return tempodb_write_by_path(config, path, value, response_buffer, response_buffer_size);
}

int tempodb_write_by_id(tempodb_config *config, const char *series_id, const float value, char *response_buffer, const ssize_t response_buffer_size) {
  char path[255];
  snprintf(path, 255, "/v1/series/id/%s/data", series_id);

  return tempodb_write_by_path(config, path, value, response_buffer, response_buffer_size);
}

int tempodb_write_by_path(tempodb_config *config, const char *path, const float value, char *response_buffer, const ssize_t response_buffer_size) {
  char *query_buffer = (char *)malloc(512);
  char body_buffer[255];
  int status;

  snprintf(body_buffer, 255, "[{\"v\":%f}]", value);
  tempodb_build_query(config, query_buffer, 512, TEMPODB_POST, path, body_buffer);

  status = tempodb_write(config, query_buffer, response_buffer, response_buffer_size);

  free(query_buffer);
  return status;
}

int tempodb_write(tempodb_config *config, const char *query_buffer, char *response_buffer, const ssize_t response_buffer_size) {
  int status;
  tempodb_platform_open_socket(config->platform_config);
  status = tempodb_platform_send(config->platform_config, query_buffer);
  if (status != -1) {
    status = tempodb_platform_read_response(config->platform_config, response_buffer, response_buffer_size);
  }
  tempodb_platform_close_socket(config->platform_config);
  return status;
}
