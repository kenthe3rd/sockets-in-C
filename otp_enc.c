#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues
int stringCheck(const char*);
int main(int argc, char *argv[])
{
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[256];
	int keyIsValid;
	int keylen;
	int plainTextIsValid;
	int plainlen;
	FILE* fp;
    
	// Check usage & args
	if(argc < 4){ 
		fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); exit(0); 
	} else {
		// check plaintext
		fp = fopen(argv[1], "r");
		memset(buffer, '\0', sizeof(buffer));
		fgets(buffer, 255, (FILE*)fp);
		plainTextIsValid = stringCheck(buffer);
		plainlen = strlen(buffer);
		if(!plainTextIsValid){
			fprintf(stderr, "the plaintext contained one or more invalid characters\n");
			fclose(fp);
			exit(1);
		}
		fclose(fp);

		// check key
		fp = fopen(argv[2], "r");
		memset(buffer, '\0', sizeof(buffer));
		fgets(buffer, 255, (FILE*)fp);
		plainTextIsValid = stringCheck(buffer);
		keylen = strlen(buffer);
		if(!plainTextIsValid){
			fprintf(stderr, "the key contained one or more invalid characters\n");
			fclose(fp);
			exit(1);
		}
		fclose(fp);

		//compare string lengths
		if(keylen < plainlen){
			fprintf(stderr, "ERROR: key length less than plaintext length\n");
			exit(1);
		}
	}

	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("os1.engr.oregonstate.edu"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");

	// verify connection to otp_enc_d
	memset(buffer, '\0', sizeof(buffer));
	strcpy(buffer, "OTP_ENC REQUESTING ENCRYPTION");
	charsWritten = send(socketFD, buffer, strlen(buffer), 0);
	if(charsWritten < 0){
		fprintf(stderr, "FAILED TO WRITE TO SERVER\n");
	} else if(charsWritten < strlen(buffer)){
		fprintf(stdout, "DID NOT WRITE ALL DATA TO SOCKET\n");
	}
	memset(buffer, '\0', sizeof(buffer));
	charsRead = recv(socketFD, buffer, sizeof(buffer) -1, 0);
	if(charsRead < 0){
		error("CLIENT: ERROR reading from socket\n");
	} else if(strcmp(buffer, "1") != 0){
		error("CONNECTION ERROR: FAILED TO CONFIRM CONNECTION TO OTP_ENC_D\n");
	} else {
		// Get input message from user
		printf("CLIENT: Enter text to send to the server, and then hit enter: ");
		memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer array
		fgets(buffer, sizeof(buffer) - 1, stdin); // Get input from the user, trunc to buffer - 1 chars, leaving \0
		buffer[strcspn(buffer, "\n")] = '\0'; // Remove the trailing \n that fgets adds

		// Send message to server
		charsWritten = send(socketFD, buffer, strlen(buffer), 0); // Write to the server
		if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
		if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");

		// Get return message from server
		memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
		charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
		if (charsRead < 0) error("CLIENT: ERROR reading from socket");

		printf("CLIENT: I received this from the server: \"%s\"\n", buffer);
	}
	close(socketFD); // Close the socket
	return 0;
}

int stringCheck(const char* input){
	char temp[256] = "";
	strcpy(temp, input);
	int i;
	for(i=0; i<strlen(temp) - 1; ++i){
		if( temp[i] != ' ' && (temp[i] < 'A' || temp[i] > 'Z') ){
			return 0;
		}
	}
	return 1;
}