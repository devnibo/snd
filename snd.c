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
#include <arpa/inet.h>
#include <getopt.h>
#include <ctype.h>
#include "dns.c"

enum transportProtocol
{
	TCP,
	UDP
};

int connectToServer(enum transportProtocol prot, char *ip, int port)
{
	int *fd = malloc(sizeof(int));
	if (prot == TCP)
		*fd = socket(AF_INET, SOCK_STREAM, 0);
	else
		*fd = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in addr;
	struct in_addr iaddr;
	if (inet_aton(ip, &iaddr) == 0)
	{
		perror("inet_aton failed.");
		return -1;
	}
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr = iaddr;
	if (connect(*fd, (struct sockaddr *) &addr, (socklen_t) sizeof(addr)) == -1)
	{
		perror("connect failed.");
		return -1;
	}
	return *fd;
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

bool snd(int fd, char *data)
{
	int bytesWrote;
	int leftBytes = strlen(data);
	while (leftBytes > 0)
	{
		bytesWrote = write(fd, data, leftBytes);
		leftBytes -= bytesWrote;
	}
	return true;
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
	/* Remove last line break from file
	if (buffer[i-1] == '\n')
		buffer[i-1] = '\0'; */
	fclose(fp);
	return buffer;
}

int main(int argc, char *argv[])
{
	static struct option options[] = {
		// { "ip", required_argument, 0, 'i' },
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
	// char ip[16];
	char *ip;
	// char domain[253];
	char *domain;
	bool isDomain = false;
	int port = 0;
	bool isData = false;
	bool isFileData = false;
	char *data;
	char *fileData;
	while ((o = getopt_long(argc, argv, "h:p:d:f:tu", options, &optionIndex)) != -1)
	{
		switch (o)
		{
			case 'h':
				if (strlen(optarg) > 0)
					isHost = true;
				if (isHost)
				{
					if (isdigit(optarg[0])) // Probably an ip and not domain
					{
						if (strlen(optarg) > 15 || strlen(optarg) < 7)
						{
							printf("ip is not valid.\n");
							return -1;
						}
						ip = malloc(16 * sizeof(char));
						strcpy(ip, optarg);
					}
					else
					{
						domain = malloc(strlen(optarg) + 1 * sizeof(char));
						strcpy(domain, optarg);
						isDomain = true;
					}
				}
				break;
			case 'p':
				port = atoi(optarg);
				break;
			case 'd':
				isData = true;
				data = optarg;
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
	if (isHost)
	{
		if (isDomain)
		{
			ip = getIPByDomain(&domain);
			if (ip == NULL)
			{
				printf("getIPByDomain failed.\n");
				return -1;
			}
		}
	}
	else
	{
		printf("No host provided.\n");
		return -1;
	}
	if (port == 0)
	{
		printf("No port provided.\n");
		return -1;
	}
	if (isFileData && isData)
	{
		printf("Provide either data via the -d or the -f argument.\n");
		return -1;
	}
	int fd = connectToServer(prot, ip, port);
	if (fd == -1)
	{
		printf("connectToServer failed.\n");
		return -1;
	}
	char *res;
	if (isData)
	{
		if (snd(fd, data))
			res = rcv(fd);
	}
	else if (isFileData)
	{
		if (snd(fd, fileData))
			res = rcv(fd);
	}
	else
	{
		char *stdinData = readFromStdin();
		if (snd(fd, stdinData))
			res = rcv(fd);
	}
	if (strcmp(res, "") != 0)
		printf("%s\n", res);
	return 0;
}
