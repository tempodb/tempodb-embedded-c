#ifndef TempoDB_H
#define TempoDB_H

#define GET "GET"
#define POST "POST"

#define ID "id"
#define KEY "key"

#include "base64.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

struct tempodb_bulk_update {
  const char *series;
  const char *id_or_key;
  float value;
};

void tempodb_create(const char *key, const char *secret);
void tempodb_destroy(void);

void tempodb_build_query(char *buffer, const size_t buffer_size, const char *verb, const char *path, const char *payload);

void tempodb_write_by_id(const char *series_id, const float value, char *response_buffer, const ssize_t response_buffer_size);
void tempodb_write_by_key(const char *series_key, const float value, char *response_buffer, const ssize_t response_buffer_size);

void tempodb_increment_by_key(const char *series_key, const float value, char *response_buffer, const ssize_t response_buffer_size);
void tempodb_increment_by_id(const char *series_id, const float value, char *response_buffer, const ssize_t response_buffer_size);

void tempodb_bulk_increment(const struct tempodb_bulk_update *updates, ssize_t update_count, char *response_buffer, const ssize_t response_buffer_size);
void tempodb_bulk_write(const struct tempodb_bulk_update *updates, ssize_t update_count, char *response_buffer, const ssize_t response_buffer_size);

#define ACCESS_KEY_SIZE 32

#define DOMAIN "api.tempo-db.com"

#endif
