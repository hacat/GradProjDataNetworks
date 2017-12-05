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
struct sockaddr_in udpClient, udpServer, router;

int receiveFromServer(char* buf){
    int returnStatus;
    addrlen = sizeof(udpServer);
    returnStatus = recvfrom(udpSocket, buf, MAXBUF, 0, (struct sockaddr*)&udpServer, &addrlen);
    if (returnStatus == -1) {
	fprintf(stderr, "Did not receive confirmation!\n");
    } else {
	buf[returnStatus] = 0;
	printf("Received from Server: %s\n", buf);
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
        printf("Message sent to Server.\n");
        /* message sent: look for confirmation */
     
    }
    return returnStatus;
}
int sendToRouter(char* buffer) {
    int returnStatus;
    returnStatus = sendto(udpSocket, buffer, strlen(buffer)+1, 0, (struct sockaddr*)&router, sizeof(router));
    if (returnStatus == -1) {
        fprintf(stderr, "Could not send message!\n");
    }
    else {
        printf("Message sent to router.\n");
        /* message sent: look for confirmation */
     
    }
    return returnStatus;
}
int receiveFromRouter(char* buf){
    int returnStatus;
    addrlen = sizeof(router);
    returnStatus = recvfrom(udpSocket, buf, MAXBUF, 0, (struct sockaddr*)&router, &addrlen);
    if (returnStatus == -1) {
	fprintf(stderr, "Did not receive confirmation!\n");
    } else {
	buf[returnStatus] = 0;
	printf("Received from router: %s\n", buf);
    }
    return returnStatus;
}
int main(int argc, char* argv[]) {

    int returnStat;
    char clientPortNumber[256];    
    char authenticationServerIPAddress[256];
    char authenticationServerPortNumber[256];
    char neighborRouterIPAddress[256];
    char neighborRouterPortNumber[256];
    char nameOfDestinationHostMachine[256];
 
    int returnStatus;
    //make sure all ports are filled.  
    if (argc < 7)
    {
       fprintf(stderr, "Usage: %s <clientPort> <serverIP> <serverPort> <routerIP> <routerPort> <destClientName>\n", argv[0]);
       exit(1);
    }
    strcpy(clientPortNumber, argv[1]);    
    strcpy(authenticationServerIPAddress, argv[2]); 
    strcpy(authenticationServerPortNumber, argv[3]);
    strcpy(neighborRouterIPAddress, argv[4]);
    strcpy(neighborRouterPortNumber, argv[5]);
    strcpy(nameOfDestinationHostMachine, argv[6]);

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
    returnStat = bind(udpSocket, (struct sockaddr*)&udpClient, 							sizeof(udpClient));

 if (returnStat == 0) {
        fprintf(stderr, "Bind completed!\n");
    }else {
        fprintf(stderr, "Could not bind to address!\n");
        close(udpSocket);
        exit(1);
    }
    
    /* server address */
    udpServer.sin_family = AF_INET;
    udpServer.sin_addr.s_addr = inet_addr(authenticationServerIPAddress);
    udpServer.sin_port = htons(atoi(clientPortNumber));

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
    int triedOriginalDestination = 0;
    strcpy(inputDestinationClientName, nameOfDestinationHostMachine); 
    while(!validDestinationClient){
	if(triedOriginalDestination){
		printf("Please enter destination client name (e.g. 'gpel1.cs.ou.edu'): ");
		fgets(inputDestinationClientName, 256, stdin);
	}
	triedOriginalDestination = 1;
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
/******Send message to a neighbor router with UDP initial inputs********/
    int neighborBreakResponse = 0;
    char messageToSend[256];
    printf("Enter a message that will be sent to the destination client machine through router: ");
    fgets(messageToSend, 256, stdin);

    router.sin_family = AF_INET;
    router.sin_addr.s_addr = inet_addr(neighborRouterIPAddress);
    router.sin_port = htons(atoi(neighborRouterPortNumber)); 
    
    char neighborRouterIPAddressConfirmation[256];
    char neighborRouterPortNumberConfirmation[256];
    char destClientConfirmation[256];
    //Let the router know what IP address to check in its client list
    sendToRouter(neighborRouterIPAddress);
    //get confirmation that the IP address was received
    receiveFromRouter(neighborRouterIPAddressConfirmation);
    //Let the router know what Port Number to check in its client list
    sendToRouter(neighborRouterPortNumber);
    //Get a message of checking neighbor router if not exist, if does 
    receiveFromRouter(neighborRouterPortNumberConfirmation);

    while(!neighborBreakResponse){
	printf("Received from router IP: %s, router port number: %s, message: %s\n", neighborRouterIPAddress, neighborRouterPortNumber, neighborRouterPortNumberConfirmation);
    	if(strcmp(neighborRouterPortNumberConfirmation, "Destination host doesn't exist. Dropping Packet.\n") ==0){
	//DUMP: drop the message and send a negative mesage back to the client
		printf("The destination host doesn't exist\n");
		neighborBreakResponse = 1;
    	}else if(strcmp(neighborRouterPortNumberConfirmation, "This pair does not exist, checking neighbor router.\n") == 0){
		//get info from next neighbor
		printf("Checking the neighbor router\n");
		receiveFromRouter(neighborRouterPortNumberConfirmation);
		printf("Received something from checking the neighbor router\n");
	}else if(strcmp(neighborRouterPortNumberConfirmation, "Found client. Sending message.\n")){
		printf("There is a client found\n");
		neighborBreakResponse = 1; 
		//Send the message from the client to send to the router to the client dest
		sendToRouter(messageToSend);
		printf("Sent message:'%s' to router IP: %s, router port number: %s", messageToSend, neighborRouterIPAddress, neighborRouterPortNumberConfirmation);
		//Receive the confirmation to the dest client
		receiveFromRouter(destClientConfirmation);
		printf("Received message:'%s' from router IP: %s, router port number: %s", destClientConfirmation, neighborRouterIPAddress, neighborRouterPortNumberConfirmation);
	}
   }
    
    /* cleanup */
    close(udpSocket);
    return 0;
}


