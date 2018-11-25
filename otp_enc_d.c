#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	socklen_t sizeOfClientInfo;
	char buffer[256];
	struct sockaddr_in serverAddress, clientAddress;
	int pid;
	char encryptorSignature[64] = "OTP_ENC REQUESTING ENCRYPTION\0";
	char plaintext[256];
	char key[256];

	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

	// while listening, accept connections, blocking if one is not available until one connects
	while(1){
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0){ 
			error("ERROR on accept");
		} else {
			// split off a child-worker
			pid = fork();
		}

		// child-worker
		if(pid == 0){
			// Get the signature from the client and verify it
			memset(buffer, '\0', 256);
			charsRead = recv(establishedConnectionFD, buffer, 255, 0); // Read the client's message from the socket
			if (charsRead < 0) error("ERROR reading from socket");
			if(strcmp(buffer, encryptorSignature) != 0){
				fprintf(stderr, "Invalid signature! Closing connection.\n");
			} else {
				// Send a Success message back to the client, then read some more
				charsRead = send(establishedConnectionFD, "1", 39, 0); // Send success back
				if(charsRead < 0){
					error("ERROR writing to socket");
				} else {
					// read plaintext
					memset(buffer, '\0', 256);
					charsRead = recv(establishedConnectionFD, buffer, 255, 0);
					if(charsRead < 0){
						error("ERROR reading plaintext data from socket");
					} else {
						strcpy(plaintext, buffer);
					}
					memset(buffer, '\0', 256);
					charsRead = recv(establishedConnectionFD, buffer, 255, 0);
					if(charsRead < 0){
						error("ERROR reading key data from socket");
					} else {
						strcpy(key, buffer);
					}
				}
				int i;
				int plainNum;
				int keyNum;
				int outputNum;
				char output[256] = "";
				for(i=0; i<strlen(plaintext)-1; ++i){
					plainNum = plaintext[i] - 'A';
					keyNum = key[i] - 'A';
					// shift spaces to '26th' character of alphabet
					if(plainNum == -33){
						plainNum = 26;
					}
					if(keyNum == -33){
						keyNum = 26;
					}
					outputNum = ( (keyNum + plainNum) % 27 );
					if(outputNum == 26){
						// convert to space
						output[i] = 32;
					} else {
						// convert to capital
						output[i] = outputNum + 'A';
					}
				}
				charsRead = send(establishedConnectionFD, output, sizeof(output), 0);
				if(charsRead < 0){
					error("ERROR writing to socket");
				}
			}
			close(establishedConnectionFD); // Close the existing socket which is connected to the client
		}
	}
	close(listenSocketFD); // Close the listening socket
	return 0;
}
