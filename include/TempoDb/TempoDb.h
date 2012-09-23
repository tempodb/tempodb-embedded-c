#ifndef D_LightController_H
#define D_LightController_H

#define GET "GET"
#define POST "POST"

#include "base64.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

void TempoDb_Create(const char *key, const char *secret);
void TempoDb_Destroy(void);

void TempoDb_BuildQuery(char *buffer, const size_t buffer_size, const char *verb, const char *path, const char *payload);
void TempoDb_WriteById(const char *seriesName, const float value);

#define ACCESS_KEY_SIZE 32

#define DOMAIN "api.tempo-db.com"

#endif
