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
struct sockaddr_in udpServer, udpClient;
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
	printf("Received: %s\n", buf);
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
	    printf("Confirmation sent.\n");
        }
	return returnStatus;
}

int main(int argc, char* argv[]) {
    udpSocket;
    returnStatus = 0;
  addrlen = 0;
   
   
    /* check for the right number of arguments */
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }
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
    udpServer.sin_family = AF_INET;
    udpServer.sin_addr.s_addr = htonl(INADDR_ANY);
    /* use the port passed as argument */
    udpServer.sin_port = htons(atoi(argv[1]));

/* bind to the socket */
    returnStatus = bind(udpSocket, (struct sockaddr*)&udpServer,
    sizeof(udpServer));
    if (returnStatus == 0) {
       fprintf(stderr, "Bind completed!\n");
    } else {
        fprintf(stderr, "Could not bind to address!\n");
        close(udpSocket); exit(1);
    }
/*********Read in username and password doc***********/
	char *usernames[MAXBUF];
	char *passwords[MAXBUF];

	FILE *filePointer = fopen("userList.txt", "r");
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
					usernames[row] = strdup(token);
					//printf("usrlength:'%i'",strlen(token));
				} else if(column%2 == 1) {
					passwords[row] = strdup(token);
					//printf("passwords:'%s'", token);
					row++;
				} 

				column++;
				token = strtok(NULL, " ");
			}
		}
	}
	fclose(filePointer);
/*********check for authentication***********/
   int correctPass = 0;
   while (!correctPass) {
	char inputUserName[256];
	char inputPassword[256];
        receiveFromClient(inputUserName);
	sendToClient("OK");
	receiveFromClient(inputPassword);
	//check if correct
	int iterator = 0;
	int index = 0;

	for(iterator = 0; usernames[iterator] != '\0'; iterator++){
		if(strcmp(strtok(inputUserName,"\n"), usernames[iterator])==0){
			index = iterator;
			//printf("%d", iterator);
			printf("'%s'", passwords[index]);
		}
	}

	if(strcmp(inputPassword,passwords[index])==0){
		sendToClient("correct");
		correctPass = 1;
	}else{
		sendToClient("Incorrect username and password pair.\n");
	}

    }	
	
/******Check if destination client name exists********/
	char *destClientName[MAXBUF];
	char *destClientIP[MAXBUF];
	char *destClientPortNumber[MAXBUF];

	FILE *fp = fopen("destinationClients.txt", "r");
	char Str[MAXBUF];	
		
	if(fp == NULL){
		printf("Could not open file");
		return 1;
	} else{
		int column = 0;
		int row = 0;

		while(fgets(Str, MAXBUF, fp) != NULL) {
			//uncomment line below to see what is in the file
			printf("'%s'",Str);
			char *token = strtok(Str, " ");
	
			while(token){
				if(column%3 == 0) {
					destClientName[row] = strdup(token);
				} else if(column%3 == 1) {
					destClientIP[row] = strdup(token);
					//printf("passwords:'%s'", token);
				} else if(column%3 == 2){
					destClientPortNumber[row] = strdup(token);
				row++;
				}
				column++;
				token = strtok(NULL, " ");
			}
		}
	}
	fclose(fp);


    char inputDestinationClient[256];
    char inputDestinationClientIP[256];
    char inputDestinationClientPortNumber[256];
    int validDestinationClient = 0;
    while(!validDestinationClient){
	receiveFromClient(inputDestinationClient);
	int iterator = 0;
	int index = 0;
	remove_new_line(inputDestinationClient);
	printf("input:'%s'\n", inputDestinationClient);
	for(iterator = 0; destClientName[iterator] != '\0'; iterator++){
		//printf("iterator:'%s'\n", destClientName[iterator]);
		if(strcmp(inputDestinationClient, destClientName[iterator])==0){
			index = iterator;
			//printf("%d", iterator);
			//printf("'%s'", destClientIP[index]);
			sendToClient("correct");
			strcpy(inputDestinationClientIP, destClientIP[index]);
			sendToClient(inputDestinationClientIP);
			strcpy(inputDestinationClientPortNumber, destClientPortNumber[index]);
			sendToClient(inputDestinationClientPortNumber);
			validDestinationClient = 1;
		} 
   	} 
	if(!validDestinationClient){
		sendToClient("incorrect");
	}
    }
/******Send message to a neighbor router********/

    /*cleanup */
    close(udpSocket);
    return 0;
}
