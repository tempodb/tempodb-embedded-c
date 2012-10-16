#ifndef D_LightController_H
#define D_LightController_H

#define GET "GET"
#define POST "POST"

#include "base64.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

void tempodb_create(const char *key, const char *secret);
void tempodb_destroy(void);

void tempodb_build_query(char *buffer, const size_t buffer_size, const char *verb, const char *path, const char *payload);
void tempodb_write_by_id(const char *series_id, const float value, char *response_buffer, const ssize_t response_buffer_size);

#define ACCESS_KEY_SIZE 32

#define DOMAIN "api.tempo-db.com"

#endif
