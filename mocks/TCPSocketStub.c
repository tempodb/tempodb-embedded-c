#include "TCPSocketStub.h"
#include <stdio.h>

static struct hostent *hent;
static void *inet_ips;
char *last_request;

static size_t response_buffer_size = 1024;
static size_t response_buffer_bytes_sent;
char *response_buffer;
char *response_buffer_remaining;


static int last_request_size = 1024;

int socket(int domain, int type, int protocol) {
  return 27; /* meaningless */
}

int connect(int sock, const struct sockaddr *name, socklen_t namelen) {
  return 0;
}

struct hostent *gethostbyname(const char *name) {
  char *ips[2];
  ips[0] = inet_ips;
  ips[1] = NULL;

  hent->h_name = "name.company.com";
  hent->h_addrtype = AF_INET;
  hent->h_addr_list = ips;
  return hent;
}

ssize_t recv(int socket, void *buffer, size_t size, int flags) {
  int max_bytes_per_response = 2; /* small enough to make sure we are capturing the whole response */
  size_t bytes_left_in_response = strlen(response_buffer) - response_buffer_bytes_sent;
  ssize_t bytes_to_send;

  if (bytes_left_in_response > 0) {
    if (bytes_left_in_response > max_bytes_per_response) {
      bytes_to_send = max_bytes_per_response;
    } else {
      bytes_to_send = bytes_left_in_response;;
    }

    if (bytes_to_send > size) {
      bytes_to_send = size;
    }

    strncpy(buffer, response_buffer_remaining, bytes_to_send);
    response_buffer_bytes_sent += bytes_to_send;
    response_buffer_remaining += bytes_to_send;
  } else {
    bytes_to_send = 0;
  }
  return bytes_to_send;
}

ssize_t send(int socket, const void *buffer, size_t size, int flags) {
  int remaining_buffer_size = last_request_size - strlen(last_request);
  if (size > remaining_buffer_size) {
    printf("Test error: Unable to cache message (out of space in buffer)\n");
    exit(1);
  }
  strncat(last_request, (char *) buffer, size);
  return size;
}

void set_test_response(const char *buffer) {
  strncpy(response_buffer, buffer, response_buffer_size);
  response_buffer_remaining = response_buffer;
  response_buffer_bytes_sent = 0;
}

const char * test_ip(void) {
  return "1.2.3.4";
}

void test_init(void) {
  int inet_pton_result;

  inet_ips = malloc(32);
  last_request = malloc(last_request_size + 1);
  memset(last_request, 0, last_request_size + 1);

  inet_pton_result = inet_pton(AF_INET, test_ip(), inet_ips);
  if (inet_pton_result == 0) {
    printf("Invalid IP for testing: %s\n", test_ip());
    exit(1);
  } else if (inet_pton_result == -1) {
    perror("Invalid address class");
    exit(1);
  }
  hent = (struct hostent *)malloc(sizeof(struct hostent *));

  response_buffer = malloc(response_buffer_size + 1);
  memset(response_buffer, 0, response_buffer_size + 1);
  set_test_response("200 OK");
}

void test_cleanup(void) {
  free(hent);
  free(inet_ips);
  free(last_request);
  free(response_buffer);
}
