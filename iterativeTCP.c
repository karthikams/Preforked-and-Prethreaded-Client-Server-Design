#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define QLEN 5

/* main - iterative tcp server */
int main(int argc, char *argv[])
{
	/* code */
	struct sockaddr_in fsin;	/* the from address of a client */
	char* service;
	int msock, ssock;			/* master and slave sock */
	int alen;					/* from address length */
	char filename[1024];

	if(argc != 2)
	{
		printf("error: usage %s [port]\n", argv[0]);
		return 1;
	}
	memset(&fsin, 0, sizeof(fsin));
	fsin.sin_family = AF_INET;
	fsin.sin_port = htons(atoi(argv[1]));
	fsin.sin_addr.s_addr = (INADDR_ANY);
	msock = socket(AF_INET, SOCK_STREAM, 0);
	if(msock < 0)
	{
		printf("error while creating a socket\n");
		return 1;
	}


	if (  ( bind(msock, (struct sockaddr*)&fsin, sizeof(fsin)) )  < 0  )
	{
		printf("error while binding\n");
		return 1;
	}

	listen(msock, QLEN);

	printf("waiting for connection from client.......\n");
	while(1)
	{
		ssock = accept(msock, (struct sockaddr*)NULL, NULL);
		if(ssock < 0)
		{
			printf("accept failed\n");
		}
		printf("client connection received\n");


		recv(ssock, filename, sizeof(filename), 0);

		printf("filename requested is: %s\n", filename);

		FILE *fp;
		fp = fopen(filename, "rb");
		if(fp == NULL)
		{
			printf("Error while opening the file to write\n");
			return 1;
		}

		printf("file opened succesfully\n");
		while(1)
		{
			char read_buffer[1024] = {0};

			int bytes_read = fread(read_buffer, 1, 1024, fp);
			printf("bytes_read: %d\n", bytes_read);
			if(bytes_read > 0)
			{
				send(ssock, read_buffer, bytes_read, 0);
				printf("bytes sent: %d\n", bytes_read );
			}

			if(feof(fp))
			{
				printf("reached end of file: %s\n", filename);
				break;
			}

		}


		close(ssock);
	}
	return 0;
}
