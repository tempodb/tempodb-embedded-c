#include "tempodb.h"
#include <stdio.h>

int main(int argc, char **argv) {
  tempodb_create("aafb854d6c404d3895e32d612417eb36", "b46d412e26b94bd3852e5721ff66a54d");

  char *response_buffer = (char *)malloc(255);
  tempodb_write_by_id("seriesid", 10, response_buffer, 255);
  printf("Response: %s\n", response_buffer);

  tempodb_destroy();
}
