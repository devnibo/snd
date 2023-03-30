// DNS spec used: https://www.rfc-editor.org/rfc/rfc1035.html
#include <stdio.h>
#include <netdb.h>
#include <stdint.h>

#define DNS_HEADER_LENGTH 12

struct dns_header
{
	char ID[2];
	bool QR;
	bool OPCODE[4];
	bool AA;
	bool TC;
	bool RD;
	bool RA;
	bool Z[3];
	bool RCODE[4];
	char QDCOUNT[2];
	char ANCOUNT[2];
	char NSCOUNT[2];
	char ARCOUNT[2];
};

struct dns_question
{
	char *QNAME;
	char QTYPE[2];
	char QCLASS[2];
};

// Either Answer, Authority or Additional
struct dns_resource_record
{
	char *NAME;
	char TYPE[2];
	char CLASS[2];
	unsigned int32_t TTL;
	unsigned int16_t RDLENGTH;
	char *RDATA;
};

char *formDNSHeader(struct dns_header *h)
{
	char flagOne = 0;
	flagOne = h->QR ? flagOne | 0b10000000 : flagOne;
	flagOne = h->OPCODE[0] ? flagOne | 0b01000000 : flagOne;
	flagOne = h->OPCODE[1] ? flagOne | 0b00100000 : flagOne;
	flagOne = h->OPCODE[2] ? flagOne | 0b00010000 : flagOne;
	flagOne = h->OPCODE[3] ? flagOne | 0b00001000 : flagOne;
	flagOne = h->AA ? flagOne | 0b00000100 : flagOne;
	flagOne = h->TC ? flagOne | 0b00000010 : flagOne;
	flagOne = h->RD ? flagOne | 0b00000001 : flagOne;
	char flagTwo = 0;
	flagTwo = h->RA ? flagTwo | 0b10000000 : flagTwo;
	flagTwo = h->Z[0] ? flagTwo | 0b01000000 : flagTwo;
	flagTwo = h->Z[1] ? flagTwo | 0b00100000 : flagTwo;
	flagTwo = h->Z[2] ? flagTwo | 0b00010000 : flagTwo;
	flagTwo = h->RCODE[0] ? flagTwo | 0b00001000 : flagTwo;
	flagTwo = h->RCODE[1] ? flagTwo | 0b00000100 : flagTwo;
	flagTwo = h->RCODE[2] ? flagTwo | 0b00000010 : flagTwo;
	flagTwo = h->RCODE[3] ? flagTwo | 0b00000001 : flagTwo;
	char *header = malloc(DNS_HEADER_LENGTH * sizeof(char));
	header[0] = h->ID[0];
	header[1] = h->ID[1];
	header[2] = flagOne;
	header[3] = flagTwo;
	header[4] = h->QDCOUNT[0];
	header[5] = h->QDCOUNT[1];
	header[6] = h->ANCOUNT[0];
	header[7] = h->ANCOUNT[1];
	header[8] = h->NSCOUNT[0];
	header[9] = h->NSCOUNT[1];
	header[10] = h->ARCOUNT[0];
	header[11] = h->ARCOUNT[1];
	return header;
}

char *formDNSQuestion(struct dns_question *q)
{
	char *question;
	int i = 0;
	int l = 0;
	int k = 0;
	int domainLabelCharCount = 0;
	char c;
	while ((c = q->QNAME[i]) != '\0')
	{
		if (c == '.')
		{
			domainLabelCharCount = l + 1;
			question = realloc(question, (k + 1) * sizeof(char));
			question[k] = domainLabelCharCount;
			k++;
			for (int e=0; e<domainLabelCharCount; e++)
			{
				question = realloc(question, (k + 1) * sizeof(char));
				question[k] = q->QNAME[e];
				k++;
			}
			l = 0;
		}
		i++;
		l++;
	}
}

