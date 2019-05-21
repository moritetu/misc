//
//  testpoll.c
//  Socket timeout test with event polling.
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
#include <poll.h>
#include <time.h>

void mylog(const char *format, ...);
void print_sockerror(int sockfd);
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

//
// We can use getsockopt for asynchronous operations on sockets.
//
//  Ref: https://stackoverflow.com/questions/21031717/so-error-vs-errno
//
void print_sockerror(int sockfd)
{
	int optval;
	socklen_t optlen;
	optlen = sizeof(optval);

	if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR,
				   (char *) &optval, &optlen) == -1)
	{
		perror2("Failed to getsockopt");
		exit(1);
	}

	if (optval != 0)
	{
		mylog("Found socket error: %s: errornode = %d\n",
				strerror(optval), optval);
		mylog("ETIMEDOUT = %d\n", ETIMEDOUT);
	}
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
	int timeout;

	int flags;
	int errcode;
	struct addrinfo hints, *address;

	if (argc < 3) {
    puts("Usage: testpoll <ipaddr> <port> [<timeout>]");
    return EXIT_SUCCESS;
  }

	ipaddr = strdup(argv[1]);
	port = strdup(argv[2]);
	timeout = -1;
	if (argc > 3) {
		timeout = atoi(argv[3]);
	}

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

	// Put socket into nonblock mode.
	flags = fcntl(sockfd, F_GETFL);
	if (flags < 0) {
		perror2("Failed to do fcntl F_GETFL");
		return EXIT_FAILURE;
	} else if (fcntl(sockfd, F_SETFL, (flags | O_NONBLOCK)) == -1) {
		perror2("Failed to do non block mode");
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

	// Connect
	if (connect(sockfd, address->ai_addr, address->ai_addrlen) < 0)
	{
		if (errno == EINPROGRESS ||
			errno == EINTR)
		{
			puts("Now, we enter into mode to check socket status asynchronously");
		}
	}

	mylog("Check socket error before polling.\n");
	print_sockerror(sockfd);

	//
	// Poll
	//
	int	rc;
	struct pollfd fds[2];

	memset(&fds, 0, sizeof(fds));

	fds[0].fd = sockfd;
	fds[0].events = POLLIN | POLLOUT;


  mylog("connect to %s:%s\n", ipaddr, port);
	mylog("polling (timeout = %d)...\n", timeout);

	rc = poll(fds, 1, timeout);

	do {
		if (fds[0].revents & POLLIN) {
			mylog("Returned with POLLIN");
			break;
		}

		if (fds[0].revents & POLLOUT) {
			mylog("Returned with POLLOUT");
			break;
		}

		if (fds[0].revents & POLLHUP) {
			mylog("Returned with POLLHUP");
			break;
		}

		if (fds[0].revents & POLLERR) {
			mylog("Returned with POLLERR");
			break;
		}

		if (fds[0].revents & POLLNVAL) {
			mylog("Returned with POLLNVAL");
			break;
		}
	} while (0);

	print_sockerror(sockfd);

	mylog("Done.\n");

	return EXIT_SUCCESS;
}
