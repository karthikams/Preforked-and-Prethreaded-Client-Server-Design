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

#define PORTNUMBER 	9090										// default port number
#define BUFFSIZE	256
#define QUEUELEN	10


void *childProcess(void* thread_argument_child_sock);

int main(int argc, char *argv[])
{
	/* code */
	int msock;							// master socket
	int portNumber = PORTNUMBER;		// variable for storing portNumber
	struct sockaddr_in sin;  			// object of type sockaddr_in
	struct sockaddr_in child_sin;  		// object of type sockaddr_in for child data structure
	struct protoent *ppe;				// pointer to protoent
	int pthread_return_value;			// should be zero
	int child_sock;						// socket descriptor for child socket
	pthread_t *thread_init;

	if(argc == 1)
	{
		printf("%s%d\n","port number taken by default: ", PORTNUMBER);
	}
	else if(argc == 2)
	{
		portNumber = atoi(argv[1]);
		printf("%s%d\n","port number taken: ", portNumber);
	}

	// starting socket creation
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons((uint16_t)portNumber);
	sin.sin_addr.s_addr = INADDR_ANY;

	/*
	 * mapping the protocol number depending on the type of protocol
	 * Validating its value
	 * if null, protocol is not valid
	 */
	ppe = getprotobyname("tcp");
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


	if(listen(msock, QUEUELEN) < 0 )
	{
		printf("%s \n", "listen operation failed.");
		exit(1);
	}
	else
	{
		printf("%s \n", "listen operation success.");
	}



    int child_address_len = sizeof(child_sin);
    while(1)
    {
    	printf("Waiting to accept client connection.......\n");
    	child_sock = accept(msock, (struct sockaddr*)&child_sin, &child_address_len);	//blocking call

    	printf("connection accepted from client \n");

	    if(child_sock < 0)
		{
			printf("%s \n","error in accepting client connection");
			exit(1);
		}
										// thread initialization
		int *thread_arg = malloc(sizeof(int));
    	*thread_arg = child_sock;
    	thread_init = malloc(sizeof(pthread_t));
		pthread_return_value = pthread_create(thread_init, NULL, childProcess, thread_arg);
		if(!pthread_return_value)
		{
			printf("pthread successfully executed\n");
		}
		else
		{
			printf("error in pthread process\n");
		}

    }
	return 0;
}


void *childProcess(void* thread_argument_child_sock)
{
	printf("Thread created %lu\n",pthread_self());
	int child_sock = *(int*)thread_argument_child_sock;		// typecasting child sock back to int.
	char file[256] = {0};									// buffer to store the filename from client
	FILE *fileptr;											// file pointer to open and read the contents of the file
	int w; 													// no of bytes written while writing
	int write_counter;										// total no of bytes written to client

	// Reading request(filename) from client
	if(read(child_sock, file, BUFFSIZE+1) < 0)
	{
		printf("%s \n", "error in reading request");
		exit(1);
	}
	else
	{
		printf("reading filename from client successful\n");
		printf("filename is %s \n", file);
	}
	printf("waiting for few seconds \n");
	sleep(5);

	fileptr = fopen(file, "r");
	if(!fileptr)
	{
		printf("Error opening the requested file \n");
		exit(1);
	}

	printf("Opened the file\n");

	while(!feof(fileptr))
	{
		printf("reading from file\n");
		unsigned char write_buffer[BUFFSIZE + 1] = {0};
		int reading = fread(write_buffer, 1, BUFFSIZE, fileptr);

		printf("read %d bytes and wrote it to write bufer\n", reading );
		if(reading < 0)
		{
			printf("error reading \n");
			exit(1);
		}
		printf("%s \n", write_buffer);

		if(reading > 0)
		{
			w = write(child_sock, write_buffer, reading);
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

	}

	printf("written %d bytes to client \n", write_counter);
	close(fileptr);
	printf("request completed \n");
	printf("Thread terminated %lu\n\n",pthread_self());
	close(child_sock);

	pthread_exit(0);

}
