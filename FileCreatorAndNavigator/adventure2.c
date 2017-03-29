#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

struct Room // to arrange rooms in files
{
	struct Room** connections; // An array of pointers to Rooms to hold what Rooms this Room is connected with
	FILE* roomFile; 
	char* name;
	char* type;
	int connectionNumber;
	char* fileName;
};

// construct the directory in which the files will be held
char* makeDirectory()
{
	char firstPart[] = "hoffera.rooms.";
	char* roomDirectory = malloc(sizeof(char) * 30);
	snprintf(roomDirectory, 30, "%s%d", firstPart, getpid());
	mkdir(roomDirectory, 0755);
	return roomDirectory;
}

// allocate some memory to be used as an array of structs
struct Room** makeStruct()
{
	struct Room** newStruct = malloc(7 * sizeof(struct Room*));
	int i = 0;

	for (i; i < 7; i++)
	{
		newStruct[i] = malloc(sizeof(struct Room));
	}	

	return newStruct;
}

// get an array of strings that holds each room name in a random order
void assignRoomNames(struct Room** holder)
{
	char** roomNames = malloc(10 * sizeof(char*));

	int j = 0;

	for (j; j < 10; j++)
	{
		roomNames[j] = (char*)malloc(20);
	}

	char* roomName1 = "Palace";
	char* roomName2 = "Dungeon";
	char* roomName3 = "Swamp";
	char* roomName4 = "Alleyway";
	char* roomName5 = "Rooftop";
	char* roomName6 = "Bar";
	char* roomName7 = "Den";
	char* roomName8 = "Basement";
	char* roomName9 = "Museum";
	char* roomName10 = "Store";

	roomNames[0] = roomName1;
	roomNames[1] = roomName2;
	roomNames[2] = roomName3;
	roomNames[3] = roomName4;
	roomNames[4] = roomName5;
	roomNames[5] = roomName6;
	roomNames[6] = roomName7;
 	roomNames[7] = roomName8;
	roomNames[8] = roomName9;
	roomNames[9] = roomName10;

	j = 0;

	// SHUFFLE AROUND THE ARRAY ELEMENTS
	// We get a random index using the formula 
	// (currentIndex + randomNumber) int divide by the max number of rand() / (size of array - current element) + 1

	for (j; j < 9; j++)
	{
		int i = j + rand() / (RAND_MAX / (10 - j) + 1);
		char* temp = roomNames[i];
		roomNames[i] = roomNames[j];
		roomNames[j] = temp;
	}
	
	// we can only take 7 names
	char** roomNames2 = malloc(7 * sizeof(char*));
	
	for (j = 0; j < 7; j++)
	{
		roomNames2[j] = roomNames[j];
	}
	
	// now assign the names
	for (j = 0; j < 7; j++)
	{
			holder[j]->name = malloc(50 * sizeof(char));
			memset(holder[j]->name, '\0', sizeof(holder[j]->name));
			strcpy(holder[j]->name, roomNames2[j]);
	}
}

// open the files in the right directory to be used later
FILE** generateRoomFiles(char* directory, struct Room** holder)
{
	FILE** files = malloc(sizeof(FILE*) * 7); // an array of pointers to files
	
	char path[35]; // Path will look like: (directory)/(roomFile.txt), how we will access these files later for reading
	memset(path, '\0', sizeof(path));
	
	snprintf(path, 34, "%s/%s", directory, "room1.txt"); // generate path to this file by concatenating the directory with the room file name
	files[0] = fopen(path, "a+"); // a+ allows us to open for reading and appending to EOF
	holder[0]->fileName = "room1.txt"; // I used ".txt" because I don't want wonky behavior from not having an extension on it
	
	snprintf(path, 34, "%s/%s", directory, "room2.txt");
	files[1] = fopen(path, "a+");
	holder[1]->fileName = "room2.txt";

	snprintf(path, 34, "%s/%s", directory, "room3.txt");
	files[2] = fopen(path, "a+");
	holder[2]->fileName = "room3.txt";

	snprintf(path, 34, "%s/%s", directory, "room4.txt");
	files[3] = fopen(path, "a+");
	holder[3]->fileName = "room4.txt";

	snprintf(path, 34, "%s/%s", directory, "room5.txt");
	files[4] = fopen(path, "a+");
	holder[4]->fileName = "room5.txt";

	snprintf(path, 34, "%s/%s", directory, "room6.txt");
	files[5] = fopen(path, "a+");
	holder[5]->fileName = "room6.txt";

	snprintf(path, 34, "%s/%s", directory, "room7.txt");
	files[6] = fopen(path, "a+");
	holder[6]->fileName = "room7.txt";

	return files;
}

