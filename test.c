#include <stdio.h>
#include <stdlib.h>

void formDNSDomain(char *domain)
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
	for (int e=0; e<q+1; e++)
	{
		printf("%2X ", dnsDomain[e]);
	}
	printf("\n");
}

int main()
{
	formDNSDomain("mail._domainkey.relim.de");
	return 0;
}
