#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

int main()
{
	// DNS server
	const char *ip = "192.168.178.1";
	int port = 53;

	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd == -1)
	{
		perror("socket failed.\n");
		return -1;
	}
	struct sockaddr_in addr;
	struct in_addr iaddr;
	if (inet_aton(ip, &iaddr) == 0)
	{
		perror("inet_aton failed\n");
		return -1;
	}
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr = iaddr;
	int connected = connect(fd, (struct sockaddr *) &addr, (socklen_t) sizeof(addr));
	if (connected == -1)
	{
		perror("connect failed.\n");
		return -1;
	}
	int data[] = {  };
	const char *request = (const char *)data;
	write(fd, &request, strlen(request));
	return 0;
}