// get an array of strings that hold each room type
char** _generateRoomTypes()
{
	char** roomTypes = malloc(3 * sizeof(char*));

	int j = 0;

	for (j; j < 3; j++)
	{
		roomTypes[j] = (char*)malloc(20);
	}

	roomTypes[0] = "START_ROOM";
	roomTypes[1] = "END_ROOM";
	roomTypes[2] = "MID_ROOM";

	return roomTypes;
}

// helper function for assignroomtypes
// go through the bool and see if that room type has been taken
// NOTE: mid_room is never identified as taken because it can be many rooms' types
int _checkIfTaken(int* boolArray, int index)
{
	if (boolArray[index] == 1)
		return 1;

	else
		return 0;
} 

// give each room a random room type with start room and end room used only once
void assignRoomTypes(struct Room** roomHolder)
{
	char** roomTypes = _generateRoomTypes();

	int i = 0;

	int roomTypesTaken[3] = {0, 0, 0}; // Element 0 corresponds to START_ROOM, element 1 is END_ROOM, 2 is MID_ROOM. The first two elements are switched to 1 if they become taken by a room because those rooms can only be assigned once

	for (i; i < 7; i++)
	{
		// give room file a random room type
		int thisRoomType = rand() % 3;

		while (_checkIfTaken(roomTypesTaken, thisRoomType) == 1)
			thisRoomType = rand() % 3; // keep randomizing if that element has already been assigned

		if (thisRoomType == 0) // if its the start room, mark the start room as having been taken so it doesn't get assigned again
			roomTypesTaken[0] = 1;

		else if (thisRoomType == 1) // same thing for the end room
			roomTypesTaken[1] = 1;

		roomHolder[i]->type = roomTypes[thisRoomType]; // assign the room's type to be that room type
	}
	
}	

// We have to dynamically allocate each Room's connections pointer as well as the connections pointers' array
void allocateRoomConnections(struct Room** holder)
{
	int a = 0;
	int b = 0;
	
	for (a; a < 7; a++)
	{
		holder[a]->connections = malloc(sizeof(struct Room) * 7);
		
		for(b; b < 6; b++)
			holder[a]->connections[b] = 0;	// Initialize their connection to 0 so that we can loop through this array later to find where an unoccupied element is
	}
}

// CONNECTING THE ROOMS
// returns an element to a room that needs connections or 7 (invalid element) if there are no more connections to be made
int checkIfGraphFull(struct Room** rooms)
{
	int a = 0;
	
	for(a; a < 7; a++)
	{
		if(rooms[a]->connectionNumber < 3) // doesn't have enough connections
		{
			return a; // return the element of a valid room to connect to
		}
	}
	
	return 7; // there are no rooms to connect to
}

// Loop through Room A and see if Room B is already connected to it
int areRoomsConnected(struct Room* A, struct Room* B)
{
	int m = 0;
	
	while ((A->connections[m] != 0) && (m < 7))
	{
		if (strcmp(A->connections[m]->name, B->name) == 0)
			return 1; // yes the rooms are connected
			
		else
			m += 1;
	}
	
	return 0; // no the rooms are not connected
}

