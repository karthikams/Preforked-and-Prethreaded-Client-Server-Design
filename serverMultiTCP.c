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

void childProcess(int ssock);


#define PORT 9090		//portnumber where server is listening
#define QUEUELEN 10 	//Length of queue for connection oriented server
#define BUFSIZE 2500		//Buffer size for reading request from client
#define FILEPATH	"/home/karthika/"

int main(int argc, char **argv)
{
	int msock;					//Listening socket descriptor
	int portNumber = PORT;		//variable for storing portNumber
	struct sockaddr_in fsin_client;
	int len = sizeof(struct sockaddr_in);
	int ssock;						// client socket descriptor
	int pid;
	int i = 0;

	if(argc == 2)
	{
		portNumber = atoi(argv[1]);
	}
	else
	{
		printf("%s \n","ip address and port number taken by default");
	}
	//Calling the passive socket creation function

	msock = passiveSock("tcp", portNumber);
	printf("listening sock is %d \n", msock );

	(void)signal(SIGCHLD, reaper);

	while (1)
	{
		ssock = accept(msock, (struct sockaddr*)&fsin_client, &len);
		if(ssock < 0)
		{
			printf("%s \n","error in child sock creation");
			exit(1);
		}
		else
		{
			printf("%s \n", "child socket creation success.");
		}

		//Creating child process
		pid = fork();

		if(pid == 0)
		{
			printf("child process created \n");
			close(msock);
			childProcess(ssock);

		}
		else if(pid)
		{
			close(ssock);
		}
		else
		{
			printf("error occurred while forking \n");
			exit(1);
		}

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
	msock = socket(PF_INET, SOCK_STREAM, ppe->p_proto);
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

	//Listening
	if(listen(msock, QUEUELEN) < 0 )
		{
			printf("%s \n", "listen operation failed.");
			exit(1);
		}
		else
		{
			printf("%s \n", "listen operation success.");
		}


	return msock;
}

void childProcess(int ssock)
{

	unsigned char buf[BUFSIZE+1] = {0};
	int w;
	socklen_t len;
	FILE *fileptr;
	char file[2048] = {0};
	int counter_write = 0; 				//Outputs the total number of bytes send to client
	int size = BUFSIZE+1;
	int reading;
	int reading_counter = 0;
	unsigned char buf_write[1000] = {0};

	printf("connection accepted from client \n ");
	printf("waiting for few seconds \n");
	sleep(5);

	//Reading request from client
	if(recv(ssock, buf, BUFSIZE+1, 0) < 0)
	{
		printf("%s \n", "error in reading request");
		exit(1);
	}
	else
	{
		printf("reading request from client successfull\n");
		printf("%s \n", buf);
	}

	strcpy(file,FILEPATH);
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
	else
	{
		printf("%s\n", "opened the file" );
	}

	while(!feof(fileptr))
	{
		memset(buf_write, 0, sizeof(buf_write));
	    reading = fread(buf_write, 1, sizeof(buf_write), fileptr);
		if(reading < 0)
		{
			printf("error reading \n");
			exit(1);
		}

		printf("total number of bytes read %d \n",reading);
		if(reading > 0)
		{
			w = write(ssock, buf_write, reading);
			if(w < 0)
			{
				printf("Error in writing to socket \n");
				exit(1);
			}
			else
			{
				printf("write successful %d \n", w);
				counter_write += w;
				printf("counter write %d \n", counter_write);
			}
		}
	}
	fclose(fileptr);

	printf("written %d bytes to client \n", counter_write);

	if(close(ssock) == 0)
	{
		printf("close happened properly\n" );
	};

	printf("request completed \n");
	exit(0);

}

void reaper(int sig)
{
	int status;
	while(wait3(&status, WNOHANG, (struct rusage*)0) > 0);
}
