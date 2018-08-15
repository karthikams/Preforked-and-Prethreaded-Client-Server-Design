/*
 * clientForked.c
 *
 *  Created on: Mar 10, 2018
 *      Author: karthika
 */


#include<stdio.h>

#include<sys/socket.h>
#include<sys/types.h>

#include<netdb.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdarg.h>

#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>

#include<ctype.h>

/*
 * Function for connecting to server
 * Arguments:
 * server - hostname to which the client want to be connected
 * transport - Transport layer protocol (tcp/udp)
 * prtnumber - server port
 */
int connectServer(char *server, char *transport, char *portnumber);

#define FILEPATH "/home/karthika/Desktop/"				//Path where file to be stored
#define BUFSIZE 2500								//Buffer size for reading

int main(int argc, char **argv) {

	char *server, *portnumber;
	char* transport = "udp";
	char buf[BUFSIZE+1] = {0};
	char file[120] = {0};
	char fileName[128]= {0};
	int reading;
	int counter_read = 0;


	printf("%s \n", "client program started");

	if(argc == 4)
	{
		server = argv[1];
		portnumber = argv[2];
		strcpy(fileName, argv[3]);
		printf("file is %s \n", fileName);
	}
	else if(argc == 3)
	{
		server = argv[1];
		portnumber = argv[2];
		printf("please enter the file name to be transferred \n");
		exit(1);
	}
	else{
		printf("please enter the server details \n "
				"Server name, portnumber, file name to be transferred \n");
		exit(1);
	}

	int sock = connectServer(server, transport, portnumber);

	//Writing to the server
	if(write(sock, fileName, strlen(fileName) ) < 0)
	{
		printf("error in write \n");
		exit(1);
	}
	else
	{
		printf("Request for file with name %s sent to server \n", fileName);
	}

	//Appending file name
	strcat(file,FILEPATH);
	strcat(file,fileName);
	printf("file name is %s \n", file);


	//Creating the file to be stored

	FILE *fptr;
	fptr = fopen(file, "w");

	if(!fptr)
	{
		printf("Error in creating / opening file. \n");
		exit(1);
	}

	//Reading response from the server
	reading = read(sock, buf, BUFSIZE+1);
	printf("read bytes %d \n", reading);
	if(reading < 0)
		{
			printf("not able to write in to file \n");
			exit(1);
		}

	fwrite(buf,1,reading,fptr);
	printf("Closing file \n");
	fclose(fptr);
	printf("closing connection \n");
	close(sock);
}


int connectServer(char *server, char *transport, char *portnumber){

	int counter_num = 0;
	int sock,type, port;
	socklen_t len;

	struct sockaddr_in sin;		//Object of struct sockaddr_in
	struct hostent *phe;		//Pointer to hostent
	struct protoent *ppe;		//Pointer to protoent

	//Setting values in sin
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	port = atoi(portnumber);
	sin.sin_port = htons((uint16_t)port);

	printf("server is %s \n", server);

	//Mapping hostname to ip address
	phe = gethostbyname(server);
	if(!phe)
	{
		printf("error in mapping hostname to ip \n");
		exit(1);
	}
	else
	{
		memcpy(&sin.sin_addr.s_addr, phe->h_addr, phe->h_length);
		printf("hostname mapping successful \n");

	}

	//Mapping tcp to protocol number
	ppe = getprotobyname("udp");
	if(!ppe)
	{
		printf("error in mapping protocol number to ip \n");
		exit(1);
	}
	else
	{
		printf("protocol number is %d", ppe -> p_proto);
	}


	//Creating socket
	sock = socket(PF_INET, SOCK_DGRAM, ppe->p_proto);

	if(sock <0)
	{
		printf("error in client socket creation \n");
		exit(1);
	}
	else
	{
		printf("client sock created sucessfully # %d \n",sock);
	}

	//Connecting to the server
	if(connect(sock,(struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		printf("error in connecting to server \n");
		exit(1);
	}
	else
	{
		printf("connected to server \n");
	}

	return sock;

}