// checks to see if rooms A, B are the same
// it does so on the basis of their name
int areSameRoom(struct Room* A, struct Room* B)
{
	int compare = strcmp(A->name, B->name);
	
	if (compare == 0)
		return 1; // yes, same room
	
	else	
		return 0; // no, diff rooms
}

// checks to see if Room A already has the maximum number of connections
int isMaxSize(struct Room* A)
{
	if (A->connectionNumber >= 6)
		return 1; // yes, max size
	
	else	
		return 0; // no, not max size
}

// connects rooms A and B which are valid
void connect(struct Room* A, struct Room* B)
{
	int counter = 0;
	
	// Progress through connections until you find a valid element that is currently unoccupied
	while (A->connections[counter] != 0)
	{
		++counter;
	}

	A->connections[counter] = B; // A's furthest unoccupied element now holds Room B
	A->connectionNumber++; // A now has more connections

	counter = 0;
	
	// same logic as above while loop
	while (B->connections[counter] != 0)
	{
		++counter;
	}
		
	B->connections[counter] = A;
	B->connectionNumber++;
}

// addRoom is a room that needs a connection, loop until it finds a valid connection
// A valid connection is identified as a room where the room is not full (<6), the rooms are not the same, and the rooms are not already connected
void addConnection(struct Room* addRoom, struct Room** holder)
{
	int element;
	
	while(1) // loops until broken
	{
		element = rand() % 7; // generate index [0, 6]
		
		if((isMaxSize(holder[element]) == 0) && (areSameRoom(addRoom, holder[element]) == 0) && (areRoomsConnected(holder[element], addRoom) == 0))
		{	
		// if the element is not full and if the element is not the same as the room it's trying to add to and the element isn't already connected
			connect(addRoom, holder[element]); // basic connection function, glue these two rooms together to form a valid connection
			break;
		}
	}
}

// Uses Brewster's method from lecture
void fillGraph(struct Room** rooms)
{
	int element;
	
	while((element = checkIfGraphFull(rooms)) != 7) // checkIfGraphFull returns a valid index if there is one. If there isn't one, it returns 7, which is an invalid array element
	{
		addConnection(rooms[element], rooms); // rooms[element] holds a room that needs a connection. Send it to a function where it can find a valid connection
	}
}

// We have generated the file pointers (contained in FILE** files), but they need to be assigned to the Rooms
void assignFilePointers(FILE** files, struct Room** holder)
{
	int x = 0;
	
	for(x; x < 7; x++)
	{
		holder[x]->roomFile = files[x];
	}
}

// We have the structs all correct and the files are established. Loop through each room struct and use fprintf to place 
// the room's name, connections, and type into a corresponding file
// fprintpf(FILE*, char*, char* variable) --> print the given string into the file
void writeToFiles(struct Room** holder)
{
	int i, j;
	
	for(i = 0; i < 7; i++)
		fprintf(holder[i]->roomFile, "ROOM NAME: %s\n", holder[i]->name); // note \n at the end of each line
	
	for(i = 0; i < 7; i++)
	{
		while(holder[i]->connections[j] != 0 && j < holder[i]->connectionNumber)
		{
			fprintf(holder[i]->roomFile, "CONNECTION %d: %s\n", j+1, holder[i]->connections[j]->name);
			j++;
		}
	
		j=0;
	}
	
	for(i = 0; i < 7; i++)
		fprintf(holder[i]->roomFile, "ROOM TYPE: %s", holder[i]->type);

}

// searches through all structs to find which one is the start room on the basis of its member variable "type"
struct Room* findStartRoom(struct Room** roomHolder)
{
	int i = 0;

	for (i; i < 7; i++)
	{
		if (strcmp(roomHolder[i]->type, "START_ROOM") == 0)
			return roomHolder[i];
	}
}

