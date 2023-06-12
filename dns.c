// DNS spec used: https://www.rfc-editor.org/rfc/rfc1035.html
#include <stdio.h>
#include <netdb.h>
#include <stdint.h>
#include "dns.h"

// e.g. "domain.com" -> 6, 'd', 'o', 'm', 'a', 'i', 'n', 3, 'c', 'o', 'm', 0
struct byte_array *formDNSDomain(char *domain)
{
	char *dnsDomain = malloc(sizeof(char));
	int i = 0;
	int q = 0;
	int domainLabelCharCount = 0;
	char c;
	while ((c = domain[i]) != '\0')
	{
		if (c == '.')
		{
			dnsDomain = realloc(dnsDomain, (q + 1) * sizeof(char));
			dnsDomain[q] = domainLabelCharCount;
			q++;
			for (int k=i-domainLabelCharCount; k<i; k++)
			{
				dnsDomain = realloc(dnsDomain, (q + 1) * sizeof(char));
				dnsDomain[q] = domain[k];
				q++;
			}
			domainLabelCharCount = 0;
		}
		else
		{
			domainLabelCharCount++;
		}
		i++;
	}
	dnsDomain = realloc(dnsDomain, (q + 1) * sizeof(char));
	dnsDomain[q] = domainLabelCharCount;
	q++;
	for (int k=i-domainLabelCharCount; k<i; k++)
	{
		dnsDomain = realloc(dnsDomain, (q + 1) * sizeof(char));
		dnsDomain[q] = domain[k];
		q++;
	}
	dnsDomain = realloc(dnsDomain, (q + 1) * sizeof(char));
	dnsDomain[q] = 0;
	dnsDomain = realloc(dnsDomain, (q + 1 + 4) * sizeof(char));
	struct byte_array *b = malloc(sizeof(struct byte_array));
	b->bytes = dnsDomain;
	b->length = q + 1;
	return b;
}

struct byte_array *formDNSHeader(struct dns_header *h)
{
	char flagOne = 0;
	if (h->QR)
		flagOne = flagOne | 0b10000000;
	if (h->OPCODE[0])
		flagOne = flagOne | 0b01000000;
	if (h->OPCODE[1])
		flagOne = flagOne | 0b00100000;
	if (h->OPCODE[2])
		flagOne = flagOne | 0b00010000;
	if (h->OPCODE[3])
		flagOne = flagOne | 0b00001000;
	if (h->AA)
		flagOne = flagOne | 0b00000100;
	if (h->TC)
		flagOne = flagOne | 0b00000010;
	if (h->RD)
		flagOne = flagOne | 0b00000001;
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
	struct byte_array *b = malloc(sizeof(struct byte_array));
	b->bytes = header;
	b->length = DNS_HEADER_LENGTH;
	return b;
}
 
struct byte_array *formDNSQuestion(struct dns_question *qu)
{
	struct byte_array *d = formDNSDomain(qu->QNAME);
	d->bytes[d->length] = qu->QTYPE[0];
	d->bytes[d->length+1] = qu->QTYPE[1];
	d->bytes[d->length+2] = qu->QCLASS[0];
	d->bytes[d->length+3] = qu->QCLASS[1];
	d->length = d->length + 4;
	return d;
}

