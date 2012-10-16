#include "tempodb.h"
#include <stdio.h>

int main(int argc, char **argv) {
  tempodb_create("key", "secret");

  char *response_buffer = (char *)malloc(255);
  tempodb_write_by_key("my_favorite_series", 10, response_buffer, 255);
  printf("Response: %s\n", response_buffer);

  tempodb_destroy();
}
