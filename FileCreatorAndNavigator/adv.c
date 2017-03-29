#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

struct Room
{
	int fileDescriptor;
	int roomNumber;
	int numberOfConnections;
	int currentNumOfConnections;
	char* roomName;
	char* roomType;
	struct Room** roomConnections; // array of pointers to the room structures
};

// Make a room, append process id to the end of it
// Access the directory in the future using directoryName
char* makeDirectory()
{
	int pid = getpid();
	char firstPart[] = "hoffera.rooms.";
	char* roomDirectory = malloc(sizeof(char) * 30);
	memset(roomDirectory, '\0', sizeof(roomDirectory));
	snprintf(roomDirectory, 30, "%s%d", firstPart, pid);
	mkdir(roomDirectory, 0755);
	return roomDirectory;
}

// make 7 files corresponding to the rooms, return an array
// of filedescriptors which they can be reached at
struct Room** makeRoomFiles(char* roomDirectory)
{

	struct Room** roomHolder = malloc(7 * sizeof(struct Room));

	char room1[260];
	memset(room1, '\0', 260);
	sprintf(room1, "%s/room1", roomDirectory);
	int fileDescriptor1 = open(room1, O_RDWR | O_APPEND | O_CREAT, 0666);
	struct Room* r1 = malloc(sizeof(struct Room));
	r1->fileDescriptor = fileDescriptor1;
	r1->roomNumber = 1;
	roomHolder[0] = r1;
	
	char room2[260];
	memset(room2, '\0', 260);
	sprintf(room2, "%s/room2", roomDirectory);
	int fileDescriptor2 = open(room2, O_RDWR | O_APPEND | O_CREAT, 0666);	
	struct Room* r2 = malloc(sizeof(struct Room));
	r2->fileDescriptor = fileDescriptor2;
	r2->roomNumber = 2;
	roomHolder[1] = r2;

	char room3[260];
	memset(room3, '\0', 260);
	sprintf(room3, "%s/room3", roomDirectory);
	int fileDescriptor3 = open(room3, O_RDWR | O_APPEND | O_CREAT, 0666);
	struct Room* r3 = malloc(sizeof(struct Room));
	r3->fileDescriptor = fileDescriptor3;
	r3->roomNumber = 3;
	roomHolder[2] = r3;

	char room4[260];
	memset(room4, '\0', 260);
	sprintf(room4, "%s/room4", roomDirectory);
	int fileDescriptor4 = open(room4, O_RDWR | O_APPEND | O_CREAT, 0666);	
	struct Room* r4 = malloc(sizeof(struct Room));
	r4->fileDescriptor = fileDescriptor4;
	r4->roomNumber = 4;
	roomHolder[3] = r4;

	char room5[260];
	memset(room5, '\0', 260);
	sprintf(room5, "%s/room5", roomDirectory);
	int fileDescriptor5 = open(room5, O_RDWR | O_APPEND | O_CREAT, 0666);
	struct Room* r5 = malloc(sizeof(struct Room));
	r5->fileDescriptor = fileDescriptor5;
	r5->roomNumber = 5;
	roomHolder[4] = r5;	

	char room6[260];
	memset(room6, '\0', 260);
	sprintf(room6, "%s/room6", roomDirectory);
	int fileDescriptor6 = open(room6, O_RDWR | O_APPEND | O_CREAT, 0666);
	struct Room* r6 = malloc(sizeof(struct Room));
	r6->fileDescriptor = fileDescriptor6;
	r6->roomNumber = 6;
	roomHolder[5] = r6;
	
	char room7[260];
	memset(room7, '\0', 260);
	sprintf(room7, "%s/room7", roomDirectory);
	int fileDescriptor7 = open(room7, O_RDWR | O_APPEND | O_CREAT, 0666);
	struct Room* r7 = malloc(sizeof(struct Room));
	r7->fileDescriptor = fileDescriptor7;
	r7->roomNumber = 7;
	roomHolder[6] = r7;

	return roomHolder;
}

