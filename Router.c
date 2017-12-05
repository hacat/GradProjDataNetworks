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
int returnStatus;
int addrlen;
struct sockaddr_in router, udpClient, destClient;
char buf[MAXBUF];

//This method was made out of convenience for the comparison of strings
void remove_new_line(char* string)
{
  size_t length;
  if( (length =strlen(string) ) >0)
  {
       if(string[length-1] == '\n')
                string[length-1] ='\0';
  }
}
int receiveFromClient(char* buf){
   addrlen = sizeof(udpClient);
   returnStatus = recvfrom(udpSocket, buf, MAXBUF, 0, (struct sockaddr*)&udpClient, &addrlen);
   if (returnStatus == -1) { fprintf(stderr, "Could not receive message!\n"); 
   } else {
	printf("Received from Client: %s\n", buf);
        /* a message was received so send a confirmation */
        //strcpy(buf, "OK");
	//sendToClient(buf);
   }  
   return returnStatus;
}

int sendToClient(char* buffer){
	int returnStatus;
	returnStatus = sendto(udpSocket, buffer, strlen(buffer)+1, 0, (struct sockaddr*)&udpClient,sizeof(udpClient));
        if (returnStatus == -1) {
            fprintf(stderr, "Could not send confirmation!\n");
        } else {
	    printf("Confirmation sent to Client.\n");
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
        printf("Message sent to Router.\n");
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
	printf("Received from Router: %s\n", buf);
    }
    return returnStatus;
}

int sendToDestClient(char* buffer) {
    int returnStatus;
    returnStatus = sendto(udpSocket, buffer, strlen(buffer)+1, 0, (struct sockaddr*)&destClient, sizeof(router));
    if (returnStatus == -1) {
        fprintf(stderr, "Could not send message!\n");
    }
    else {
        printf("Message sent to Router.\n");
        /* message sent: look for confirmation */
     
    }
    return returnStatus;
}
int receiveFromDestClient(char* buf){
    int returnStatus;
    addrlen = sizeof(router);
    returnStatus = recvfrom(udpSocket, buf, MAXBUF, 0, (struct sockaddr*)&destClient, &addrlen);
    if (returnStatus == -1) {
	fprintf(stderr, "Did not receive confirmation!\n");
    } else {
	buf[returnStatus] = 0;
	printf("Received from Router: %s\n", buf);
    }
    return returnStatus;
}

int main(int argc, char* argv[]) {
    returnStatus = 0;
    addrlen = 0; 

    char routerPort[256];
    char neighborRouterIP[256];
    char neighborRouterPort[256];
    char neighborRouterClientListFile[256];
    /* check for the right number of arguments */
    if (argc < 5) {
        fprintf(stderr, "Usage: %s <routerPort> <neighborRouterIP> <neighborRouterPort> <neighborClientList_File> \n", argv[0]);
        exit(1);
    }
    strcpy(routerPort, argv[1]);
    strcpy(neighborRouterIP, argv[2]);
    strcpy(neighborRouterPort, argv[3]);
    strcpy(neighborRouterClientListFile, argv[4]);
/* create a socket */
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket == -1) {
        fprintf(stderr, "Could not create a socket!\n");
        exit(1);
    } else {
        printf("Socket created.\n");
    }
    /* set up the server address and port */
    /* use INADDR_ANY to bind to all local addresses */
    router.sin_family = AF_INET;
    router.sin_addr.s_addr = htonl(INADDR_ANY);
    /* use the port passed as argument */
    router.sin_port = htons(atoi(argv[1]));

/* bind to the socket */
    returnStatus = bind(udpSocket, (struct sockaddr*)&router, sizeof(router));
    if (returnStatus == 0) {
       fprintf(stderr, "Bind completed!\n");
    } else {
        fprintf(stderr, "Could not bind to address!\n");
        close(udpSocket); exit(1);
    }
/*********Read in username and password doc***********/
	char *IPaddresses[MAXBUF];
	char *PortNumbers[MAXBUF];
	
	remove_new_line(neighborRouterClientListFile);
	FILE *filePointer = fopen(neighborRouterClientListFile, "r");
	char str[MAXBUF];	
		
	if(filePointer == NULL){
		printf("Could not open file");
		return 1;
	} else{
		int column = 0;
		int row = 0;

		while(fgets(str, MAXBUF, filePointer) != NULL) {
			//uncomment line below to see what is in the file
			//printf("'%s'",str);
			char *token = strtok(str, " ");
	
			while(token){
				if(column%2 == 0) {
					IPaddresses[row] = strdup(token);
					//printf("usrlength:'%i'",strlen(token));
				} else if(column%2 == 1) {
					PortNumbers[row] = strdup(token);
					//printf("passwords:'%s'", token);
					row++;
				} 

				column++;
				token = strtok(NULL, " ");
			}
		}
	}
	fclose(filePointer);