struct byte_array *formDNSResourceRecord(struct dns_resource_record *r)
{
	struct byte_array *b = formDNSDomain(r->NAME);
	b->bytes[b->length] = r->TYPE[0];
	b->bytes[b->length+1] = r->TYPE[1];
	b->bytes[b->length+2] = r->CLASS[0];
	b->bytes[b->length+3] = r->CLASS[1];
	b->bytes[b->length+4] = (r->TTL >> 24) & 0xFF;
	b->bytes[b->length+5] = (r->TTL >> 16) & 0xFF;
	b->bytes[b->length+6] = (r->TTL >> 8) & 0xFF;
	b->bytes[b->length+7] = r->TTL & 0xFF;
	b->bytes[b->length+8] = (r->RDLENGTH >> 8) & 0xFF;
	b->bytes[b->length+9] = r->RDLENGTH & 0xFF;
	if (r->RDLENGTH > 0)
	{
		int i = 0;
		for (; i<r->RDLENGTH; i++)
		{
			b->bytes[b->length+i+1] = r->RDATA[i];
		}
		b->length = b->length + i;
	}
	else
	{
		b->length = b->length + 10;
	}
	return b;
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
	h->ANCOUNT[0] = res[6];
	h->ANCOUNT[1] = res[7];
	h->NSCOUNT[0] = res[8];
	h->NSCOUNT[1] = res[9];
	h->ARCOUNT[0] = res[10];
	h->ARCOUNT[1] = res[11];
	return h;
}

/* char *tryParseDNSDomain(char **res, int start)
{
	char *domain = malloc(sizeof(char));
	bool finished = false;
	int i = start;
	int d = 0;
	while ((*res)[i] != '\0')
	{
		printf("%02X ", (*res)[i]);
		i++;
		int len = (*res)[i];
		for (int k=0; k<len; k++)
		{
			domain[d] = (*res)[i+k];
			d++;
		}
		domain[d] = '.';
		d++;
		i += len;
	}
	printf("\n");
	domain[d] = '\0';
	return domain;
} */

bool isValidDNSResponse(struct dns_header *req, struct dns_header **res)
{
	// Is it even the response desired for us?
	if (req->ID[0] != (*res)->ID[0] || req->ID[1] != (*res)->ID[1])
	{
		printf("res ID: %02X %02X\n", (*res)->ID[0], (*res)->ID[1]);
		return false;
	}
	if ((*res)->ANCOUNT[1] == 0x00)
	{
		printf("res ANCOUNT: %02X\n", (*res)->ANCOUNT[1]);
		return false;
	}
	// We don't want other resource records except for an answer
	if ((*res)->NSCOUNT[1] != 0x00 || (*res)->ARCOUNT[1] != 0x00)
	{
		printf("res NSCOUNT[1]: %02X, res ARCOUNT[1]: %02X\n", (*res)->NSCOUNT[1], (*res)->ARCOUNT[1]);
		return false;
	}
	// Any other than 0 is some sort of error
	if ((*res)->RCODE[0] || (*res)->RCODE[1] || (*res)->RCODE[2] || (*res)->RCODE[3])
	{
		printf("res RCODE: %02X %02X %02X %02X\n", (*res)->RCODE[0], (*res)->RCODE[1], (*res)->RCODE[2], (*res)->RCODE[3]);
		return false;
	}
	return true;
}

/*
 * The way I parse RDATA is only meant
 * for a response of TYPE A, which means
 * in case you request the ip of a certain
 * domain. So the function name is way
 * to generalized. Still I don't want to
 * change the function name and possibly
 * the structure.
*/
struct dns_resource_record *tryParseDNSResourceRecord(unsigned char *res, int questionLength)
{
	struct dns_resource_record *rr = malloc(sizeof(struct dns_resource_record));
	int start = DNS_HEADER_LENGTH + questionLength;
	// I don't know why its two bytes long and what the content means
	rr->NAME = malloc(2 * sizeof(char));
	rr->NAME[0] = res[start];
	rr->NAME[1] = res[start+1];
	rr->TYPE[0] = res[start+2];
	rr->TYPE[1] = res[start+3];
	rr->CLASS[0] = res[start+4];
	rr->CLASS[1] = res[start+5];
	// Need to load it in reverse. Has something to do with endianness.
	rr->TTL = res[start+9] + 256U*res[start+8] + 65536U*res[start+7] + 16777216U*res[start+6]; // I copied this
	// Need to load it in reverse. Has something to do with endianness.
	rr->RDLENGTH = res[start+11] + 256U*res[start+10];
	if (rr->RDLENGTH > 0)
	{
		rr->RDATA = malloc(sizeof(char));
		int i = 0;
		int d = 0;
		while (i < rr->RDLENGTH)
		{
			char number[4];
			sprintf(number, "%d", res[start+12+i]);
			number[3] = '\0';
			if (i != 0)
			{
				rr->RDATA[d] = '.';
				rr->RDATA = realloc(rr->RDATA, (d+1) * sizeof(char));
				d++;
			}
			for (int k=0; k<strlen(number); k++)
			{
				rr->RDATA[d] = number[k];
				rr->RDATA = realloc(rr->RDATA, (d+1) * sizeof(char));
				d++;
			}
			i++;
		}
		rr->RDATA[d] = '\0';
	}
	return rr;
}

