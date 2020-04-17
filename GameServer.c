#include <sys/shm.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

void servicePlayers(int player1, int player2); //function to maintain the fork
void calculateScore(char *playerName, int playerId, int *scoreCard, int score); //function to calculate score
void declareWinner(char *playerName, int winnerPlayer, int loserPlayer, int *scoreCard,int  sharedMemoryId); //function to declare the winner
void releaseSharedMemory(int *scoreCard,int  sharedMemoryId); //function to release shared memory


int main(int argc, char *argv[])
{
	int sd, player1, player2, portNumber;
	socklen_t len;
	struct sockaddr_in servAdd;

	fprintf(stderr,"Server IP : ");
	//to display the ip address
    system("hostname -I"); 
    fprintf(stderr,"*************************************\n");
	if(argc != 2)
	{
		printf("Call model: %s <Port #>\n", argv[0]);
		exit(0);
	}
	// Creating the TCP Socket
	if ((sd=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		fprintf(stderr, "Cannot create socket\n");
		exit(1);
	}
	//Prepare the sockaddr_in structure
	servAdd.sin_family = AF_INET; //setting domain to run over the internet
	servAdd.sin_addr.s_addr = htonl(INADDR_ANY); //host to network conversion of multibyte integer for long
	sscanf(argv[1], "%d", &portNumber); //formating the Port Number
	servAdd.sin_port = htons((uint16_t)portNumber); //host to network conversion of multibyte integer for short
	
	// Binding the IP address with Port Number
	bind(sd,(struct sockaddr*)&servAdd,sizeof(servAdd));
	// Put the server in passive mode and wait for server to accept the player
	listen(sd, 5);

	while(1){
		// block until player 1 approach
		player1 = accept(sd,(struct sockaddr*)NULL,NULL);
		printf("Got Player\n");
		// block until player 2 approach
		player2 = accept(sd,(struct sockaddr*)NULL,NULL);
		printf("Got Player\n");

		if(!fork())
			servicePlayers(player1, player2);

        close(player1);
		close(player2);
	}
}

void servicePlayers(int player1, int player2)
{
	char   resp[255];
	int   *scoreCard, sharedMemoryId;

	//shared memory id of two integers
    sharedMemoryId = shmget(IPC_PRIVATE, 2*sizeof(int), IPC_CREAT | 0666);
	if ( sharedMemoryId < 0) {
		printf("*** shmget error ***\n");
       	exit(1);
    }
    //attaching the shared memory to address space
	scoreCard = (int *) shmat(sharedMemoryId, NULL, 0);

	char *msg = "You can now play";
	char *playerName1 = "TOTO";
	char *playerName2 = "TITI";

	while(1)
	{
		// Sending Message to Player 1
		write(player1, msg, strlen(msg)+1);
		//Reading response from the Player 1
		read(player1, resp, 255);
		// Calcuate the score and convert the response into integer
		calculateScore(playerName1, player1,scoreCard,atoi(resp));
		// Displaying the total score and checking winner 
		declareWinner(playerName1, player1, player2, scoreCard, sharedMemoryId);

		sleep(1);
		
		// Sending Message to Player 2
		write(player2,msg, strlen(msg)+1);
		//Reading response from the Player 1
		read(player2, resp, 255);
		// Calcuate the score and convert the response into integer
		calculateScore(playerName2, player2, scoreCard, atoi(resp));
		// Displaying the total score and checking winner 
		declareWinner(playerName2, player2, player1, scoreCard, sharedMemoryId);

		sleep(1);
	}
}
/*
* Method to calculate  and display the score obtained by the Player
*/
void calculateScore(char *playerName, int playerId, int *scoreCard, int  score){
	printf("Score Obtained by %s : %d\n", playerName, score);
	// Based on player Id updating the score inside the shared Memory i.e., scoreCard
	scoreCard[playerId] = scoreCard[playerId] + score;
}
/*
* Method to display Total score and check winner player
* and send message to winner player and loser player
*/
void declareWinner(char *playerName, int winnerPlayer, int loserPlayer, int *scoreCard,int  sharedMemoryId){
	printf("%s: Total so far %d \n", playerName, scoreCard[winnerPlayer]);

	if (scoreCard[winnerPlayer] >= 100){
		char *winMsg = "Game over: you won the game";
		char *loseMsg = "Game over: you lost the game";
		// Sending Message to Winner Player
		write(winnerPlayer, winMsg, strlen(winMsg)+ 1);
		// Sending Message to Loser Player
		write(loserPlayer, loseMsg, strlen(loseMsg)+ 1);
		// Releasing shared memory
		releaseSharedMemory(scoreCard, sharedMemoryId);
		// Closing socket for Winner Player
		close(winnerPlayer);
		// Closing socket for Loser Player
		close(loserPlayer);
		exit(0);
	}
}
/*
* Method detach the shared memory from program and address space 
*/
void releaseSharedMemory(int *scoreCard,int  sharedMemoryId){
	// Detaches the shared memory from the program
	shmdt((void *) scoreCard);
   	// Remove the shared memory segment
	shmctl(sharedMemoryId, IPC_RMID, NULL);
}