// prints the currentRoom's name after a formatted string
void printCurLocation(struct Room* currentRoom, char* directory)
{
	printf("CURRENT LOCATION: ");
	char buff[100];
	memset(buff, '\0', sizeof(buff));

	char path[30]; 
	memset(path, '\0', sizeof(path));
	
	snprintf(path, 29, "%s/%s", directory, currentRoom->fileName);

	FILE* fptr = NULL;
	fptr = fopen(path, "r");

	fseek(fptr, 0, SEEK_SET);

	fread(buff, 1, 10, fptr);

	printf("%s", buff);

	fclose(fptr);

	printf("%s", currentRoom->name);	
}

// takes the current room and prints what rooms its connected to
void printConnections(struct Room* currentRoom, char* directory)
{
	printf("\nPOSSIBLE CONNECTIONS: ");

	int i = 0;

	for (i; i < currentRoom->connectionNumber; i++)
	{
		printf("%s", currentRoom->connections[i]->name);

		if (i != (currentRoom->connectionNumber - 1))
			printf(", ");

		else
			printf(".");
	}
}

// for use with printPrompt
void printWhereTo()
{
	printf("\nWHERE TO? >");
}

// uses helper functions to print the current room's location, what connections it has, and offers the user a chance to change rooms
void printPrompt(struct Room* currentRoom, char* directory)
{
	printCurLocation(currentRoom, directory);
	printConnections(currentRoom, directory);
	printWhereTo(); 
}

