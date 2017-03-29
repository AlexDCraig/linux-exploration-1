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

// combine plaintext and encryptionkey into one string separated by a 
// \n. this will be sent to the encryption algorithm which will parse it
// and encrypt it
char* combine(char* plaintext, char* encryptionkey, int plaintextlength)
{
	char* fullText = malloc((plaintextlength * 3) * sizeof(char));
	memset(fullText, '\0', sizeof(fullText));
	
	int i;

	for (i = 0; i < plaintextlength; i++)
	{
		fullText[i] = plaintext[i];
	}

	
	int j;
	i = 0;

	for (j = plaintextlength; j < (plaintextlength * 2 + 1); j++)
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


	/* GET PLAINTEXT FROM FILE */

	// Read from the file
	char* plaintextFileName = argv[1];
	FILE* fp;
	fp = fopen(plaintextFileName, "r+"); // open file at beginning for reading
	
	if (fp == NULL)
	{
		error("ERROR");
	}

	// get the line of plaintext
	char* plainText = NULL;
	ssize_t len1; 
	size_t len = 0;
	len1 = getline(&plainText, &len, fp);

	if (len1 == -1)
	{
		error("ERROR: File not readable.");
	}

	int plainTextLength = len1;
	fclose(fp);

	/*** Now plainText holds the plaintext and plainTextLength holds the length of the null terminated string***/

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

	if (plainTextLength > encryptionKeyLength)
		error("ERROR: Plaintext length and key length not compatible");

	/*** Now encryptionKey holds the encryption key and encryptionKeyLength holds the length of the null terminated string***/

	// begin networking stuff
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[256];
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

	// Validate we're talking to otp_enc_d

	char validationMessage[] = "1234";
	char newLineChar[] = "\n";

	// Send validation message to otp_enc_d
	//write(socketFD, validationMessage, sizeof(validationMessage)); // Write to the server
	send(socketFD, validationMessage, strlen(validationMessage), 0);
	
	// get validation message from otp_enc_d
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	//read(socketFD, buffer, sizeof(buffer - 1)); // Read data from the socket, leaving \0 at end
	recv(socketFD, buffer, sizeof(buffer - 1), 0);

	if (strcmp(buffer, "5678") != 0)
	{
		printf("BUFFER: %s", buffer);
		error("ERROR: otp_enc is not talking to otp_enc_d.");
	}

	// Now that we know we are talking to otp_enc_d, send plaintext and key
	// first construct one message consisting of [plaintext]\n[key]
	
	char* totalMessage;
	totalMessage = combine(plainText, encryptionKey, plainTextLength);

	//charsWritten = write(socketFD, totalMessage, sizeof(totalMessage));	
	charsWritten = send(socketFD, totalMessage, strlen(totalMessage), 0);	

	if (charsWritten == 0)
		error("ERROR: Did not write to otp_enc_d");

	char ciphertext[260];
	memset(ciphertext, '\0', sizeof(ciphertext));
	
	//charsRead = read(socketFD, ciphertext, sizeof(ciphertext - 1));
	charsRead = recv(socketFD, ciphertext, sizeof(ciphertext), 0);

	if (charsRead == 0)
		error("Did not get ciphertext back");

	// print the ciphertext to stdout
	printf("%s", ciphertext);

	close(socketFD); // Close the socket
	return 0;
}
