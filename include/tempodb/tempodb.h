#ifndef TempoDB_H
#define TempoDB_H

#include "base64.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define TEMPODB_GET "GET"
#define TEMPODB_POST "POST"

enum id_or_key {TEMPODB_ID, TEMPODB_KEY};

struct tempodb_bulk_update {
  const char *series;
  enum id_or_key type;
  float value;
};

void tempodb_create(const char *key, const char *secret);
void tempodb_destroy(void);

void tempodb_build_query(char *buffer, const size_t buffer_size, const char *verb, const char *path, const char *payload);

int tempodb_write_by_id(const char *series_id, const float value, char *response_buffer, const ssize_t response_buffer_size);
int tempodb_write_by_key(const char *series_key, const float value, char *response_buffer, const ssize_t response_buffer_size);

int tempodb_increment_by_key(const char *series_key, const float value, char *response_buffer, const ssize_t response_buffer_size);
int tempodb_increment_by_id(const char *series_id, const float value, char *response_buffer, const ssize_t response_buffer_size);

int tempodb_bulk_increment(const struct tempodb_bulk_update *updates, ssize_t update_count, char *response_buffer, const ssize_t response_buffer_size);
int tempodb_bulk_write(const struct tempodb_bulk_update *updates, ssize_t update_count, char *response_buffer, const ssize_t response_buffer_size);

#define ACCESS_KEY_SIZE 32

#define DOMAIN "api.tempo-db.com"

#endif