/*********check if exists in router neighbors***********/
	char inputIPAddress[256];
	char inputPortNumber[256];
	char msgToSend[256];
	char msgToDestConfirmation[256];
	int foundDestination = 0;
        receiveFromClient(inputIPAddress);
	sendToClient("OK");
	receiveFromClient(inputPortNumber);
	printf("Received from IP:%s and Port Number: %s\n", inputIPAddress, inputPortNumber);
	//check if correct
	int iterator = 0;
	int index = 0;
	
	//since for our text file, the addresses are all local, just check port number
	for(iterator = 0; IPaddresses[iterator] != '\0'; iterator++){
		if(strcmp(strtok(inputIPAddress,"\n"), IPaddresses[iterator])==0
		&& strcmp(strtok(inputPortNumber,"\n"), PortNumbers[iterator])==0){
			index = iterator;
			//printf("%d", iterator);
			printf("'%s'", IPaddresses[index]);
			sendToClient("Found client. Sending message.\n");
			router.sin_family = AF_INET;
    			router.sin_addr.s_addr = inet_addr(inputIPAddress);
    			router.sin_port = htons(atoi(inputPortNumber));
			//Get the message from the client to send to the client destination		
			receiveFromClient(msgToSend);				
			printf("Received message to send from client IP:%s and Port Number: %s\n", inputIPAddress, inputPortNumber);
			//Send the message to the destination client
			sendToDestClient(msgToSend);
			printf("Sent message to destination client IP:%s and Port Number: %s\n", inputIPAddress, inputPortNumber);
			//Get confirmation that the message was sent to destination client
			receiveFromDestClient(msgToDestConfirmation);
			printf("Received destination client confirmation message from client IP:%s and Port Number: %s\n", inputIPAddress, inputPortNumber);
			//Send the confirmation to the Client
			sendToClient(msgToDestConfirmation);
			printf("Sent destination client confirmation message to client.\n");
			foundDestination = 1;
		}
	}
	
	if(!foundDestination){
		/******Send message to a neighbor router with UDP initial inputs********/
		int EndPort = atoi(neighborRouterPort);
		if(strcmp(neighborRouterPort,"0") ==0){
			//send message to everyone that it doesnt exist and they need to close out
			sendToClient("Destination host doesn't exist. Dropping Packet.\n");
			printf("Sent destination host doesn't exist, dropping packet,\n");
			printf("Sent from IP:%s and Port Number: %s\n", inputIPAddress, inputPortNumber);
			close(udpSocket);
		}else { //we need to look at another neighbor
			sendToClient("This pair does not exist, checking neighbor router.\n");
			printf("This pair does not exist, checking neighbor router.\n");
			printf("Sent from IP:%s and Port Number: %s\n", inputIPAddress, inputPortNumber);
			router.sin_family = AF_INET;
    			router.sin_addr.s_addr = inet_addr(neighborRouterIP);
   			router.sin_port = htons(atoi(neighborRouterPort)); 
			char sendNeighborRouterIPConfirmation[256];
			char receiveMessageFromRouterTalkingTo[256];
			//Send neighbor router IP
			sendToRouter(neighborRouterIP);
			printf("Sending neighbor router IP. ");
			printf("Sent from Neighbor Router IP:%s and Neighbor Router Port Number: %s\n", neighborRouterIP, neighborRouterPort);
			//Get "OK" confirmation
			receiveFromRouter(sendNeighborRouterIPConfirmation);
			printf("Getting neighbor router IP 'OK' confirmation. ");
			printf("Received from Neighbor Router IP:%s and Neighbor Router Port Number: %s\n", neighborRouterIP, neighborRouterPort);
			//Send neighbor router port number
			printf("Sending neighbor router port number. ");
			sendToRouter(neighborRouterPort);
			printf("Sent from Neighbor Router IP:%s and Neighbor Router Port Number: %s\n", neighborRouterIP, neighborRouterPort);

			//Get the type of categorization obtained: exist, does not exist, look at another neighbor
			receiveFromRouter(receiveMessageFromRouterTalkingTo);
			printf("Received from Neighbor Router IP:%s and Neighbor Router Port Number: %s\n", neighborRouterIP, neighborRouterPort);
			//Send to the client so they know what is going on
			sendToClient(receiveMessageFromRouterTalkingTo);
			printf("THIS IS WHAT I AM SENDING TO CLIENT: %s", receiveMessageFromRouterTalkingTo);
			printf("Sent from IP:%s and Port Number: %s\n", inputIPAddress, inputPortNumber);
			//send an ok or send the message
			//sendToRouter("OKAYYY");
			//printf("Sent from Neighbor Router IP:%s and Neighbor Router Port Number: %s\n", neighborRouterIP, neighborRouterPort);
		}	
	}

 
/******Send message to destination host********/
	if(!foundDestination){
		char doesNotExist[256];
		receiveFromClient(doesNotExist);
		printf("message recieved in router: '%s'", doesNotExist);
		if(strcmp(doesNotExist, "Destination host doesn't exist. Dropping Packet.\n")==0){
			//send message back to the client so that they know
			sendToClient(doesNotExist);
			printf("YEAHHHHHHHHHHHHHHHHH");
		}
		printf("Received from IP:%s and Port Number: %s\n", inputIPAddress, inputPortNumber);
	}
    /*cleanup */
    close(udpSocket);
    return 0;
}
