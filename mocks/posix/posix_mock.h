#ifndef TEMPODB_PLATFORM_POSIX_MOCK_H
#define TEMPODB_PLATFORM_POSIX_MOCK_H

#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

int socket(int domain, int type, int protocol);
int connect(int sock, const struct sockaddr *name, socklen_t namelen);
ssize_t send(int socket, const void *buffer, size_t size, int flags);
ssize_t recv(int socket, void *buffer, size_t size, int flags);
struct hostent *gethostbyname(const char *name);

void test_init(void);
void test_cleanup(void);

void set_test_response(const char *buffer);

const char * test_ip(void);
char *last_request;

#endif
