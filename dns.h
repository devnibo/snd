#define DNS_HEADER_LENGTH 12

struct byte_array
{
	char	*bytes;
	int		length;
};

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

const struct dns_header DNS_HEADER_DEFAULT = {
	.ID[0]			= 0x05,
	.ID[1]			= 0x05,
	.QR					= false,
	.OPCODE[0]	= false,
	.OPCODE[1]	= false,
	.OPCODE[2]	= false,
	.OPCODE[3]	= false,
	.AA					= false,
	.TC					= false,
	.RD					= false,
	.RA					= false,
	.Z[0]				= false,
	.Z[1]				= false,
	.Z[2]				= false,
	.RCODE[0]		= false,
	.RCODE[1]		= false,
	.RCODE[2]		= false,
	.RCODE[3]		= false,
	.QDCOUNT[0]	= 0x00,
	.QDCOUNT[1]	= 0x00,
	.ANCOUNT[0]	= 0x00,
	.ANCOUNT[1]	= 0x00,
	.NSCOUNT[0]	= 0x00,
	.NSCOUNT[1]	= 0x00,
	.ARCOUNT[0]	= 0x00,
	.ARCOUNT[1]	= 0x00
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
	uint32_t TTL;
	uint16_t RDLENGTH;
	char *RDATA;
};
