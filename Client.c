#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <netdb.h>
#define MAXBUF 1024
int udpSocket;
int addrlen;
struct sockaddr_in udpClient, udpServer;

int receiveFromServer(char* buf){
    int returnStatus;
    addrlen = sizeof(udpServer);
    returnStatus = recvfrom(udpSocket, buf, MAXBUF, 0, (struct sockaddr*)&udpServer, &addrlen);
    if (returnStatus == -1) {
	fprintf(stderr, "Did not receive confirmation!\n");
    } else {
	buf[returnStatus] = 0;
	printf("Received: %s\n", buf);
    }
    return returnStatus;
}
int sendToServer(char* buffer) {
    int returnStatus;
    returnStatus = sendto(udpSocket, buffer, strlen(buffer)+1, 0, (struct sockaddr*)&udpServer, sizeof(udpServer));
    if (returnStatus == -1) {
        fprintf(stderr, "Could not send message!\n");
    }
    else {
        printf("Message sent.\n");
        /* message sent: look for confirmation */
     
    }
    return returnStatus;
}

int main(int argc, char* argv[]) {
	char buf[MAXBUF];
    int returnStatusq;
    if (argc < 3)
    {
       fprintf(stderr, "Usage: %s <ip address> <port>\n", argv[0]);
       exit(1);
    }

 /* create a socket */
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket == -1){
       fprintf(stderr, "Could not create a socket!\n");
       exit(1);
    }else {
       printf("Socket created.\n");     }
    /* client address */
    /* use INADDR_ANY to use all local addresses */
    udpClient.sin_family = AF_INET;
    udpClient.sin_addr.s_addr = INADDR_ANY;
    udpClient.sin_port = 0;
    returnStatusq = bind(udpSocket, (struct sockaddr*)&udpClient, 							sizeof(udpClient));

 if (returnStatusq == 0) {
        fprintf(stderr, "Bind completed!\n");
    }else {
        fprintf(stderr, "Could not bind to address!\n");
        close(udpSocket);
        exit(1);
    }
    
    /* server address */
    /* use the command-line arguments */
    udpServer.sin_family = AF_INET;
    udpServer.sin_addr.s_addr = inet_addr(argv[1]);
    udpServer.sin_port = htons(atoi(argv[2]));

    /* set up the message to be sent to the server */
    //user authentication
    char inputUserName[256];
    char inputPassword[256];
    char receiveUserPassConfirmation[256];
    int authorized = 0;

    //While the user is not authorized...
    while(!authorized){
	//Getting info from the user
   	printf("Welcome. Please enter user name: ");
    	fgets(inputUserName, 256, stdin);
    	//strcpy(buf, "University of Oklahoma\n");
	if(sendToServer(inputUserName) != -1){
		receiveFromServer(receiveUserPassConfirmation); //receive an "OK"
		printf("Please enter password: ");
    		fgets(inputPassword, 256, stdin);
		if(sendToServer(inputPassword) != -1){
			//check if the user and password pair was correct
			receiveFromServer(receiveUserPassConfirmation);	
			if(strcmp(receiveUserPassConfirmation, "correct")==0)
			{ authorized = 1;}	
		}
	}
    }
    
/******Check if destination client name exists********/
    char inputDestinationClientName[256];
    char inputDestinationClientIP[256];
    char inputDestinationClientPortNumber[256];
    char receiveClientConfirmation[256];
    int validDestinationClient = 0;
    while(!validDestinationClient){
	printf("Please enter destination client name (e.g. 'gpel8.cs.ou.edu'): ");
	fgets(inputDestinationClientName, 256, stdin);
	if(sendToServer(inputDestinationClientName) != -1){
		receiveFromServer(receiveClientConfirmation);
		if(strcmp(receiveClientConfirmation, "correct") == 0){
			receiveFromServer(inputDestinationClientIP);
			receiveFromServer(inputDestinationClientPortNumber);
			validDestinationClient = 1;
			printf("this is right!!!");
			printf("'%s', '%s'", inputDestinationClientIP, inputDestinationClientPortNumber);
		} else { printf("Invalid client destination name.\n"); }	
	}
    }
/******Send message to a neighbor router********/
    char messageToSend[256];
    printf("Enter a message that will be sent to the destination client machine through router: ");
    fgets(messageToSend, 256, stdin);

    /* cleanup */
    close(udpSocket);
    return 0;
}


