#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

const char alphabet[27] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', ' ' };

void error(const char *msg) 
{ 
	perror(msg); 
	exit(1); 
} // Error function used for reporting issues

// key has to be either larger or equal in size to plaintext 
void checkKeyLength(int plaintextLength, int keyLength)
{
	int plainLength = plaintextLength;
	int kLength = keyLength;

	if (kLength < plainLength)
	{
		error("ERROR: The key is not of sufficient size.");
	}
}

int convertCharToNum(char c1)
{
	int index = 0;

	for (index; index < 27; index++)
	{
		if (c1 == alphabet[index])
			return index;
	}
	
}

char convertNumToChar(int n1)
{
	return alphabet[n1];	
}

int sumAlphabetNums(int n1, int n2)
{
	return n1 + n2;
}

int getNewAlphabetNum(int sum)
{
	int newAlphabetNum;

	if (sum <= 27)
	{
		newAlphabetNum = sum % 27;
	}

	else
	{
		sum = sum - 27;
		newAlphabetNum = sum % 27;
	}	

	return newAlphabetNum;	
}

char* generateCiphertext(char* plaintext, char* key, int plaintextLength, int keyLength)
{
	checkKeyLength(plaintextLength, keyLength);

	int length = plaintextLength;
	
	char* ciphertext = malloc((length+1) * sizeof(char));

	int i = 0;

	for (i; i < length+1; i++)
	{
		int plaintextNum = convertCharToNum(plaintext[i]);
		int keyNum = convertCharToNum(key[i]);
		int sum = sumAlphabetNums(plaintextNum, keyNum);
		int newAlphabetNum = getNewAlphabetNum(sum);
		char nextChar = convertNumToChar(newAlphabetNum);
		ciphertext[i] = nextChar;
	}

	return ciphertext;
}

int main(int argc, char *argv[])
{
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead, status;
	socklen_t sizeOfClientInfo;
	char buffer[1000000];
	memset(buffer, '\0', sizeof(buffer));
	pid_t PID;
	struct sockaddr_in serverAddress, clientAddress;

	if (argc < 2) 
	{ 
		fprintf(stderr,"USAGE: %s port\n", argv[0]); 
		exit(1); 
	} // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) 
		error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

	sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect

	while (1)
	{
		// Accept a connection, blocking if one is not available until one connects
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0) 
			error("ERROR on accept");

		int status;
		PID = fork();

		if (PID < 0)
		{
			error("SERVER: Problem with forking into new process.");
			exit(EXIT_FAILURE);
		}

		if (PID == 0)// child process. We have the connection in the establishedConnectionFD
		{	// Get the message from the client and display it
			// charsRead holds how many characters came thru the pipe
			// buffer holds the message
			memset(buffer, '\0', sizeof(buffer));
			recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0);
	
			// the buffer does not contain the verification message we need
			if (strcmp(buffer, "1234") != 0)
			{
				char failedAuthentication[] = "ERROR: Not the correct validation code.";
				//write(establishedConnectionFD, failedAuthentication, sizeof(failedAuthentication));
				send(establishedConnectionFD, failedAuthentication, strlen(failedAuthentication), 0);
				_Exit(2);
			}

			// verification message is correct, send a message back to verify
			else	
			{
				char* successfulAuthentication = "5678";
				//write(establishedConnectionFD, successfulAuthentication, sizeof(successfulAuthentication));
				send(establishedConnectionFD, successfulAuthentication, strlen(successfulAuthentication), 0);
			}

			// now that it has been verified, get the message
			memset(buffer, '\0', sizeof(buffer));
			
			int messageSize;
			//charsRead = read(establishedConnectionFD, buffer, sizeof(buffer));
			messageSize = recv(establishedConnectionFD, buffer, sizeof(buffer) - 1, 0);
			char* plaintext = malloc(messageSize * sizeof(char));
			strcpy(plaintext, buffer);

			memset(buffer, '\0', sizeof(buffer));

			int keySize;
			keySize = recv(establishedConnectionFD, buffer, sizeof(buffer) - 1, 0);
			char* key = malloc(keySize * sizeof(char));
			strcpy(key, buffer);

			char* ciphertext = generateCiphertext(plaintext, key, messageSize, keySize);
			
			//write(establishedConnectionFD, ciphertext, sizeof(ciphertext));
			send(establishedConnectionFD, ciphertext, strlen(ciphertext), 0);
			close(establishedConnectionFD);
			close(listenSocketFD);

			exit(0);
		}

		else
		{	
			close(establishedConnectionFD);

			do
			{
				PID = waitpid(-1, &status, WNOHANG);
			} while (PID > 0);
		}
	}

	return 0; 
}
