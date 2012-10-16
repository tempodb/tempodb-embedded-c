#include "CppUTest/TestHarness.h"

extern "C" {
#include "tempodb.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include "TCPSocketStub.h"
#include <string.h>
}

TEST_GROUP(tempodb) {
  void setup() {
    test_init();
    tempodb_create("my_access_key", "my_secret_key");
  }

  void teardown() {
    tempodb_destroy();
    test_cleanup();
  }
};

TEST(tempodb, WriteByKey_Request)
{
  char *response_buffer = (char *)malloc(255);
  tempodb_write_by_id("series_id", 10, response_buffer, 255);
  STRCMP_CONTAINS("POST /v1/series/id/series_id/data", last_request);
  STRCMP_CONTAINS("[{\"v\":10.000000}]", last_request);
  free(response_buffer);
}

TEST(tempodb, WriteByKey_Response)
{
  char *response_buffer = (char *)malloc(255);
  set_test_response("200 OK");
  tempodb_write_by_id("series_id", 10, response_buffer, 255);
  STRCMP_CONTAINS("200 OK", response_buffer);
  free(response_buffer);
}

TEST(tempodb, WriteByKey_Response_Overrun)
{
  char *response_buffer = (char *)malloc(255);
  memset(response_buffer, 1, 255);
  set_test_response("200 OK");
  tempodb_write_by_id("series_id", 10, response_buffer, 3);
  STRCMP_EQUAL("20", response_buffer);
  free(response_buffer);
}

TEST(tempodb, BuildQuery_IncludesHTTPVerbPathAndVersion)
{
  char *buffer = (char *)malloc(255);
  tempodb_build_query(buffer, 255, "GET", "/a/path", "");
  STRCMP_CONTAINS("GET /a/path HTTP/1.0", buffer);
  free(buffer);
}

TEST(tempodb, BuildQuery_DoesNotOverrun)
{
  char *buffer = (char *)malloc(255);
  memset(buffer, 1, 255);
  tempodb_build_query(buffer, 10, "GET", "/a/long/path", "");
  STRCMP_EQUAL("GET /a/lo", buffer);
  free(buffer);
}

TEST(tempodb, BuildQuery_IncludesUserAgent)
{
  char *buffer = (char *)malloc(255);
  tempodb_build_query(buffer, 255, "GET", "/a/path", "");
  STRCMP_CONTAINS("\r\nUser-Agent: tempodb-embedded-c/1.0.0\r\n", buffer);
  free(buffer);
}

TEST(tempodb, BuildQuery_IncludesHost)
{
  char *buffer = (char *)malloc(255);
  tempodb_build_query(buffer, 255, "GET", "/a/path", "");
  STRCMP_CONTAINS("\r\nHost: api.tempo-db.com\r\n", buffer);
  free(buffer);
}

TEST(tempodb, BuildQuery_IncludesPayload)
{
  char *buffer = (char *)malloc(255);
  tempodb_build_query(buffer, 255, "GET", "/a/path", "[{\"a\":1,\"b\":2}]");
  STRCMP_CONTAINS("\r\n\r\n[{\"a\":1,\"b\":2}]", buffer);
  free(buffer);
}

TEST(tempodb, BuildQuery_IncludesCredentials)
{
  char *buffer = (char *)malloc(255);
  tempodb_build_query(buffer, 255, "GET", "/a/path", "");
  STRCMP_CONTAINS("\r\nAuthorization: Basic bXlfYWNjZXNzX2tleTpteV9zZWNyZXRfa2V5\r\n", buffer);
  free(buffer);
}

TEST(tempodb, BuildQuery_IncludesContentType)
{
  char *buffer = (char *)malloc(255);
  tempodb_build_query(buffer, 255, "GET", "/a/path", "");
  STRCMP_CONTAINS("\r\nContent-Type: application/json\r\n", buffer);
  free(buffer);
}

TEST(tempodb, BuildQuery_IncludesContentLength)
{
  char *buffer = (char *)malloc(255);
  tempodb_build_query(buffer, 255, "POST", "/a/path", "123456");
  STRCMP_CONTAINS("\r\nContent-Length: 6\r\n", buffer);
  free(buffer);
}
