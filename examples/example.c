#include "tempodb.h"
#include <stdio.h>

/*
 * TempoDB C Library
 * www.tempo-db.com
 *
 * Use this library to send floats to your TempoDB database, associated with a given series key (or, id).
 *
 * The general strategy is:
 *
 * 1) Initialize the connection with:
 *    struct tempodb_config *config = tempodb_create("YOUR_KEY", "YOUR_SECRET");
 *
 * 2) Make one or more requests:
 *    status = tempodb_write_by_key(config, "temperature", 45.3, response_buffer, 255);
 *    if (status == -1) {
 *      perror("Error sending to tempodb:");
 *    } else {
 *      printf("Response: %s\n", response_buffer);
 *    }
 *
 * 3) Free TempoDB allocations
 *    tempodb_destroy(config);
 *
 *
 * An important note on timestamps:
 * Because this library is targeted at small, embedded devices,
 * it does not require the system to know the current time and
 * instead relies on the TempoDB servers to timestamp the values
 * as they come in. This means that the value gets assigned a
 * timestamp slightly after it left your system, and multiple
 * values may arrive at TempoDB at the same time. There are two
 * important take-aways from this:
 * 1) If you are doing high-resolution sampling (more than once
 *    a minute), the slight, somewhat inconsistent delay due to
 *    internet latencies may affect you.
 * 2) It is possible that two values sent at different times will
 *    arrive at TempoDB at the same time. If you want TempoDB to
 *    overwrite one value with the other, use the "write"
 *    functions. If you want TempoDB to sum the values, use the
 *    "increment" functions.
 */

int main(int argc, char **argv) {
  struct tempodb_config *config = tempodb_create("YOUR_KEY", "YOUR_SECRET");

  char *response_buffer = (char *)malloc(255);
  int status;

  /*
   * Write By Key
   * Used to send a single value to TempoDB.
   *
   * If a value already exists at the same timestamp, it will be overwritten.
   *
   * This type of request makes sense if you have a sensor that you are collecting values for, like a thermometer.
   *
   * The series (eg "temperature") will be created if it does not yet exist
   *
   * Also available: tempodb_write_by_id, which takes an TempoDB-generated id instead of the user-specified key
   */
  status = tempodb_write_by_key(config, "temperature", 45.3, response_buffer, 255);

  /*
   * Increment By Key
   * Used to send a single value to TempoDB.
   *
   * If a value already exists at the same timestamp, it will be incremented (by the value specified).
   * If a value does not already exist at the same timestamp, it gets written normally.
   *
   * This type of request makes sense if you are counting something, like button clicks.
   * See the note (below) on how timestamps work.
   *
   * The series (eg "my_series") will be created if it does not yet exist
   *
   * Also available: tempodb_increment_by_id, which takes an TempoDB-generated id instead of the user-specified key
   */
  status = tempodb_increment_by_key(config, "button_clicks", 1, response_buffer, 255);

  /* Bulk Write & Bulk Increment
   *
   * This is the same idea as the functions above, but it allows you to batch multiple values (one per series) together in one request
   * There is both a write and an increment version of the call
   */
  struct tempodb_bulk_update *updates = (struct tempodb_bulk_update *)malloc(sizeof(struct tempodb_bulk_update) * 2);

  struct tempodb_bulk_update update1 = { "series_1_key", TEMPODB_KEY, 1.1};
  updates[0] = update1;
  struct tempodb_bulk_update update2 = { "series_2_key", TEMPODB_KEY, 2};
  updates[1] = update2;

  /* use whichever makes sense for you */
  status = tempodb_bulk_write(config, updates, 2, response_buffer, 255);
  status = tempodb_bulk_increment(config, updates, 2, response_buffer, 255);

  free(updates);

  if (status == -1) {
    perror("Error sending to tempodb:");
  } else {
    printf("Response: %s\n", response_buffer);
  }

  free(response_buffer);
  tempodb_destroy(config);
}
