#include "CppUTest/TestHarness.h"

extern "C" {
#include "tempodb.h"
#include "platform_mock.h"
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

TEST(tempodb, BulkIncrement_Request)
{
  tempodb_config *config = tempodb_create("my_access_key", "my_secret_key");
  char *response_buffer = (char *)malloc(255);

  tempodb_bulk_update *updates = (tempodb_bulk_update *)malloc(sizeof(tempodb_bulk_update) * 2);
  tempodb_bulk_update update_by_id = { "series_1_id", TEMPODB_ID, 1.1};
  updates[0] = update_by_id;
  tempodb_bulk_update update_by_key = { "series_2_key", TEMPODB_KEY, 2};
  updates[1] = update_by_key;

  tempodb_bulk_increment(config, updates, 2, response_buffer, 255);
  STRCMP_CONTAINS("POST /v1/increment", last_request);
  STRCMP_CONTAINS("{\"data\":[{\"id\":\"series_1_id\",\"v\":1.100000},{\"key\":\"series_2_key\",\"v\":2.000000},]}", last_request);
  free(response_buffer);
  free(updates);
  tempodb_destroy(config);
}

TEST(tempodb, BulkWrite_Request)
{
  tempodb_config *config = tempodb_create("my_access_key", "my_secret_key");
  char *response_buffer = (char *)malloc(255);

  tempodb_bulk_update *updates = (tempodb_bulk_update *)malloc(sizeof(tempodb_bulk_update) * 2);
  tempodb_bulk_update update_by_id = { "series_1_id", TEMPODB_ID, 1.1};
  updates[0] = update_by_id;
  tempodb_bulk_update update_by_key = { "series_2_key", TEMPODB_KEY, 2};
  updates[1] = update_by_key;

  tempodb_bulk_write(config, updates, 2, response_buffer, 255);
  STRCMP_CONTAINS("POST /v1/data", last_request);
  STRCMP_CONTAINS("{\"data\":[{\"id\":\"series_1_id\",\"v\":1.100000},{\"key\":\"series_2_key\",\"v\":2.000000},]}", last_request);
  free(response_buffer);
  free(updates);
  tempodb_destroy(config);
}

TEST(tempodb, IncrementByKey_Request)
{
  tempodb_config *config = tempodb_create("my_access_key", "my_secret_key");
  char *response_buffer = (char *)malloc(255);
  tempodb_increment_by_key(config, "series_key", 10, response_buffer, 255);
  STRCMP_CONTAINS("POST /v1/series/key/series_key/increment", last_request);
  STRCMP_CONTAINS("[{\"v\":10.000000}]", last_request);
  free(response_buffer);
  tempodb_destroy(config);
}

TEST(tempodb, IncrementById_Request)
{
  tempodb_config *config = tempodb_create("my_access_key", "my_secret_key");
  char *response_buffer = (char *)malloc(255);
  tempodb_increment_by_id(config, "series_id", 10, response_buffer, 255);
  STRCMP_CONTAINS("POST /v1/series/id/series_id/increment", last_request);
  STRCMP_CONTAINS("[{\"v\":10.000000}]", last_request);
  free(response_buffer);
  tempodb_destroy(config);
}

TEST(tempodb, WriteByKey_Request)
{
  tempodb_config *config = tempodb_create("my_access_key", "my_secret_key");
  char *response_buffer = (char *)malloc(255);
  tempodb_write_by_key(config, "series_key", 10, response_buffer, 255);
  STRCMP_CONTAINS("POST /v1/series/key/series_key/data", last_request);
  STRCMP_CONTAINS("[{\"v\":10.000000}]", last_request);
  free(response_buffer);
  tempodb_destroy(config);
}

TEST(tempodb, WriteById_Request)
{
  tempodb_config *config = tempodb_create("my_access_key", "my_secret_key");
  char *response_buffer = (char *)malloc(255);
  tempodb_write_by_id(config, "series_id", 10, response_buffer, 255);
  STRCMP_CONTAINS("POST /v1/series/id/series_id/data", last_request);
  STRCMP_CONTAINS("[{\"v\":10.000000}]", last_request);
  free(response_buffer);
  tempodb_destroy(config);
}

TEST(tempodb, Write_Response)
{
  tempodb_config *config = tempodb_create("my_access_key", "my_secret_key");
  char *response_buffer = (char *)malloc(255);
  set_test_response("200 OK");
  tempodb_write_by_id(config, "series_id", 10, response_buffer, 255);
  STRCMP_CONTAINS("200 OK", response_buffer);
  free(response_buffer);
  tempodb_destroy(config);
}

TEST(tempodb, BuildQuery_IncludesHTTPVerbPathAndVersion)
{
  tempodb_config *config = tempodb_create("my_access_key", "my_secret_key");
  char *buffer = (char *)malloc(255);
  tempodb_build_query(config, buffer, 255, "GET", "/a/path", "");
  STRCMP_CONTAINS("GET /a/path HTTP/1.0", buffer);
  free(buffer);
  tempodb_destroy(config);
}

TEST(tempodb, BuildQuery_DoesNotOverrun)
{
  tempodb_config *config = tempodb_create("my_access_key", "my_secret_key");
  char *buffer = (char *)malloc(255);
  memset(buffer, 1, 255);
  tempodb_build_query(config, buffer, 10, "GET", "/a/long/path", "");
  STRCMP_EQUAL("GET /a/lo", buffer);
  free(buffer);
  tempodb_destroy(config);
}

TEST(tempodb, BuildQuery_IncludesUserAgent)
{
  tempodb_config *config = tempodb_create("my_access_key", "my_secret_key");
  char *buffer = (char *)malloc(255);
  tempodb_build_query(config, buffer, 255, "GET", "/a/path", "");
  STRCMP_CONTAINS("\r\nUser-Agent: tempodb-embedded-c/1.0.1\r\n", buffer);
  free(buffer);
  tempodb_destroy(config);
}

TEST(tempodb, BuildQuery_IncludesHost)
{
  tempodb_config *config = tempodb_create("my_access_key", "my_secret_key");
  char *buffer = (char *)malloc(255);
  tempodb_build_query(config, buffer, 255, "GET", "/a/path", "");
  STRCMP_CONTAINS("\r\nHost: api.tempo-db.com\r\n", buffer);
  free(buffer);
  tempodb_destroy(config);
}

TEST(tempodb, BuildQuery_IncludesPayload)
{
  tempodb_config *config = tempodb_create("my_access_key", "my_secret_key");
  char *buffer = (char *)malloc(255);
  tempodb_build_query(config, buffer, 255, "GET", "/a/path", "[{\"a\":1,\"b\":2}]");
  STRCMP_CONTAINS("\r\n\r\n[{\"a\":1,\"b\":2}]", buffer);
  free(buffer);
  tempodb_destroy(config);
}

TEST(tempodb, BuildQuery_IncludesCredentials)
{
  tempodb_config *config = tempodb_create("my_access_key", "my_secret_key");
  char *buffer = (char *)malloc(255);
  tempodb_build_query(config, buffer, 255, "GET", "/a/path", "");
  STRCMP_CONTAINS("\r\nAuthorization: Basic bXlfYWNjZXNzX2tleTpteV9zZWNyZXRfa2V5\r\n", buffer);
  free(buffer);
  tempodb_destroy(config);
}

TEST(tempodb, BuildQuery_IncludesContentType)
{
  tempodb_config *config = tempodb_create("my_access_key", "my_secret_key");
  char *buffer = (char *)malloc(255);
  tempodb_build_query(config, buffer, 255, "GET", "/a/path", "");
  STRCMP_CONTAINS("\r\nContent-Type: application/json\r\n", buffer);
  free(buffer);
  tempodb_destroy(config);
}

TEST(tempodb, BuildQuery_IncludesContentLength)
{
  tempodb_config *config = tempodb_create("my_access_key", "my_secret_key");
  char *buffer = (char *)malloc(255);
  tempodb_build_query(config, buffer, 255, "POST", "/a/path", "123456");
  STRCMP_CONTAINS("\r\nContent-Length: 6\r\n", buffer);
  free(buffer);
  tempodb_destroy(config);
}
