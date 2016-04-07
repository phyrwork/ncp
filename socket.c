#include "socket.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int sock_listen(unsigned short port)
{
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock == -1) return -1;

	struct sockaddr_in server;
	bzero((void *)&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	int rc = bind(sock, (struct sockaddr *) &server, sizeof(server));
	if(rc == -1) return -1;

	listen(sock,1);

	return sock;
}

int sock_accept(int sock)
{
	sock = accept(sock, (struct sockaddr *)NULL, NULL);
	if(sock == -1) return -1;
	else return sock;
}

int sock_connect(unsigned long long addr, unsigned short port)
{
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock == -1) return -1;

	struct sockaddr_in server;
	bzero((void *)&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(addr);

	int rc = connect(sock, (struct sockaddr *) &server, sizeof(server));
	if(rc == -1) return -1;

	return sock;
}

//int sock_send(int sock, const char *buf, size_t len)
//{
//	return sctp_sendmsg(
//		sock, // sd
//		(void *)buf, // msg
//		len, // len
//        (struct sockaddr *) NULL, // to
//        0, // tolen
//        0, // ppid
//        0, // flags
//        0, // stream_no
//        0, // timetolive
//        0 // context
//    );
//}

//int sock_recv(int sock, const char *buf, size_t len)
//{
//	struct sctp_sndrcvinfo sndrcvinfo;
//	int flags;
//
//	int rc = sctp_recvmsg(
//		sock, //sd
//		(void *)buf, // msg
//		len, // len
//		(struct sockaddr *) NULL, // from
//		0, // fromlen
//		&sndrcvinfo, // sinfo
//		&flags // msg_flags
//	);
//
//	// if(rc > 0) fprintf(stderr,"sctp_recvmsg(): ssn:%u\n",sndrcvinfo.sinfo_ssn);
//
//	return rc;
//}

int sock_close(int sock)
{
	return shutdown(sock,SHUT_RDWR);
}
