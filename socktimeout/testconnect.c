//
//  testconnect.c
//  Socket timeout test.
//
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>


void mylog(const char *format, ...);
void perror2(const char *message);


void mylog(const char *format, ...)
{
	va_list va;
	char date[64];
	time_t t = time(NULL);

	strftime(date, sizeof(date), "%Y/%m/%d %a %H:%M:%S", localtime(&t));
	printf("%s: ", date);
	va_start(va, format);
	vprintf(format, va);
	va_end(va);

}

void perror2(const char *message)
{
	mylog("errno: %d\n", errno);
	perror(message);
}

//
// Test Main
//
// program <ipaddr> <port> [<timeout>]
//
int main(int argc, const char * argv[])
{
	char *ipaddr, *port;
	int sockfd;

	int errcode;
	struct addrinfo hints, *address;

	if (argc < 3) {
    puts("Usage: testconnect <ipaddr> <port>");
    return EXIT_SUCCESS;
  }

	ipaddr = strdup(argv[1]);
	port = strdup(argv[2]);

	memset(&hints, 0, sizeof(hints));

	// Get address.
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if ((errcode = getaddrinfo(ipaddr, port, &hints, &address)) != 0) {
		perror2("Failed to do getaddrinfo");
		return -1;
	}

	// Create a socket.
	sockfd = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
	if (sockfd < 0) {
		perror2("Failed to create socket");
		return EXIT_FAILURE;
	}

	// Set FD_CLOEXEC to socket.
	if (fcntl(sockfd, F_SETFD, FD_CLOEXEC) == -1) {
		perror("Failed to do fcntl");
		return EXIT_FAILURE;
	}

	// Enable keepalive.
	int on = 1;
	if (setsockopt(sockfd,
				   SOL_SOCKET, SO_KEEPALIVE,
				   (char *) &on, sizeof(on)) < 0)
	{
		perror2("Failed to set keepalive");
		return EXIT_FAILURE;
	}

  mylog("connect to %s:%s\n", ipaddr, port);

	// Connect
	if (connect(sockfd, address->ai_addr, address->ai_addrlen) < 0)
	{
		perror2("failed to connect");
		return EXIT_FAILURE;
	}

	close(sockfd);

	mylog("Done.\n");

	return EXIT_SUCCESS;
}
