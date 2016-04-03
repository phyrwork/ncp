#ifndef NCP_SOCKET_H
#define NCP_SOCKET_H

#include <sys/types.h>

#define EOF 0
#define EBIND 1
#define EABORT 2

int sock_in(unsigned short port);
int sock_out(unsigned long long addr, unsigned short port);

int sock_send(int sock, const char *buf, size_t len);
int sock_recv(int sock, const char *buf, size_t len);

int sock_close(int sock);

#endif /* NCP_SOCKET_H */
