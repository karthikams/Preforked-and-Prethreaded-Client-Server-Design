#include<stdio.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<pthread.h>

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

void *childProcess(void *arg);


#define PORT 9090		//portnumber where server is listening
#define QUEUELEN 10 	//Length of queue for connection oriented server
#define BUFSIZE 2500		//Buffer size for reading request from client
#define FILEPATH	"/home/karthika/"

//Initializing struct
struct argument
{
	int listening_sock;
	char *file_name;
	struct sockaddr_in *client_addr;
};

int main(int argc, char **argv)
{
	int msock;			//Listening socket descriptor
	int portNumber = PORT;		//variable for storing portNumber
	struct sockaddr_in *fsin_client;
	int len = sizeof(struct sockaddr_in);;
	int ssock;				// client socket descriptor
	int pid = 7;
	int i = 0;
	struct argument *obj;
	char *buf;
	int read;
	pthread_t *child;

	if(argc == 2)
	{
		portNumber = atoi(argv[1]);
	}
	else
	{
		printf("%s \n","ip address and port number taken by default");
	}
	//Calling the passive socket creation function

	msock = passiveSock("udp", portNumber);
	printf("listening sock is %d \n", msock );

	(void)signal(SIGCHLD, reaper);

	while (1)
	{
		buf = malloc(64);
		fsin_client = malloc(sizeof(struct sockaddr_in));

	    read = recvfrom(msock, buf, sizeof(buf), 0, (struct sockaddr*)fsin_client,&len);
	    buf[read] = '\0';
	    printf("%s \n", buf);

		if(read < 0)
		{
			printf("error in receiving request from client \n");
			exit(1);
		}
		else
		{
			printf("request received from client. processing request");
		}

		//Creating child thread
		obj = malloc(sizeof(struct argument));
		obj->listening_sock = msock;
		obj->file_name = buf;
		obj->client_addr = fsin_client;
		child = malloc(sizeof(pthread_t));
		pid = pthread_create(child, NULL, childProcess,(void*)obj);

		if(pid == 0)
		{
			printf("child thread created \n");
		}
		else
		{
			printf("error occurred while thread creation \n");
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

void *childProcess(void *arg)
{

	int w;
	socklen_t len;
	FILE *fileptr;
	char file[2048] = {0};
	int counter_write = 0;
	int reading;
	unsigned char buf_write[BUFSIZE+1] = {0};
	int size = BUFSIZE+1;
	struct sockaddr_in *fsin;
	int read_counter = 0;

	printf("connection accepted from client \n ");
	fflush(stdout);
	sleep(20);
	struct argument *obj = (struct argument*) arg;

	strcpy(file,FILEPATH);
	strcat(file, obj->file_name);
	printf(obj->file_name);
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

		 reading = fread(&buf_write[read_counter], 1, 64, fileptr);

		if(reading < 0)
		{
			printf("error reading \n");
			exit(1);
		}
		else
		{
			//printf("No of bytes read from the file is %d\n", reading);
			size -= reading;
			read_counter+=reading;
		}
		if(size == 0)
		{
			break;
		}
	}
	printf("read bytes %d \n", read_counter);
	fclose(fileptr);
	fsin = obj->client_addr;
	int s = sendto(obj->listening_sock,
				   buf_write, read_counter,0,
				   (struct sockaddr*)fsin, sizeof(struct sockaddr));
	if(s<0)
	{
		printf("request not completed\n");
		exit(1);
	}
	printf("request completed \n");
	free(obj->file_name);
	free(obj->client_addr);
	free(obj);
	pthread_exit(0);
}


void reaper(int sig)
{
	int status;
	while(wait3(&status, WNOHANG, (struct rusage*)0) > 0);
}
