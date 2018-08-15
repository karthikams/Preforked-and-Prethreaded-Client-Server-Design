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
#define PORT 9090
#define BUFFSIZE 2500		//Buffer size for reading request from client
#define FILEPATH	"/home/karthika/"

void *childProcess(void* thread_argument_msock);

int passiveSock(char *protocol, int portNumber);

int main(int argc, char **argv)
{
	int portNumber = PORT;
	int msock;
	int len;
	int read;
	char buf[BUFFSIZE+1]={0};
	char file[64];
	struct sockaddr_in fsin;
	int read_byte;
	int size = BUFFSIZE+1;
	FILE* fptr;
	unsigned char buf_write[BUFFSIZE+1]={0};
	int read_counter = 0;
	int numThreads = 0;			// number
	int pthread_return_value;		// should be zero
	pthread_t childThreads[numThreads];	// num of thread initializations

	if(argc == 1)
	{
		printf("%s \n", "please enter number of slave processes and port number.");
		exit(1);
	}
	else if(argc == 2)
	{
		numThreads = atoi(argv[1]);
		printf("%s \n","port number taken by default");
	}
	else if (argc == 3)
	{
		numThreads = atoi(argv[1]);
		portNumber = atoi(argv[2]);
		printf("number of slave processes is %d and port number is %d. \n",numThreads, portNumber);
	}


	msock = passiveSock("udp", portNumber);
	printf("bound to port \n");

	int *thread_arg = malloc(sizeof(int));
    	*thread_arg = msock;
	for(int i = 0; i < numThreads; i = i + 1)
	{
		printf("%s%d \n", "Creating thread #",i);
		pthread_return_value = pthread_create(&childThreads[i], NULL, childProcess, thread_arg);

		if(!pthread_return_value)
		{
			printf("pthread successfully executed\n");
		}
		else
		{
			printf("error in pthread process\n");
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
	int type;			//socket type decided depending on protocol
	int msock;			//descriptor of listening socket
	int size = BUFFSIZE+1;

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


void *childProcess(void* thread_argument_msock)
{
	int msock = *(int*)thread_argument_msock;		// typecasting msock back to int.
	struct sockaddr_in child_sin;				// child data structure
	socklen_t len;						// len of child data structure
	//int child_sock;					// child socket descriptor
	char file[256] = {0};					// buffer to store the filename from client
	FILE *fileptr;						// file pointer to open and read the contents of the file
	int w; 							// no of bytes written while writing
	int write_counter;					// total no of bytes written to client
	unsigned char write_buffer[BUFFSIZE + 1] = {0};
	int size = BUFFSIZE+1;


	printf("`Thread created %lu\n",pthread_self());
	// place it in a while loop for it to always accept and process client communications
	while(1)
	{
		//printf("Waiting to accept client connection\n");

		/*child_sock = accept(msock, (struct sockaddr*)&child_sin, &len);		//blocking call
		if(child_sock < 0)
		{
			printf("%s \n","error in child thread creation");
			exit(1);
		}
		else
		{
			printf("%s \n", "child thread creation success.");
		}

		printf("connection accepted from client \n ");
	*/

		// Reading request(filename) from client
		printf("%s\n",file);
		len = sizeof(child_sin);

		if(recvfrom(msock, file, BUFFSIZE+1,0,(struct sockaddr *)&child_sin,&len) < 0)
		{
			printf("%s \n", "error in reading request");
			exit(1);
		}
		else
		{
			printf("reading filename from client successful\n");
			printf("filename is %s \n", file);
		}
		printf("Waiting for 5 seconds to check concurrency");
		sleep(5);
		fileptr = fopen(file, "r");
		if(!fileptr)
		{
			printf("Error opening the requested file \n");
			exit(1);
		}

		printf("Opened the file\n");
		int reading,noofbytes_read=0;

		while(!feof(fileptr))
		{
			printf("writing to file\n");
			reading = fread(write_buffer, 1, BUFFSIZE+1, fileptr);
			//printf("%s",write_buffer);
			printf("\n");
			noofbytes_read+=reading;
			printf("read %d bytes and wrote it to write bufer\n", reading );

			if(reading < 0)
			{
				printf("error reading \n");
				exit(1);
			}
			else if(reading >0 )
			{
				size -= reading;
				printf("read:%d\n",reading);
			}
			if(size == 0)
			{
				break;
			}



		}
		printf("%s \n", "Came out of read");


		if(noofbytes_read > 0)
			{
				w = sendto(msock, write_buffer, noofbytes_read,0,(struct sockaddr *)&child_sin,len);
				if(w < 0)
				{
					printf("Error in writing to socket \n");
					exit(1);
				}
				else
				{
					printf("write successfull \n");
					write_counter += w;
				}
			}
		//printf("written %d bytes to client \n", write_counter);
		close(fileptr);
		printf("request completed \n");
		printf("Thread terminated %lu\n\n",pthread_self());

	}
}