// get an array of strings that holds each room name in a random order
void generateRoomNames(struct Room** roomHolder)
{
	char** roomNames = malloc(10 * sizeof(char*));
	memset(roomNames, '\0', sizeof(roomNames));

	int j = 0;

	for (j; j < 10; j++)
	{
		roomNames[j] = (char*)malloc(20);
		memset(roomNames[j], '\0', sizeof(roomNames[j]));
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

	strcpy(roomNames[0], roomName1);
	strcpy(roomNames[1], roomName2);
	strcpy(roomNames[2], roomName3);
	strcpy(roomNames[3], roomName4);
	strcpy(roomNames[4], roomName5);
	strcpy(roomNames[5], roomName6);
	strcpy(roomNames[6], roomName7);
 	strcpy(roomNames[7], roomName8);
	strcpy(roomNames[8], roomName9);
	strcpy(roomNames[9], roomName10);

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
	
	for (j = 0; j < 7; j++) // roomNames is shuffled, random selection of 7 of 10 room names. assign them to roomHolder's rooms
	{
		struct Room* tempRoom = roomHolder[j];
		tempRoom->roomName = roomNames[j];
	}
}

// get an array of strings that hold each room type
char** _generateRoomTypes()
{
	char** roomTypes = malloc(3 * sizeof(char*));
	memset(roomTypes, '\0', sizeof(roomTypes));

	int j = 0;

	for (j; j < 3; j++)
	{
		roomTypes[j] = (char*)malloc(20);
		memset(roomTypes[j], '\0', sizeof(roomTypes[j]));
	}

	roomTypes[0] = "START_ROOM";
	roomTypes[1] = "END_ROOM";
	roomTypes[2] = "MID_ROOM";

	return roomTypes;
}

int _checkIfTaken(int* boolArray, int index)
{
	if (boolArray[index] == 1)
		return 1;

	else
		return 0;
} 

void assignRoomTypes(struct Room** roomHolder)
{
	char** roomTypes = _generateRoomTypes();

	int i = 0;

	int roomTypesTaken[3] = {0, 0, 0};

	for (i; i < 7; i++)
	{
		// give room file a room type
		int thisRoomType = rand() % 3;

		while (_checkIfTaken(roomTypesTaken, thisRoomType) == 1)
			thisRoomType = rand() % 3;

		if (thisRoomType == 0)
			roomTypesTaken[0] = 1;

		else if (thisRoomType == 1)
			roomTypesTaken[1] = 1;

		roomHolder[i]->roomType = roomTypes[thisRoomType];
	}
	
}
	
int IsGraphFull(struct Room** roomHolder)
{
	int i = 0;

	for (i; i < 7; i++)
	{
		struct Room* tempRoom = roomHolder[i];
		
		if (tempRoom->currentNumOfConnections < 3) // at least 1 room doesn't have appropriate num of connections
			return 0;
	}

	return 1;
}

int CanAddConnectionFrom(struct Room* room)
{
	if (room->currentNumOfConnections == 6) // the room is full
		return 0;

	else
		return 1;
}

int IsSameRoom(struct Room* room1, struct Room* room2)
{
	if (strcmp(room1->roomName, room2->roomName) == 0)
		return 1;

	else
		return 0;
}

struct Room* GetRandomRoom(struct Room** roomHolder)
{
	int index = rand() % 7;
	return roomHolder[index];
}

void ConnectRoom(struct Room* A, struct Room* B)
{
	struct Room** AConnections = A->roomConnections;
	AConnections[A->currentNumOfConnections] = B;
	A->currentNumOfConnections++;
}	

void AddRandomConnection(struct Room** roomHolder)
{
	struct Room* A;
	struct Room* B;

	while (1)
	{
		A = GetRandomRoom(roomHolder);
		if (CanAddConnectionFrom(A) == 1)
		break;
	}

	do
	{
		B = GetRandomRoom(roomHolder);
	} while (CanAddConnectionFrom(B) == 1 && IsSameRoom(A, B) == 0);

	ConnectRoom(A, B);
	ConnectRoom(B, A);
}

void initializeCurrentNums(struct Room** roomHolder)
{
	int i = 0;

	for (i; i < 7; i++)
	{
		roomHolder[i]->currentNumOfConnections = 0;
	}
}
		
void createGraph(struct Room** roomHolder)
{
	initializeCurrentNums(roomHolder);

	while (IsGraphFull(roomHolder) == 0)
	{
		AddRandomConnection(roomHolder);
	}
}	

void writeToFiles(struct Room** roomHolder)
{	
	char* roomNameString = "ROOM NAME: ";
	char* connect1 = "CONNECTION 1: ";
	char* connect2 = "CONNECTION 2: ";
	char* connect3 = "CONNECTION 3: ";
	char* connect4 = "CONNECTION 4: ";
	char* connect5 = "CONNECTION 5: ";
	char* connect6 = "CONNECTION 6: ";
	char* connectionStrings[6] = {connect1, connect2, connect3, connect4, connect5, connect6}; 
	char* roomTypeString = "ROOM TYPE: ";

	int k;

	for (k = 0; k < 7; k++)
	{
		struct Room* tempRoom = roomHolder[k];
		write(tempRoom->fileDescriptor, roomNameString, strlen(roomNameString));
		write(tempRoom->fileDescriptor, tempRoom->roomName, strlen(tempRoom->roomName));
		write(tempRoom->fileDescriptor, "\n", 1);
	
		int counter = 0;
	
		for (counter; counter < tempRoom->currentNumOfConnections; counter++)
		{
			struct Room** connections = tempRoom->roomConnections;
			struct Room* thisConnection = connections[counter];
			write(tempRoom->fileDescriptor, connectionStrings[counter], strlen(connectionStrings[counter]));
			write(tempRoom->fileDescriptor, thisConnection->roomName, strlen(thisConnection->roomName));
			write(tempRoom->fileDescriptor, "\n", 1);
		}

		write(tempRoom->fileDescriptor, roomTypeString, strlen(roomTypeString));
		write(tempRoom->fileDescriptor, tempRoom->roomType, strlen(tempRoom->roomType));
	}
}		

/*	write(fileDescriptors[0], roomNameString, strlen(roomNameString));
	write(fileDescriptors[0], roomNames[0], strlen(roomNames[0]));
	write(fileDescriptors[0], "\n", 1);
	write(fileDescriptors[0], connectionStrings[0], strlen(connectionStrings[0]));
	write(fileDescriptors[0], roomNames[1], strlen(roomNames[1]));
	write(fileDescriptors[0], "\n", 1);
	write(fileDescriptors[0], roomTypeString, strlen(roomTypeString));
	write(fileDescriptors[0], roomTypes[0], strlen(roomTypes[0]));
*/

// returns start room
struct Room* findStartRoom(struct Room** roomHolder)
{
	int i = 0;
	char* startRoomString = "START_ROOM";	

	for (i; i < 7; i++)
	{
		struct Room* tempRoom = roomHolder[i];

		int comparison = strcmp(tempRoom->roomType, startRoomString);

		if (comparison == 0)
			return tempRoom;
	}
}

void printCurLocationAndConnections(struct Room* r1)
{
	printf("CURRENT LOCATION: ");
	char* genericRoomNameString = "ROOM NAME: ";
	int genericRoomNameLength = strlen(genericRoomNameString);
	lseek(r1->fileDescriptor, 0, SEEK_SET);
	lseek(r1->fileDescriptor, genericRoomNameLength, SEEK_CUR);
	int actualRoomNameLength = strlen(r1->roomName);
	char buffer[actualRoomNameLength];
	memset(buffer, '\0', sizeof(buffer));
	read(r1->fileDescriptor, buffer, actualRoomNameLength);
	printf("%s", buffer);
	printf("\n");

	printf("POSSIBLE CONNECTIONS: ");

	int i = 0;

	for (i; i < r1->numberOfConnections; i++)
	{
		char* genericConnectionString = "CONNECTION 1: ";
		int genericConnectionLength = strlen(genericConnectionString);
		lseek(r1->fileDescriptor, genericConnectionLength, SEEK_CUR);
		int actualConnectionNameLength = strlen(r1->roomConnections[0]->roomName);
		actualConnectionNameLength++;
		char buffer2[actualConnectionNameLength];
		memset(buffer2, '\0', sizeof(buffer2));
		read(r1->fileDescriptor, buffer2, actualConnectionNameLength);
		printf("%s", buffer2);
		printf(", ");
	}

	printf("\n");
	
}

void printWhereTo()
{
	printf("WHERE TO? >");
}

void printPrompt(struct Room* r1)
{
	printCurLocationAndConnections(r1);
	printWhereTo();
}

void playGame(struct Room** roomHolder)
{
	int IsGameOver = 0;

	struct Room* start_room = findStartRoom(roomHolder);
	printPrompt(start_room);	
}

int main()
{
	time_t t; 
	srand((unsigned) time(&t)); // seed random numbers
	char* roomDirectory= makeDirectory(); // holds directory name
	struct Room** r1 = makeRoomFiles(roomDirectory);	
	generateRoomNames(r1);
	assignRoomTypes(r1);
	createGraph(r1);
//	writeToFiles(r1);
	//playGame(r1);
	return 0;
}