struct dns_header *tryParseDNSHeader(char *res)
{
	struct dns_header *h = malloc(sizeof(struct dns_header));
	h->ID[0] = res[0];
	h->ID[1] = res[1];
	h->QR = res[2] & 0b10000000 ? true : false;
	h->OPCODE[0] = res[2] & 0b01000000 ? true : false;
	h->OPCODE[1] = res[2] & 0b00100000 ? true : false;
	h->OPCODE[2] = res[2] & 0b00010000 ? true : false;
	h->OPCODE[3] = res[2] & 0b00001000 ? true : false;
	h->AA = res[2] & 0b00000100 ? true : false;
	h->TC = res[2] & 0b00000010 ? true : false;
	h->RD = res[2] & 0b00000001 ? true : false;
	h->RA = res[3] & 0b10000000 ? true : false;
	h->Z[0] = res[3] & 0b01000000 ? true : false;
	h->Z[1] = res[3] & 0b00100000 ? true : false;
	h->Z[2] = res[3] & 0b00010000 ? true : false;
	h->RCODE[0] = res[3] & 0b00001000 ? true : false;
	h->RCODE[1] = res[3] & 0b00000100 ? true : false;
	h->RCODE[2] = res[3] & 0b00000010 ? true : false;
	h->RCODE[3] = res[3] & 0b00000001 ? true : false;
	h->QDCOUNT[0] = res[4];
	h->QDCOUNT[1] = res[5];
	h->ANCOUNT[0] = res[6]
	h->ANCOUNT[1] = res[7]
	h->NSCOUNT[0] = res[8];
	h->NSCOUNT[1] = res[9];
	h->ARCOUNT[0] = res[10];
	h->ARCOUNT[1] = res[11];
	return h;
}

bool isValidDNSResponse(struct dns_header **h)
{
	return true;
}

struct dns_answer *tryParseDNSAnswer(char *res)
{
}

char *getIPByDomain(char **domain)
{
	printf("domain: %s\n", *domain);
	struct hostent *host = (struct hostent *) gethostbyname((char *) "192.168.178.1");
	int port = 53;
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd == -1)
	{
		perror("socket failed.\n");
		return NULL;
	}
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr = *((struct in_addr *)host->h_addr);
	bzero(&(addr.sin_zero), 8);
	socklen_t size = (socklen_t) sizeof(addr);
	unsigned char headerFlagsPartOne = 0;
	// Activate RD, means RecursiveDesired
	headerFlagsPartOne = headerFlagsPartOne | 0b00000001;
	unsigned char headerFlagsPartTwo = 0;
	const char req[] = {
		/* BEGIN HEADER */
		0x05, 0x05, // ID
		headerFlagsPartOne, // QR, OPCODE, AA, TC, RD
		headerFlagsPartTwo, // RA, Z, RCODE
		0x00, 0x01, // QDCOUNT
		0x00, 0x00, // ANCOUNT
		0x00, 0x00, // NSCOUNT
		0x00, 0x01, // ARCOUNT
		/* END HEADER */
		/* BEGIN QUESTION */
		5, 'r', 'e', 'l', 'i', 'm', 2, 'd', 'e', 0, // QNAME
		0x00, 0x01, // QTYPE
		0x00, 0x01, // QCLASS
		/* END QUESTION */
		/* BEGIN ADDITIONAL RECORDS */
		5, 'r', 'e', 'l', 'i', 'm', 2, 'd', 'e', 0, // NAME
		0x00, 0x41, // TYPE: OPT
		0x00, 0x01, // CLASS: Internet
		0x00, 0x00, 0x00, 0x00, // TTL (Time To Live)
		0x00, 0x00 // RDLENGTH
			   // RDATA (No RDLENGTH so no RDATA)
		/* END ADDITIONAL RECORDS */
	};
	size_t bytesSent = sendto(fd, &req, sizeof(req), 0, (struct sockaddr *) &addr, size);
	printf("bytesSent: %d\n", bytesSent);
	// char res[200];
	char *res;
	res = malloc(200 * sizeof(char));
	size_t bytesReceived = recvfrom(fd, res, 200, 0, (struct sockaddr *) &addr, &size);
	printf("bytesReceived: %d\n", bytesReceived);
	if (bytesReceived > DNS_HEADER_LENGTH)
	{
		struct dns_header *h = tryParseDNSHeader(res);
		if (h == NULL)
		{
			printf("Parsing DNS header failed.\n");
			return NULL;
		}
		if (isValidDNSResponse(&h))
		{
			struct dns_answer *a = tryParseDNSAnswer(res);
			if (a == NULL)
			{
				printf("Parsing DNS answer failed.\n");
				return NULL;
			}
		}
	}
	else
	{
		printf("DNS response too short to parse header.\n");
	}
	return "202.61.254.236";
}
