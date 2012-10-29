# TempoDB (Embedded) C Client

[![Build Status](https://travis-ci.org/tempodb/tempodb-embedded-c.png)](https://travis-ci.org/tempodb/tempodb-embedded-c)

The [TempoDB](http://www.tempo-db.com) (Embedded) C API Client is intended for use with embedded electronics. It is designed to be lightweight and portable, and to address a variety of platforms. In the interest of targeting smaller devices, it provides a write-only subset of the API features you'd get from one of [our higher-level libraries](http://tempo-db.com/docs/clients/).

## Beta

This library is under active development, and we are looking for feedback. If you give this it a try, drop us a note (<patrick@tempo-db.com>). Your experience will help us with continued development, and if you have problems, we may be able to help.

## Timestamps

Because this library is targeted at small, embedded devices,
it does not require the system to know the current time and
instead relies on the TempoDB servers to timestamp the values
as they come in. This means that the value gets assigned a
timestamp slightly after it left your system, and multiple
values may arrive at TempoDB at the same time. There are two
important take-aways from this:

1. If you are doing high-resolution sampling the slight, somewhat inconsistent delay due to internet latencies may affect you.
2. It is possible that two values sent at different times will arrive at TempoDB at the same time. If you want TempoDB to overwrite one value with the other, use the "write" functions. If you want TempoDB to sum the values, use the "increment" functions.

## API

This library provides the following functions. You can find out more information in the [example](https://github.com/tempodb/tempodb-embedded-c/blob/master/examples/example.c).

### Setup and Teardown
```
tempodb_config * tempodb_create(const char *key, const char *secret);
void tempodb_destroy(tempodb_config *config);
```

### Writing Values
```
int tempodb_write_by_id(tempodb_config *config, const char *series_id, const float value, char *response_buffer, const ssize_t response_buffer_size);
int tempodb_write_by_key(tempodb_config *config, const char *series_key, const float value, char *response_buffer, const ssize_t response_buffer_size);
```

### Incrementing Values
```
int tempodb_increment_by_key(tempodb_config *config, const char *series_key, const float value, char *response_buffer, const ssize_t response_buffer_size);
int tempodb_increment_by_id(tempodb_config *config, const char *series_id, const float value, char *response_buffer, const ssize_t response_buffer_size);
```

### Bulk/Batch Updates
```
int tempodb_bulk_increment(tempodb_config *config, const tempodb_bulk_update *updates, ssize_t update_count, char *response_buffer, const ssize_t response_buffer_size);
int tempodb_bulk_write(tempodb_config *config, const tempodb_bulk_update *updates, ssize_t update_count, char *response_buffer, const ssize_t response_buffer_size);
```
