#include "TempoDb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
  TempoDb_Create("my_access_key", "my_secret_key");
  TempoDb_WriteById("series_id", 10);
}
