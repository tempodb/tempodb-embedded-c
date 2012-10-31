#ifndef TEMPODB_PLATFORM_MOCK_H
#define TEMPODB_PLATFORM_MOCK_H

#include <string.h>
#include "tempodb.h"

char *last_request;

void test_init(void);
void test_cleanup(void);

void set_test_response(const char *buffer);

#endif
