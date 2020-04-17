#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <string.h>


int main(int argc, char *argv[])
{
	char message[255]; //to display the message 
	int server, portNumber, dice;
	socklen_t len;
	struct sockaddr_in servAdd;
	if(argc != 3){
		printf("Call model:%s <IP> <Port#>\n",argv[0]);
		exit(0);
	}
	//creating and checking if socket is available
	if((server = socket(AF_INET, SOCK_STREAM, 0))<0){ 
	fprintf(stderr, "Cannot create socket\n");
	 exit(1);
	}
	//setting domain to run over the internet
	servAdd.sin_family = AF_INET;
    //formating the Port Number
	sscanf(argv[2], "%d", &portNumber);
	//host to network conversion of multibyte integer
	servAdd.sin_port = htons((uint16_t)portNumber); 
	//converts an address from presentation to network format
	if(inet_pton(AF_INET,argv[1],&servAdd.sin_addr)<0){ 
		fprintf(stderr, " inet_pton() has failed\n");
		exit(2);
	}
	// Establishing connection with TCP server
	if(connect(server,(struct sockaddr *)&servAdd, sizeof(servAdd))<0){ //check if connection is created
		fprintf(stderr, "connect() has failed, exiting\n"); 
		exit(3);
	}
	
	while(1)
	{
		if(read(server, message, 255)<0)
		{
			fprintf(stderr, "read() error\n");
			close(server);
			exit(3);
		}
		if(strcmp(message, "You can now play")==0)
		{
			fprintf(stderr, "I will play now!\n");
			srand(time(0)); 
			dice=(int) rand()%10 +1;
			sprintf(message, "%d", dice);
			printf("Score obtained : %s \n", message);
			sleep(1);
			write(server, message, strlen(message)+1);
		}
		else if(strcmp(message, "Game over: you won the game")==0)
		{
			printf("I won the game\n");
			close(server);
			exit(0);
		}
		else if(strcmp(message, "Game over: you lost the game")==0)
		{
			printf("I lost the game\n");
			close(server);
			exit(0);
		}
	}
}
