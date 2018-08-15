/*
 * iterUDPserver.c
 *
 *  Created on: Mar 20, 2018
 *      Author: karthika
 */

#include<stdio.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/wait.h>

#include<netdb.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdarg.h>
#include<pthread.h>

#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

int passiveSock(char *protocol, int portNumber);

#define PORT 9090
#define BUFSIZE 2500		//Buffer size for reading request from client
#define FILEPATH	"/home/karthika/"

int main(int argc, char **argv)
{
	int portNumber = PORT;
	int msock;
	int len;
	int read;
	char buf[BUFSIZE+1]={0};
	char *bufptr = buf;
	char file[64];
	struct sockaddr_in fsin;
	int read_byte;
	int size = BUFSIZE+1;
	FILE* fptr;
	unsigned char buf_write[BUFSIZE+1]={0};
	int read_counter = 0;


	if(argc == 2)
	{
		portNumber = atoi(argv[1]);
	}
	else if(argc == 1)
	{
		printf("port number taken by default \n");
	}

	msock = passiveSock("udp", portNumber);
	printf("bound to port \n");

	while(1)
	{
		printf("Waiting to accept client connection\n");
		len = sizeof(fsin);
		read = recvfrom(msock,buf,BUFSIZE+1,0,(struct sockaddr*)&fsin,&len);

		if(read < 0)
		{
			printf("error in receiving connection \n");
			exit(1);
		}
		else
		{
			printf("connection recieved successfully");
			printf("waiting for few seconds \n");
			sleep(5);
		}
		strcat(file,FILEPATH);
		strcat(file,buf);
		printf("file to be searched %s \n", file);

		fptr = fopen(file, "r");
		if(!fptr)
		{
			printf("error in opening file \n");
			exit(1);
		}
		else
		{
			printf("successfully opened file \n");
		}
		int reading = 0;
		while(!feof(fptr))
		{
			reading = fread(&buf[read_counter],1, 64, fptr);
			read_counter += reading;
			size-=reading;
			if(reading < 0)
			{
				printf("error in reading file \n");
				exit(1);
			}
			if(size == 0)
			{
				break;
			}
		}

		int s = sendto(msock, buf, read_counter, 0, (struct sockaddr*)&fsin, sizeof(fsin));
		if(s < 0)
		{
			printf("send to didn't happen properly \n");
			exit(1);
		}
		else{
			printf("continuing listening \n");
		}
	}
}

int passiveSock(char *protocol, int portNumber)
{
	struct sockaddr_in sin;  	//object of type sockaddr_in
	struct protoent *ppe;		//pointer to protoent
	int type;					//socket type decided depending on protocol
	int msock;					//descriptor of listening socket
	int size = BUFSIZE+1;

	/*Initializing sin setting all values to zero
	 * setting family and port number and IP address for sin which is the local endpoint
	 */
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons((uint16_t)portNumber);
	sin.sin_addr.s_addr = INADDR_ANY;

	/*
	 * mapping the protocol number depending on the type of protocol
	 * Validating its value
	 * if null, protocol is not valid
	 */
	ppe = getprotobyname(protocol);
	if(!(ppe))
	{
		printf("%s \n","Protocol name is not valid. "
				"Cannot map the pointer to protoent.");
		exit(1);
	}

	//Creation of listening socket.
	msock = socket(PF_INET, SOCK_DGRAM, ppe->p_proto);
	if(msock < 0)
	{
		printf("%s \n", "Listening socket creation is unsuccessful. ");
		exit(1);
	}
	else
	{
		printf("%s \n", "Listening socket created successfully.");
	}

	//Binding the socket to the local machine.
	if(bind(msock, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		printf("%s \n","Binding of listening socket to local m/c failed.");
		exit(1);
	}
	else
	{
		printf("%s \n", "Bind successful.");
	}

	return msock;
}

