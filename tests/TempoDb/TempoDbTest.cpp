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

TEST(tempodb, WriteByKey)
{
  tempodb_write_by_id("series_id", 10);
  STRCMP_CONTAINS("POST /v1/series/key/series_id/data", last_request);
  STRCMP_CONTAINS("[{\"v\":10.000000}]", last_request);
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
  STRCMP_CONTAINS("\nUser-Agent: tempodb-embedded-c/1.0.0\n", buffer);
  free(buffer);
}

TEST(tempodb, BuildQuery_IncludesHost)
{
  char *buffer = (char *)malloc(255);
  tempodb_build_query(buffer, 255, "GET", "/a/path", "");
  STRCMP_CONTAINS("\nHost: api.tempo-db.com\n", buffer);
  free(buffer);
}

TEST(tempodb, BuildQuery_IncludesPayload)
{
  char *buffer = (char *)malloc(255);
  tempodb_build_query(buffer, 255, "GET", "/a/path", "[{\"a\":1,\"b\":2}]");
  STRCMP_CONTAINS("\n\n[{\"a\":1,\"b\":2}]\n", buffer);
  free(buffer);
}

TEST(tempodb, BuildQuery_IncludesCredentials)
{
  char *buffer = (char *)malloc(255);
  tempodb_build_query(buffer, 255, "GET", "/a/path", "");
  STRCMP_CONTAINS("\nAuthorization: Basic bXlfYWNjZXNzX2tleTpteV9zZWNyZXRfa2V5\n", buffer);
  free(buffer);
}
