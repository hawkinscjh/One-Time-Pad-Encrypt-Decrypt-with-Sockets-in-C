#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>

void decode(char key[], char input[], char output[], int length);
void convertCharToInt(char input[], int output[], int length);
void convertIntToChar(int input[], char output[], int length);

// Error function used for reporting issues
void error(const char *msg) {
  perror(msg);
  exit(1);
} 

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);
  // Allow a client at any address to connect to this server
  address->sin_addr.s_addr = INADDR_ANY;
}


int main(int argc, char *argv[]) 
{
	// Check usage & args
	if (argc < 2) {
		fprintf(stderr,"USAGE: %s port\n", argv[0]);
		exit(1);
	}

	int connectionSocket, charsRead, status;
  int portNumber = atoi(argv[1]);
	char buffer[1024];
	char inputFileName[1024];
	char keyFileName[1024];
	char inputText[100000];
	char keyText[100000];
	char outputText[100000];
	struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);
	pid_t pid, sid;

  // Create the socket that will listen for connections
  int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (listenSocket < 0) 
  {
    error("SERVER: ERROR opening socket");
  }

	// Set up the address struct for the server socket
  setupAddressStruct(&serverAddress, portNumber);

	// Associate the socket to the port
  if (bind(listenSocket, 
          (struct sockaddr *)&serverAddress, 
          sizeof(serverAddress)) < 0)
  {
    error("SERVER: ERROR on binding");
  }

  // Start listening for connetions. Allow up to 5 connections to queue up
  listen(listenSocket, 5); 
  
  // Accept a connection, blocking if one is not available until one connects
  while(1)
  {
		// Accept the connection request which creates a connection socket
    connectionSocket = accept(listenSocket, 
                (struct sockaddr *)&clientAddress, 
                &sizeOfClientInfo); 
    if (connectionSocket < 0){
      error("SERVER: ERROR on accept");
    }

		// Fork
		pid = fork();
    
		switch (pid) 
    {
			// Error
			case -1:
      {
				error("SERVER: ERROR during fork()");
				exit(0);
				break;
      }
			// Child
			case 0:
      {
        // Clear buffer for reuse
        memset(buffer, '\0', sizeof(buffer));

        // Check right client connection
				charsRead = recv(connectionSocket, buffer, 1023, 0);
				if (charsRead < 0)
        {
          error("ERROR reading from socket");
        }
				if (strcmp(buffer, "decode") != 0)
        {
					charsRead = send(connectionSocket, "Denied", 6, 0);
					exit(2);
				} else
        {
					charsRead = send(connectionSocket, "Accepted", 8, 0);
					charsRead = 0;
        }

        // Clear buffer for reuse
				// Read client message from socket
        memset(buffer, '\0', 1024);
				charsRead = recv(connectionSocket, buffer, sizeof(buffer)-1, 0);
				if (charsRead < 0)
        {
          error("SERVER: ERROR reading from socket");
        }
					
				// Create message and key
				char newline[2] = {'\n', '\0'};
				char *token = strtok(buffer, newline);
				strcpy(inputFileName, token);
				token = strtok(NULL, newline);
				strcpy(keyFileName, token);

        // Copy inputFile to string
        FILE* inputFile = fopen(inputFileName, "r");
        fgets(inputText, 100000, inputFile);
        fclose(inputFile);
        inputText[strcspn(inputText, "\n")] = '\0';

        // Copy keyFile to string
        FILE* keyFile = fopen(keyFileName, "r");
        fgets(keyText, 100000, keyFile);
        fclose(keyFile);
        keyText[strcspn(keyText, "\n")] = '\0';

        // Decode message using key
        decode(keyText, inputText, outputText, strlen(inputText));

				// Write output to file
				int newPid = getpid();
				char newFile[1024];
				sprintf(newFile, "%d", newPid);

				// Open and write output text to file
				FILE* newFD = fopen(newFile, "w+");
				fprintf(newFD, "%s", outputText);
				fclose(newFD);

				// Send decoded text to client
				charsRead = send(connectionSocket, newFile, strlen(newFile), 0);
				if (charsRead < 0)
        {
          error("SERVER: ERROR writing to socket");
        }
				
				// Close connection socket
				close(connectionSocket);
				connectionSocket = -1;
				exit(0);

				break;
      }
      // Parent
			default:
      {
        pid_t parentPid = waitpid(pid, &status, WNOHANG);
      }
    }
    close(connectionSocket);
	}
	// Close the listening socket
	close(listenSocket);
	return 0;
}

// Decode encoded messages
void decode(char key[], char input[], char output[], int length) 
{
	// Convert input array to integers
  // Convert key to integers
	int integerInput[length];
	convertCharToInt(input, integerInput, length);
	int integerKey[length];
	convertCharToInt(key, integerKey, length);

	// Create intgerOutput array from int
	int interOutput[length];
	for (int i = 0; i < length; i++) 
  {
		interOutput[i] = integerInput[i] - integerKey[i];
		if (interOutput[i] < 0)
    {
      interOutput[i] += 27;
    }
	}

	// Convert output to char
	convertIntToChar(interOutput, output, length);
	output[length] = '\0';
}

//convert a character to an integer
void convertCharToInt(char input[], int output[], int length) 
{ 
  for (int i = 0; i < length; i++)
  {
    char c = input[i];
    if (c == ' '){
      output[i] = 26;
    }
    else {
      output[i] = (c - 'A');
    }
  }
}

//convert an integer to a character
void convertIntToChar(int input[], char output[], int length) 
{ 
  for (int i = 0; i < length; i++)
  {
    char c = input[i];
    if (c == 26){
      output[i] = ' ';
    }
    else {
      output[i] = (c + 'A');
    }
  }
}