// for use with invalid string input
void _printError()
{
	printf("\n\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
}

pthread_mutex_t mutex; // establish a mutex

void* writeTimeToFile() // write time using mutexes, second thread. Write ctime(&mytime) to a file and then read it in the main thread
{
	time_t mytime;
	mytime = time(NULL);	

	FILE* fp;
	fp = fopen("currentTime.txt", "w+"); // a+ allows us to open for reading and appending to EOF
	fprintf(fp, "%s", ctime(&mytime));
	fclose(fp);
}

void createSecondThread(int* validInput)
{
	pthread_t thread2; // create a second thread for printing time
	pthread_mutex_init(&mutex, NULL); // initialize the mutex
	pthread_mutex_lock(&mutex);
	int thread2ID = pthread_create(&thread2, NULL, writeTimeToFile, NULL); // thread2ID holds the thread's id
	pthread_mutex_unlock(&mutex);
	pthread_mutex_destroy(&mutex); // destroy the mutex
	*validInput = -1;
	sleep(1);
}

// returns index of room to go to or -1 if room invalid by comparing each connected room name to that input string
int _verifyInput(char* input, struct Room* currentRoom)
{
	int i = 0;

	for (i; i < currentRoom->connectionNumber; i++)
	{
		if (strcmp(currentRoom->connections[i]->name, input) == 0)
		{
			return i;
		}
	}

	if (strcmp(input, "time") != 0)
	{
		_printError();
		return -1;
	}

	else 
		return -2;
}

// takes validInput bool and currentRoom and gets user input. It then calls verifyInput which returns -1 if invalid text input or the index of the valid
// connection and returns it
int getUserInput(struct Room* r1)
{
	char str[100];
	memset(str, '\0', sizeof(str));
	scanf("%s", str);
	char* userInput = malloc(strlen(str) * sizeof(char));
	strcpy(userInput, str);
	int index = _verifyInput(userInput, r1);
	return index;
}

// if the room passed to this function is the end room, the game should be ended, so set the isGameOver bool to true
void checkIfGameOver(struct Room* r1, int* isGameOver)
{
	if (strcmp(r1->type, "END_ROOM") == 0)
		*isGameOver = 1;
} 

// Takes how many steps the user took, which rooms the user visited, prints them both in formatted manner
void printCongrats(int* numberOfSteps, char** pathToVictory, int* pathCount)
{
	printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
	printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", *numberOfSteps);
	int i = 0;

	for (i; i < *pathCount; i++)
	{
		printf("%s\n", pathToVictory[i]);
	}
}

// This function reads from the currentTime.txt file we outputted to earlier and prints it to the user
void printReadTimeFile()
{
	FILE* fp;
	char buffer[255];
	memset(buffer, '\0', sizeof(buffer));
	int c;
	int pos = 0;

	fp = fopen("currentTime.txt", "r"); // open currentTime.txt
	
	do
	{
		c = fgetc(fp); // get a character from the file
		
		if (c != EOF) // if the character does not represent the end of the file
		{
			buffer[pos] = (char)c; // buffer at array element now holds one of the chars necessary
			pos++;
		}
	} while (c != EOF && c != '\n'); // while c isn't the end of file or a new line. This function reads 1 line

	printf("\n\n");
	printf("%s", buffer);

	fclose(fp); 	
}

// Our primary function: after rooms have been generated in the directory, play the game
void playGame(struct Room** roomHolder, char* directory)
{
	printf("\n");
	struct Room* currentRoom = findStartRoom(roomHolder); // start at start room
	char** pathToVictory = malloc(100000 * sizeof(char*)); // path to victory could be massive, so 100000 char strings should be sufficient
	memset(pathToVictory, '\0', sizeof(pathToVictory));

	int* pathCount; // This is used to have the correct index when printing out pathToVictory, pointer because it will be passed around and adjusted
	int pc = 0;
	pathCount = &pc;

	int* isGameOver; // this is a true/false variable that identifies when the end room is found
	int var = 0;
	isGameOver = &var;

	int* numberOfSteps; // counts the number of steps taken by the user to get to the end room
	int var3 = 0;
	numberOfSteps = &var3;

	// While the game isn't over, gather valid input and print the room, its connections
	while ((*isGameOver) == 0)
	{
		int* validInput; // will track whether or not the string the user entered is an actual connection to the current room
		int var2 = -1;
		validInput = &var2;

		while (*(validInput) == -1) // while the input is invalid, keep running this
		{
		 	printPrompt(currentRoom, directory);
			int index = getUserInput(currentRoom);
			*validInput = index;
						
			if (*validInput == -2)
			{
				createSecondThread(validInput);
				printReadTimeFile();
			}

			printf("\n\n");	
		}
	
		++(*numberOfSteps);

		checkIfGameOver(currentRoom->connections[*validInput], isGameOver); // on the basis of the above while loop, we have guaranteed validInput is a correct index

		currentRoom = currentRoom->connections[*validInput]; // make currentRoom the valid connection the user entered

		pathToVictory[*pathCount] = currentRoom->name; // add another room to the pathToVictory char string for later printing
		++(*pathCount);
	}

	printCongrats(numberOfSteps, pathToVictory, pathCount);
}

// This function reads from the files written to verify that
// my structs are accurate
void readFiles(char* directory, struct Room** rooms)
{
	int c;
	int pos = 0;

	// Open the files
	FILE* fp;
	char buff[255];
	memset(buff, '\0', sizeof(buff));

	char path[30];
	memset(path, '\0', sizeof(path));
	char fileInput[256];
	memset(fileInput, '\0', sizeof(fileInput));
	
	snprintf(path, 29, "%s/%s", directory, "room1.txt");
	fp = fopen(path, "r");

	do
	{
		c = fgetc(fp); // get a character from the file
		
		if (c != EOF) // if the character does not represent the end of the file
		{
			buff[pos] = (char)c; // buffer at array element now holds one of the chars necessary
			pos++;
		}
	} while (c != EOF && c != '\n'); // while c isn't the end of file or a new line. This function reads 1 line
	printf("%s", buff);

}

int main(void)
{
	time_t t; 
	srand((unsigned) time(&t)); // seed random numbers
	char* directory = makeDirectory();
	struct Room** holder = makeStruct();
	assignRoomNames(holder);
	FILE** files = generateRoomFiles(directory, holder);
	assignRoomTypes(holder);
	allocateRoomConnections(holder);
	fillGraph(holder);
	assignFilePointers(files, holder);
	writeToFiles(holder);
	//readFiles(directory, holder);
	playGame(holder, directory);
	printf("\n");
	return 0;
}
