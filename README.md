# TempoDB (Embedded) C Client

The [TempoDB](http://www.tempo-db.com) (Embedded) C API Client is intended for use with embedded electronics. It is designed to be lightweight and portable, and to address a variety of platforms. In the interest of targeting smaller devices, it provides a write-only subset of the API features you'd get from one of [our higher-level libraries](http://tempo-db.com/docs/clients/).

## Beta

This library is under active development, and we are looking for feedback. If you give this it a try, [drop us a note](<patrick@tempo-db.com>). Your experience will help us with continued development, and if you have problems, we may be able to help.

## API

This library provides the following functions. You can find out more information in the [example](https://github.com/tempodb/tempodb-embedded-c/blob/master/examples/example.c).

### Setup and Teardown
```
void tempodb_create(const char *key, const char *secret)
void tempodb_destroy(void)
```

### Writing Values
```
int tempodb_write_by_id(const char *series_id, const float value, char *response_buffer, const ssize_t response_buffer_size)
int tempodb_write_by_key(const char *series_key, const float value, char *response_buffer, const ssize_t response_buffer_size)
```

### Incrementing Values
```
int tempodb_increment_by_key(const char *series_key, const float value, char *response_buffer, const ssize_t response_buffer_size)
int tempodb_increment_by_id(const char *series_id, const float value, char *response_buffer, const ssize_t response_buffer_size)
```

### Bulk/Batch Updates
```
int tempodb_bulk_increment(const struct tempodb_bulk_update *updates, ssize_t update_count, char *response_buffer, const ssize_t response_buffer_size);
int tempodb_bulk_write(const struct tempodb_bulk_update *updates, ssize_t update_count, char *response_buffer, const ssize_t response_buffer_size);
```