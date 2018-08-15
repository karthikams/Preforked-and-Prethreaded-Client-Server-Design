/*
 * serverForked.c
 *
 *  Created on: Mar 10, 2018
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

#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

/*
 * Creates the listening socket for the server depending on whether its TCP or UDP
 * Arguments:
 * protocol: can be tcp or udp
 * portNumber: portnumber where the server listens for incoming connections
 * return value is the sock descriptor of the listening socket
 */
int passiveSock(char *protocol, int portNumber);

/*
 * Child process handler
 */
void reaper(int sig);

void childProcess(int msock);


#define PORT 9090		//portnumber where server is listening
#define QUEUELEN 10 	//Length of queue for connection oriented server
#define BUFSIZE 2500		//Buffer size for reading request from client
#define FILEPATH "/home/karthika/"

int main(int argc, char **argv)
{
	int numSlaves;		//number if slave processes
	int msock;			//Listening socket descriptor
	int portNumber = PORT;		//variable for storing portNumber


	if(argc == 2)
	{
		numSlaves = atoi(argv[1]);
		printf("%s \n","ip address and port number taken by default");
	}
	else if(argc == 1)
	{
		printf("%s \n", "please enter number of slave processes.");
		exit(1);
	}
	else if (argc == 3)
	{
		portNumber = atoi(argv[1]);
		numSlaves = atoi(argv[2]);
	}

	//Calling the passive socket creation function

	msock = passiveSock("udp", portNumber);
	printf("bound to  sock  %d \n", msock );

	(void)signal(SIGCHLD, reaper);

	for(int i = 0; i < numSlaves; i = i+1)
	{
		printf("%s %d \n", "Creating child process #",i);
		int pid = fork();
		if(pid == 0)
		{
			childProcess(msock);
		}
		else if(pid < 0)
		{
			printf("%s, \n", "Error in forking.");
			exit(1);
		}
		else
		{
			printf("%s, \n", "parent process executing");
		}
	}
	while(1)
	{

	}
}


int passiveSock(char *protocol, int portNumber)
{
	struct sockaddr_in sin;  	//object of type sockaddr_in
	struct protoent *ppe;		//pointer to protoent
	int type;					//socket type decided depending on protocol
	int msock;					//descriptor of listening socket

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
	msock = socket(PF_INET, SOCK_DGRAM, ppe ->p_proto);
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
		printf("%s \n","Binding of lsitening socket to local m/c failed.");
		exit(1);
	}
	else
	{
		printf("%s \n", "Bind successful.");
	}

	return msock;
}

void childProcess(int msock)
{
	struct sockaddr_in fsin;
	unsigned char buf[BUFSIZE+1] = {0};
	int w,read;
	socklen_t len;
	FILE *fileptr;
	char file[128] = {0};
	int counter_write; 				//Outputs the total number of bytes send to client
	int size = BUFSIZE+1;
	unsigned char buf_write[BUFSIZE+1] = {0};
	int read_counter=0;
	int reading;

	while(1)
	{
		printf("Waiting to accept client connection\n");
		len = sizeof(fsin);
		read = recvfrom(msock,buf,BUFSIZE+1,0,(struct sockaddr*)&fsin,&len);

		printf("connection accepted from client \n %d", read);

		//Reading request from client
		if(read < 0)
		{
			printf("%s \n", "error in reading request");
			exit(1);
		}
		else
		{
			printf("reading request from client successfully\n");
			printf("%s \n", buf);
		}
		printf("waiting for few seconds \n");
		sleep(5);

		strcat(file,FILEPATH);
		strcat(file,buf);
		printf("file to be searched %s \n", file);

		//Sending file requested to client
		//opening file

		fileptr = fopen(file, "r");
		if(!fileptr)
		{
			printf("Error opening the requested file \n");
			exit(1);
		}

		while(!feof(fileptr))
		{
			reading = fread(&buf_write[read_counter],1,size,fileptr);
			read_counter+= reading;
			size -= reading;

			if(reading < 0)
			{
				printf("error reading \n");
				exit(1);
			}
			if(size == 0)
			{
				break;
			}
		}
		int s = sendto(msock, buf_write, read_counter, 0, (struct sockaddr*)&fsin, sizeof(fsin));
		if(s < 0)
		{
			printf("sent to client %d bytes \n", s);
		}
		printf("written %d bytes to client \n", s);
		printf("request completed \n");
	}
}

void reaper(int sig)
{
	int status;
	while(wait3(&status, WNOHANG, (struct rusage*)0) > 0);
}

