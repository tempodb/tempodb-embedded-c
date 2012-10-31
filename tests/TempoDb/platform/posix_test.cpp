#include "CppUTest/TestHarness.h"

extern "C" {
#include "tempodb.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include "posix_mock.h"
#include <string.h>
}

TEST_GROUP(tempodb) {
  void setup() {
    test_init();
  }

  void teardown() {
    test_cleanup();
  }
};

TEST(tempodb, Write_Response_Overrun)
{
  tempodb_config *config = tempodb_create("my_access_key", "my_secret_key");
  char *response_buffer = (char *)malloc(255);
  memset(response_buffer, 1, 255);
  set_test_response("200 OK");
  tempodb_write_by_id(config, "series_id", 10, response_buffer, 3);
  STRCMP_EQUAL("20", response_buffer);
  free(response_buffer);
  tempodb_destroy(config);
}
