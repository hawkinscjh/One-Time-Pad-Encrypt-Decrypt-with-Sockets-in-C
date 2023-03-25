#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

const char validChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

void error(const char *msg) { 
  perror(msg); 
  exit(0); 
}

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber, 
                        char* hostname){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);

  // Get the DNS entry for this host name
  struct hostent* hostInfo = gethostbyname("localhost"); 
  if (hostInfo == NULL) { 
    fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
    exit(0); 
  }
  // Copy the first IP address from the DNS entry to sin_addr.s_addr
  memcpy((char*) &address->sin_addr.s_addr, 
        hostInfo->h_addr_list[0],
        hostInfo->h_length);
}

int main(int argc, char *argv[]) {

	// Check usage & args
	if (argc < 4) {
		fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); 
		exit(0);
	}

  char *plaintext = argv[1];
  char *key = argv[2];
  int port = atoi(argv[3]);
  char buffer[1024];
  char output[100000];
  char textPlaintext[100000];
	char textKey[100000];

	int socketFD, charsWritten, charsRead;
  struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;

	// Get text from plaintext file
	FILE* textFile = fopen(plaintext, "r");
	fgets(textPlaintext, 100000, textFile);
	fclose(textFile);
	textPlaintext[strcspn(textPlaintext, "\n")] = '\0';

	// Get text from key file
	FILE* keyFile = fopen(key, "r");
	fgets(textKey, 100000, keyFile);
	fclose(keyFile);
	textKey[strcspn(textKey, "\n")] = '\0';
	
	// Check string lengths
	int textLength = strlen(textPlaintext);
	int keyLength = strlen(textKey);
	if (keyLength < textLength) 
  {
		fprintf(stderr, "CLIENT ERROR: key '%s' is too short\n", key);
		exit(1);
	}
	
	// Check message for valid characters
	for (int i = 0; i < textLength; i++) 
  {
		for (int j = 0; j < 28; j++) 
    {
			if (j == 27) 
      {
				fprintf(stderr, "CLIENT ERROR: input contains bad characters\n");
				exit(1);
			}
			if (textPlaintext[i] == validChars[j]) 
      {
				break;
			}
		}
	}
	// Check key for valid characters
	for (int i = 0; i < keyLength; i++) 
  {
		for (int j = 0; j < 28; j++) 
    {
			if (j == 27) 
      {
				fprintf(stderr, "CLIENT ERROR: key contains bad characters\n");
			}
			if (textKey[i] == validChars[j]) 
      {
				break;
			}
		}
	}

	// Create a socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFD < 0) 
  {
    error("CLIENT: ERROR opening socket");
  }

  // Set up the server address struct
  setupAddressStruct(&serverAddress, port, "localhost");
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
		error("CLIENT: ERROR connecting");

  // Check if appropriate server is being connected to by client
  char *check = "decode";
  charsWritten = send(socketFD, check, strlen(check), 0);
  memset(buffer, '\0', sizeof(buffer));
	if (charsWritten < 0)
  {
		error("CLIENT: ERROR writing from socket");
  }
	charsRead = 0;
	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); 
	if (charsRead < 0)
  {
    error("CLIENT: ERROR reading from socket");
  }
	if (strcmp(buffer, "Denied") == 0)
  {
		fprintf(stderr, "ERROR: could not contact dec_server on port %d\n", port);
		exit(2);
	}

	// Setup message separated by newline 
	memset(buffer, '\0', sizeof(buffer));
	sprintf(buffer, "%s\n%s\n", plaintext, key);

	// Send message to server
  // Write to the server
	charsWritten = send(socketFD, buffer, strlen(buffer), 0);
	if (charsWritten < 0)
  {
    error("CLIENT: ERROR writing to socket");
  }
	if (charsWritten < strlen(buffer)) 
  {
    fprintf(stderr, "CLIENT: WARNING: Not all data written to socket!\n");
  }

	// Get return message from server
  // Clear out the buffer again for reuse
	memset(buffer, '\0', sizeof(buffer));
	// Read data from the socket, leaving \0 at end
	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);
	if (charsRead < 0) 
  {
    error("CLIENT: ERROR reading from socket");
  }
	// Open returned file and print to output
	FILE* recvFile = fopen(buffer, "r");
	fgets(output, 100000, recvFile);
	fclose(recvFile);
	// Delete file
	remove(buffer);
	printf("%s\n", output);

  // Close the socket
	close(socketFD);
	return 0;
}