char *getIPByDomain(char **domain)
{
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
	struct dns_header h = DNS_HEADER_DEFAULT;
	h.RD = true;
	h.QDCOUNT[0] = 0x00;
	h.QDCOUNT[1] = 0x01;
	h.ARCOUNT[0] = 0x00;
	h.ARCOUNT[1] = 0x01;
	struct dns_question q = {
		.QNAME			= *domain,
		.QTYPE[0]		= 0x00,
		.QTYPE[1]		= 0x0f,
		.QCLASS[0]	= 0x00,
		.QCLASS[1]	= 0x01
	};
	struct dns_resource_record rr = {
		.NAME			= *domain,
		.TYPE[0]	= 0x00,
		.TYPE[1]	= 0x41,
		.CLASS[0]	= 0x00,
		.CLASS[1]	= 0x01,
		.TTL			= 0,
		.RDLENGTH	= 0
	};
	struct byte_array *header = formDNSHeader(&h);
	struct byte_array *question = formDNSQuestion(&q);
	struct byte_array *resourceRecord = formDNSResourceRecord(&rr);
	char *req = malloc(sizeof(char));
	int r = 0;
	for (int i=0; i<header->length; i++)
	{
		req = realloc(req, (r + 1) * sizeof(char));
		req[r] = header->bytes[i];
		r++;
	}
	for (int i=0; i<question->length; i++)
	{
		req = realloc(req, (r + 1) * sizeof(char));
		req[r] = question->bytes[i];
		r++;
	}
	for (int i=0; i<resourceRecord->length; i++)
	{
		req = realloc(req, (r + 1) * sizeof(char));
		req[r] = resourceRecord->bytes[i];
		r++;
	}
	int len = header->length + question->length + resourceRecord->length;
	/* printf("req: ");
	for (int i=0; i<len; i++)
	{
		printf("%02X ", req[i]);
	}
	printf("\n"); */
	size_t bytesSent = sendto(fd, req, len, 0, (struct sockaddr *) &addr, size);
	// printf("bytesSent: %d\n", bytesSent);
	unsigned char *res;
	res = malloc(200 * sizeof(char));
	size_t bytesReceived = recvfrom(fd, res, 200, 0, (struct sockaddr *) &addr, &size);
	// printf("bytesReceived: %d\n", bytesReceived);
	if (bytesReceived > DNS_HEADER_LENGTH)
	{
		struct dns_header *resHeader = tryParseDNSHeader(res);
		if (resHeader == NULL)
		{
			printf("Parsing DNS header failed.\n");
			return NULL;
		}
		if (isValidDNSResponse(&h, &resHeader))
		{
			// Parsing the answer. Answer has the structure of a resource record
			struct dns_resource_record *a = tryParseDNSResourceRecord(res, question->length);
			if (a == NULL)
			{
				printf("Parsing DNS answer failed.\n");
				return NULL;
			}
			// TYPE A
			if (a->TYPE[1] == 0x01)
			{
				return a->RDATA;
			}
			else
			{
				printf("Wrong response TYPE. TYPE: %02X %02X\n", a->TYPE[0], a->TYPE[1]);
				return NULL;
			}
		}
		else
		{
			printf("Invalid DNS response.\n");
			return NULL;
		}
	}
	else
	{
		printf("DNS response too short to parse header.\n");
		return NULL;
	}
}
