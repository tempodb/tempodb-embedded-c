#include "CppUTest/TestHarness.h"

extern "C" {
#include "TempoDb.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include "TCPSocketStub.h"
#include <string.h>
}

TEST_GROUP(TempoDb) {
  void setup() {
    test_init();
    TempoDb_Create("my_access_key", "my_secret_key");
  }

  void teardown() {
    TempoDb_Destroy();
    test_cleanup();
  }
};

TEST(TempoDb, WriteByKey)
{
  TempoDb_WriteById("series_id", 10);
  STRCMP_CONTAINS("POST /v1/series/key/series_id/data", last_request);
  STRCMP_CONTAINS("[{\"v\":10.000000}]", last_request);
}

TEST(TempoDb, BuildQuery_IncludesHTTPVerbPathAndVersion)
{
  char *buffer = (char *)malloc(255);
  TempoDb_BuildQuery(buffer, 255, "GET", "/a/path", "");
  STRCMP_CONTAINS("GET /a/path HTTP/1.0", buffer);
  free(buffer);
}

TEST(TempoDb, BuildQuery_DoesNotOverrun)
{
  char *buffer = (char *)malloc(255);
  memset(buffer, 1, 255);
  TempoDb_BuildQuery(buffer, 10, "GET", "/a/long/path", "");
  STRCMP_EQUAL("GET /a/lo", buffer);
  free(buffer);
}

TEST(TempoDb, BuildQuery_IncludesUserAgent)
{
  char *buffer = (char *)malloc(255);
  TempoDb_BuildQuery(buffer, 255, "GET", "/a/path", "");
  STRCMP_CONTAINS("\nUser-Agent: tempodb-embedded-c/1.0.0\n", buffer);
  free(buffer);
}

TEST(TempoDb, BuildQuery_IncludesHost)
{
  char *buffer = (char *)malloc(255);
  TempoDb_BuildQuery(buffer, 255, "GET", "/a/path", "");
  STRCMP_CONTAINS("\nHost: api.tempo-db.com\n", buffer);
  free(buffer);
}

TEST(TempoDb, BuildQuery_IncludesPayload)
{
  char *buffer = (char *)malloc(255);
  TempoDb_BuildQuery(buffer, 255, "GET", "/a/path", "[{\"a\":1,\"b\":2}]");
  STRCMP_CONTAINS("\n\n[{\"a\":1,\"b\":2}]\n", buffer);
  free(buffer);
}

TEST(TempoDb, BuildQuery_IncludesCredentials)
{
  char *buffer = (char *)malloc(255);
  TempoDb_BuildQuery(buffer, 255, "GET", "/a/path", "");
  STRCMP_CONTAINS("\nAuthorization: Basic bXlfYWNjZXNzX2tleTpteV9zZWNyZXRfa2V5\n", buffer);
  free(buffer);
}
