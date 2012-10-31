#include "platform_mock.h"

static size_t response_buffer_size = 1024;
char *response_buffer;

void test_init(void) {
  last_request = malloc(1);
  response_buffer = malloc(response_buffer_size + 1);
  memset(response_buffer, 0, response_buffer_size + 1);
  set_test_response("200 OK");
}

void test_cleanup(void) {
  free(last_request);
  free(response_buffer);
}

tempodb_platform_config * tempodb_platform_create(void) {
  return malloc(1);
}

int tempodb_platform_destroy(tempodb_platform_config *config) {
  free(config);
  return 0;
}

int tempodb_platform_send(tempodb_platform_config *config, const char *command) {
  free(last_request);
  last_request = malloc(strlen(command) + 1);
  strncpy(last_request, command, strlen(command));
  return 0;
}

int tempodb_platform_read_response(tempodb_platform_config *config, char *buffer, const int buffer_size) {
  int response_size = response_buffer_size;
  if (buffer_size < response_buffer_size) {
    response_size = buffer_size;
  }
  strncpy(buffer, response_buffer, response_size);
  return 0;
}

void set_test_response(const char *buffer) {
  strncpy(response_buffer, buffer, response_buffer_size);
}
