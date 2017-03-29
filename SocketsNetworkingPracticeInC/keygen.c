#include <stdio.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

char* generateAlphabet();

char grabRandomSymbol(char*);

char* generateKeygen(char*, int, char*[]);

// argv[0] contains name of prog
// rest is command line arguments
// argc is # of arguments
int main(int argc, char* argv[])
{
	time_t t;
	srand((unsigned) time(&t));
	char* alphabet = generateAlphabet();
	char* keygen = generateKeygen(alphabet, argc, argv);		
	printf("%s", keygen);
	return 0;
}

char* generateAlphabet()
{
	char* alphabet = malloc(27 * sizeof(char));
	alphabet[0] = 'A';
	alphabet[1] = 'B';
	alphabet[2] = 'C';
	alphabet[3] = 'D';
	alphabet[4] = 'E';
	alphabet[5] = 'F';
	alphabet[6] = 'G';
	alphabet[7] = 'H';
	alphabet[8] = 'I';
	alphabet[9] = 'J';
	alphabet[10] = 'K';
	alphabet[11] = 'L';
	alphabet[12] = 'M';
	alphabet[13] = 'N';
	alphabet[14] = 'O';
	alphabet[15] = 'P';
	alphabet[16] = 'Q';
	alphabet[17] = 'R';
	alphabet[18] = 'S';
	alphabet[19] = 'T';
	alphabet[20] = 'U';
	alphabet[21] = 'V';
	alphabet[22] = 'W';
	alphabet[23] = 'X';
	alphabet[24] = 'Y';
	alphabet[25] = 'Z';
	alphabet[26] = ' ';
	 
	return alphabet;
}

char grabRandomSymbol(char* alphabet)
{
	int element = rand() % 27;
	return alphabet[element];
}

char* generateKeygen(char* alphabet, int argc, char* argv[])
{
	if (argv[1] == NULL)
		exit(1);

	int keygenLength = atoi(argv[1]);

	char* keygen = malloc(keygenLength * sizeof(char));

	int i = 0;

	for (i; i < keygenLength; i++)
	{
		keygen[i] = grabRandomSymbol(alphabet);	
	}

	keygen[keygenLength] = '\n';

	return keygen;
}	
