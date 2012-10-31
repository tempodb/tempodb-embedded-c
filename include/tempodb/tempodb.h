#ifndef TempoDB_H
#define TempoDB_H

#include "base64.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define TEMPODB_GET "GET"
#define TEMPODB_POST "POST"

typedef struct tempodb_config tempodb_config;
typedef struct tempodb_platform_config tempodb_platform_config;
typedef struct tempodb_bulk_update tempodb_bulk_update;

enum id_or_key {TEMPODB_ID, TEMPODB_KEY};

struct tempodb_bulk_update {
  const char *series;
  enum id_or_key type;
  float value;
};

/* defined by platform-specific files */
tempodb_platform_config * tempodb_platform_create(void);
int tempodb_platform_destroy(tempodb_platform_config *config);
int tempodb_platform_send(tempodb_platform_config *config, const char *command);
int tempodb_platform_read_response(tempodb_platform_config *config, char *buffer, const int buffer_size);

tempodb_config * tempodb_create(const char *key, const char *secret);
void tempodb_destroy(tempodb_config *config);

void tempodb_build_query(tempodb_config *config, char *buffer, const size_t buffer_size, const char *verb, const char *path, const char *payload);

int tempodb_write_by_id(tempodb_config *config, const char *series_id, const float value, char *response_buffer, const ssize_t response_buffer_size);
int tempodb_write_by_key(tempodb_config *config, const char *series_key, const float value, char *response_buffer, const ssize_t response_buffer_size);

int tempodb_increment_by_key(tempodb_config *config, const char *series_key, const float value, char *response_buffer, const ssize_t response_buffer_size);
int tempodb_increment_by_id(tempodb_config *config, const char *series_id, const float value, char *response_buffer, const ssize_t response_buffer_size);

int tempodb_bulk_increment(tempodb_config *config, const tempodb_bulk_update *updates, ssize_t update_count, char *response_buffer, const ssize_t response_buffer_size);
int tempodb_bulk_write(tempodb_config *config, const tempodb_bulk_update *updates, ssize_t update_count, char *response_buffer, const ssize_t response_buffer_size);

#define ACCESS_KEY_SIZE 32

#define DOMAIN "api.tempo-db.com"

#endif
