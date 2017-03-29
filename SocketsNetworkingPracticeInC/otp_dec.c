#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg) 
{
 	perror(msg); 
	exit(1); 
} // Error function used for reporting issues

char* combine(char* ciphertext, char* encryptionkey, int ciphertextLength)
{
	char* fullText = malloc((ciphertextLength * 3) * sizeof(char));
	memset(fullText, '\0', sizeof(fullText));
	
	int i;

	for (i = 0; i < ciphertextLength; i++)
	{
		fullText[i] = ciphertext[i];
	}

	
	fullText[ciphertextLength] = '\n';

	int j;
	i = 0;

	for (j = ciphertextLength+1; j <= (ciphertextLength * 2 + 1); j++)
	{
		fullText[j] = encryptionkey[i];
		i++;
	}

	
	return fullText;	
}


int main(int argc, char *argv[])
{
	if (argc < 4) 
	{ 
		fprintf(stderr,"USAGE: %s plaintext key port\n", argv[0]); 
		exit(0); 
	} // Check usage & args


	/* GET CIPHERTEXT FROM FILE */

	// Read from the file
	char* ciphertextFile = argv[1];
	FILE* fp;
	fp = fopen(ciphertextFile, "r+"); // open file at beginning for reading
	
	if (fp == NULL)
	{
		error("ERROR");
	}

	// get the line of ciphertext
	char* ciphertext = NULL;
	ssize_t len1; 
	size_t len = 0;
	len1 = getline(&ciphertext, &len, fp);

	if (len1 == -1)
	{
		error("ERROR: File not readable.");
	}

	int ciphertextLength = len1;
	fclose(fp);

	/*** Now ciphertext holds the ciphertext and ciphertextLength holds the length of the null terminated string***/

	/* GET ENCRYPTION KEY FROM FILE */
	char* encryptionFileName = argv[2];
	FILE* fp1;
	fp1 = fopen(encryptionFileName, "r+");
	
	if (fp1 == NULL)
	{
		error("ERROR");
	}

	// get line of encryption key
	char* encryptionKey = NULL;
	ssize_t len2;
	size_t len3 = 0;
	len2 = getline(&encryptionKey, &len3, fp1);

	if (len2 == -1)
	{
		error("ERROR: File not readable.");
	}

	int encryptionKeyLength = len2;
	fclose(fp1); 

	if (ciphertextLength > encryptionKeyLength)
		error("ERROR: Plaintext length and key length not compatible");

	/*** Now encryptionKey holds the encryption key and encryptionKeyLength holds the length of the null terminated string***/

	// begin networking stuff
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[1000000];
	memset(buffer, '\0', sizeof(buffer));
    
	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	
	if (serverHostInfo == NULL) 
	{ 
		fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
		exit(0); 
	}

	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) 
		error("CLIENT: ERROR opening socket");
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");

	// Validate we're talking to otp_dec_d

	char validationMessage[] = "9101112";
	char newLineChar[] = "\n";

	// Send validation message to otp_dec_d
	//write(socketFD, validationMessage, sizeof(validationMessage)); // Write to the server
	send(socketFD, validationMessage, strlen(validationMessage), 0);
	
	// get validation message from otp_dec_d
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	//read(socketFD, buffer, sizeof(buffer - 1)); // Read data from the socket, leaving \0 at end
	recv(socketFD, buffer, sizeof(buffer - 1), 0);

	if (strcmp(buffer, "13141516") != 0)
	{
		close(socketFD);
		error("ERROR: otp_dec is not talking to otp_dec_d.");
	}

	// Now that we know we are talking to otp_dec_d, send ciphertext and key
	// first construct one message consisting of [ciphertext]\n[key]
	
	char* totalMessage;
	totalMessage = combine(ciphertext, encryptionKey, ciphertextLength);

	//charsWritten = write(socketFD, totalMessage, sizeof(totalMessage));	
	charsWritten = send(socketFD, totalMessage, strlen(totalMessage), 0);	

	if (charsWritten == 0)
		error("ERROR: Did not write to otp_dec_d");

	// Read the plaintext back from otp_dec_d
	char plaintext[1000000];
	memset(plaintext, '\0', sizeof(plaintext));
	
	//charsRead = read(socketFD, ciphertext, sizeof(ciphertext - 1));
	charsRead = recv(socketFD, plaintext, sizeof(plaintext), 0);

	if (charsRead == 0)
		error("Did not get plaintext back");

	printf("%s\n", plaintext);

	close(socketFD); // Close the socket
	return 0;
}
