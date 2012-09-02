#include "CppUTest/TestHarness.h"

extern "C" {
#include "TempoDb.h"
}

TEST_GROUP(TempoDb) {
  void setup() {
  }
  void teardown() {
  }
};

TEST(TempoDb, FirstTest)
{
  LONGS_EQUAL(0, 0);
}
