#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <ctype.h>

#define UDP_PAYLOAD_SIZE 512

enum transportProtocol
{
	TCP,
	UDP
};

void snd(int fd, char *data)
{
	int bytesWrote;
	int leftBytes = strlen(data);
	while (leftBytes > 0)
	{
		bytesWrote = write(fd, data, leftBytes);
		leftBytes -= bytesWrote;
	}
}

char *rcv(int fd)
{
	struct pollfd *pfds;
	struct pollfd pfd;
	if (calloc(1, sizeof(struct pollfd)) == NULL)
		perror("calloc failed.");
	pfd.fd = fd;
	pfd.events = POLLIN;
	pfds = &pfd;
	int retval = poll(pfds, 1, -1);
	if (retval > 0)
	{
		if (pfds[0].revents != 0)
		{
			if (pfds[0].revents & POLLIN)
			{
				char *response = malloc(sizeof(char));
				int i = 0;
				char buf;
				while (1)
				{
					if (read(fd, &buf, 1) == 1)
        				{
						response[i] = buf;
						i++;
						response = realloc(response, (i+1) * sizeof(char));
					}
					else
					{
						break;
					}
				}
				response[i] = '\0';
				return response;
			}
		}
	}
	return "";
}

char *readFromStdin()
{
	char *response = malloc(sizeof(char));
	int i = 0;
	char buf;
	while (1)
	{
		if (read(0, &buf, 1) == 1)
       		{
			response[i] = buf;
			i++;
			response = realloc(response, (i+1) * sizeof(char));
		}
		else
		{
			break;
		}
	}
	response[i] = '\0';
	return response;
}

// Do not use for reading from a socket fd
bool tryRead(char *buf, FILE *stream)
{
	size_t bytesRead = fread(buf, 1, 1, stream);
	if (feof(stream) != 0)
		return false;
	if (ferror(stream) != 0)
		tryRead(buf, stream);
	if (bytesRead != 1)
		tryRead(buf, stream);
	return true;
}

char *readFile(char *path)
{
	FILE *fp = fopen(path, "r");
	if (fp == NULL)
	{
		perror("fopen failed.");
		exit(EXIT_FAILURE);
	}
	char *buffer = malloc(sizeof(char));
	char buf;
	int i = 0;
	while (1)
	{
		if (tryRead(&buf, fp))
		{
			buffer[i] = buf;
			i++;
			buffer = realloc(buffer, (i+1) * sizeof(char));
		}
		else
		{
			break;
		}
	}
	buffer[i] = '\0';
	fclose(fp);
	return buffer;
}

struct addrinfo *getAddrInfo(char *host, char *port, enum transportProtocol prot)
{
	struct addrinfo *addrInfo, hints;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = prot == TCP ? SOCK_STREAM : SOCK_DGRAM;
	hints.ai_flags = AI_NUMERICSERV;
	hints.ai_protocol = prot == UDP ? IPPROTO_UDP : 0;
	int ret = getaddrinfo(host, port, &hints, &addrInfo);
	if (ret != 0)
	{
		printf("getaddrinfo failed. %s.\n", gai_strerror(ret));
		return NULL;
	}
	return addrInfo;
}

int main(int argc, char *argv[])
{
	static struct option options[] = {
		{ "host", required_argument, 0, 'h' },
		{ "port", required_argument, 0, 'p' },
		{ "data", required_argument, 0, 'd' },
		{ "file", required_argument, 0, 'f' },
		{ "tcp", no_argument, 0, 't' },
		{ "udp", no_argument, 0, 'u' },
		{ 0, 0, 0, 0 }
	};
	int optionIndex = 0;
	int o = 0;
	enum transportProtocol prot;
	bool isTCP = false;
	bool isUDP = false;
	bool isHost = false;
	char *host;
	bool isPort = false;
	char *port;
	bool isData = false;
	char *data;
	bool isFileData = false;
	char *fileData;
	char *res;
	while ((o = getopt_long(argc, argv, "h:p:d:f:tu", options, &optionIndex)) != -1)
	{
		switch (o)
		{
			case 'h':
				if (strlen(optarg) > 0)
					isHost = true;
				if (isHost)
				{
					host = malloc(strlen(optarg)+1 * sizeof(char));
					strcpy(host, optarg);
				}
				break;
			case 'p':
				isPort = true;
				port = malloc(strlen(optarg)+1 * sizeof(char));
				strcpy(port, optarg);
				break;
			case 'd':
				isData = true;
				data = malloc(strlen(optarg)+1 * sizeof(char));
				strcpy(data, optarg);
				break;
			case 'f':
				isFileData = true;
				fileData = readFile(optarg);
				if (strlen(fileData) <= 0)
				{
					printf("No data in file.\n");
					return -1;
				}
				break;
			case 't':
				isTCP = true;
				break;
			case 'u':
				isUDP = true;
				break;
		}
	}
	if (isTCP && isUDP)
	{
		printf("Please specify either tcp or udp, not both. Specifying nothing results in tcp.\n");
		return -1;
	}
	prot = TCP;
	if (isUDP)
		prot = UDP;
	if (!isHost)
	{
		printf("No host provided.\n");
		return -1;
	}
	if (!isPort)
	{
		printf("No port provided.\n");
		return -1;
	}
	if (isFileData && isData)
	{
		printf("Provide either data via the -d or the -f argument.\n");
		return -1;
	}
	char *inputData;
	if (isData)
	{
		inputData = malloc(strlen(data)+1 * sizeof(char));
		strcpy(inputData, data);
	}
	else if (isFileData)
	{
		inputData = malloc(strlen(fileData)+1 * sizeof(char));
		strcpy(inputData, fileData);
	}
	else
	{
		inputData = readFromStdin();
	}
	if (strlen(inputData) <= 0)
	{
		printf("No data provided.\n");
		return -1;
	}
	struct addrinfo *addrInfo = getAddrInfo(host, port, prot);
	int fd;
	switch(prot)
	{
		case TCP:
			fd = socket(AF_INET, SOCK_STREAM, 0);
			if (fd == -1)
			{
				perror("socket failed.\n");
				return -1;
			}
			if (connect(fd, addrInfo->ai_addr, addrInfo->ai_addrlen) == -1)
			{
				perror("connect failed");
				return -1;
			}
			snd(fd, inputData);
			res = rcv(fd);
			break;
		case UDP:
			fd = socket(AF_INET, SOCK_DGRAM, 0);
			if (fd == -1)
			{
				perror("socket failed.\n");
				return -1;
			}
			size_t bytesSent = sendto(fd, inputData, strlen(inputData), 0, addrInfo->ai_addr, addrInfo->ai_addrlen);
			res = malloc(UDP_PAYLOAD_SIZE * sizeof(char));
			ssize_t bytesReceived = recvfrom(fd, res, UDP_PAYLOAD_SIZE, 0, addrInfo->ai_addr, &addrInfo->ai_addrlen);
			break;
	}
	if (strcmp(res, "") != 0)
		printf("%s\n", res);
	return 0;
}
