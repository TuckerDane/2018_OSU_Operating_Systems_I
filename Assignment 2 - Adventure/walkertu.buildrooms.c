#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

#define MAX_CONNECTIONS 6
#define MIN_CONNECTIONS 3
#define NUMNAMES 10
#define NUMROOMS 7
#define TRUE 1
#define FALSE 0
#define START_ROOM 0
#define END_ROOM 1
#define MID_ROOM 2

/*********************************************
//	GLOBALS
**********************************************/

// hardcoded room name arrays
char* roomColors[NUMNAMES] = {		"Red ", "Yellow ", "Blue ", "Orange ", "Purple ",
					"Green ", "Black ", "White ", "Grey ", "Rainbow " };
char* roomTypes[NUMNAMES] = {		"Ballroom", "Kitchen", "Hallway", "Mezanine", "Diningroom",
					"Chamber", "Throneroom", "Cellar", "Basement", "Attic" };
char* roomDescriptions[NUMNAMES] = {	"Secrets", "Death", "Tranquility", "Bones", "Spiders",
					"Magic", "Listlessness", "Fighting", "the Elves", "Paradise" };
char* types[3] = {"START_ROOM ", "END_ROOM ", "MID_ROOM "};

/*********************************************
//	DUNGEON STRUCTURE
**********************************************/

// structure of a room
//................................
struct Room {
	int name[3];				// name of the room {color, type, description}
	int type;				// room type { 1 == START_ROOM, 2 == END_ROOM, 3 == MID_ROOM}
	int numConnections;			// total number of connections for the room
	int connections[MAX_CONNECTIONS];	// connections to other rooms { -1 == NULL, 0-6 == Respective rooms }
};

/*********************************************
//
//	FUNCTIONS
//
// **********************************************/

//***********************************************************************************************************
//
//	assignNames	
//
//***********************************************************************************************************
void assignNames(struct Room *dun, int numRooms)
{
	int i;
	for(i = 0; i < numRooms; i++)
	{
		int nameTaken = TRUE;
		int random;
		
		// while the room name assigned is taken...
		while (nameTaken == TRUE)
		{	
			// set nameTaken to False
			nameTaken = FALSE;
			
			// iterate through and assing random color/types/descriptions for the room
			int j = 0;
			for ( j = 0; j<3; j++ )
			{
				random = rand() % 10;
				dun[i].name[j] = random;
			}

			// compare the room name to other room names in the array
			int k = 0;
			for ( k = 0; k<i; k++)
			{		
				// if the name is already taken, set the nameTaken flag to true and re-iterate
				if (dun[i].name[0] == dun[k].name[0])
				{
					nameTaken = TRUE;
				}
			}
		}
	}
}

//***********************************************************************************************************
//
//	assignTypes	
//
//***********************************************************************************************************
void assignTypes(struct Room *dun, int numRooms)
{
	int i;
	for ( i=0; i<numRooms; i++)
	{
		dun[i].type = MID_ROOM;
	}
	dun[0].type = START_ROOM;
	dun[numRooms-1].type = END_ROOM;
}

//***********************************************************************************************************
//
//	isSameRoom		
//
//***********************************************************************************************************
int isSameRoom(int room1, int room2)
{
	return (room1 == room2);
}

//***********************************************************************************************************
//
//	connectRooms		
//
//***********************************************************************************************************
void connectRoom(struct Room* dun, int room1, int room2)
{
	dun[room1].connections[dun[room1].numConnections] = room2;
	dun[room1].numConnections++;
}

//***********************************************************************************************************
//
//	connectionExists		
//
//***********************************************************************************************************
int connectionExists(struct Room* dun, int room1, int room2)
{
	int i;
	for( i=0; i<dun[room1].numConnections; i++)
	{
		if (dun[room1].connections[i] == room2)
		{
			return TRUE;
		}
	}
	return FALSE;
}

//***********************************************************************************************************
//
//	canAddConnectionFrom	
//
//***********************************************************************************************************
int canAddConnectionFrom(struct Room* dun, int room)
{
	// returns true if the room in the dungeon has less than the MAX connections
	return (dun[room].numConnections < MAX_CONNECTIONS);
}

//***********************************************************************************************************
//
//	getRandomRoom	
//
//***********************************************************************************************************
int getRandomRoom(int numRooms)
{
	return (rand() % numRooms);
}

//***********************************************************************************************************
//
//	addRandomConnection		
//
//***********************************************************************************************************
void addRandomConnection(struct Room* dun, int numRooms)
{
	int room1;
	int room2;

	while(TRUE)
	{
		room1 = getRandomRoom(numRooms);

		if (canAddConnectionFrom(dun, room1) == TRUE)
		break;
	}

	do
	{
		room2 = getRandomRoom(numRooms);
	}
	while(canAddConnectionFrom(dun, room2) == FALSE || isSameRoom(room1, room2) == TRUE || connectionExists(dun, room1, room2) == TRUE);

	connectRoom(dun, room1, room2);
	connectRoom(dun, room2, room1);
}

//***********************************************************************************************************
//
//	dungeonFull		
//
//***********************************************************************************************************
int dungeonFull(struct Room *dun, int numRooms)
{
	// if any room has less than 3 connections, the dungeon is not full; return false
	// otherwise, return true	
	int i;
	int isFull = TRUE;
	for ( i=0; i<numRooms; i++)
	{
		if (dun[i].numConnections < 3)
		{
			isFull = FALSE;
			break;
		}
	}

	// if the dungeon is not full, return false
	return isFull;
}

//***********************************************************************************************************
//
//	connectDungeon
//
//***********************************************************************************************************
void connectDungeon(struct Room *dun, int numRooms)
{
	int i;

	// initialize connection arrays
	for ( i=0; i<numRooms; i++ )
	{
		int j;
		for ( j=0; j<MAX_CONNECTIONS; j++)
		{
			dun[i].connections[j] = -1;	// set connection value to null
		}
	}

	// connect rooms
	while(!dungeonFull(dun, numRooms))
	{
		addRandomConnection(dun, numRooms);
	}
}

//***********************************************************************************************************
//
//	makeDungeon	
//
//***********************************************************************************************************
struct Room* makeDungeon(int numRooms)
{
	// allocate space for a dungeon consisting of numRooms
	struct Room* newDungeon = malloc(sizeof(struct Room) * numRooms);
	assert(newDungeon != 0);

	// assign namess
	assignNames(newDungeon, numRooms);

	// assign roomtypes
	assignTypes(newDungeon, numRooms);	

	// assign connections between all rooms
	connectDungeon(newDungeon, numRooms);	

	// return newDungeon back to the caller
	return newDungeon;
}

//***********************************************************************************************************
//
//	printDungeon
//
//***********************************************************************************************************
printDungeon(struct Room* dun, int numRooms)
{
	printf("NO\tNUMC\tTYPE\t\tNAME\n");
	printf("--\t----\t----\t\t----\n");
	int i=0;
	for ( i = 0; i < numRooms; i++)
	{	
		printf("%d\t", i);					// number
		printf("%d\t", dun[i].numConnections);			// number of connections
		printf("%s\t", types[dun[i].type]);			// type
		printf("%s\n", roomColors[dun[i].name[0]]);		// color
	//	printf(" %s of ", roomTypes[dun[i].name[1]]);		// type
	//	printf("%s\n", roomDescriptions[dun[i].name[2]]);	// descriptor
		printf("\n");	
		// connections
		int j;
		for ( j=0; j<dun[i].numConnections; j++)
		{
			printf("\t\tConnection %d:\t", j+1);
			printf("%s\n", roomColors[dun[dun[i].connections[j]].name[0]]);		// color
	//		printf(" %s of ", roomTypes[dun[dun[i].connections[j]].name[1]]);	// type
	//		printf("%s\n", roomDescriptions[dun[dun[i].connections[j]].name[2]]);	// descriptor
		}
		
		printf("\n");
	}
}

//***********************************************************************************************************
//
//	writeRoomToFile
//
//***********************************************************************************************************
void writeRoomToFile(struct Room* dun, int numRooms, char* dirName)
{
	// write room to a file
	FILE * fp;

	char filePath[30];
	char fileName[] = "/room";

	int i;
	for ( i = 0; i < numRooms; i++)
	{	
		sprintf(filePath, "%s%s%d", dirName, fileName, i + 1);

		// open file
		fp = fopen(filePath, "w");

		// name
		fprintf(fp, "ROOM NAME: %s\n", roomColors[dun[i].name[0]]);
		
		// connections
		int j;
		for ( j=0; j<dun[i].numConnections; j++)
		{
			fprintf(fp, "CONNECTION %d: ", j+1);
			fprintf(fp, "%s\n", roomColors[dun[dun[i].connections[j]].name[0]]);
		}
		
		// type
		fprintf(fp, "ROOM TYPE: %s\t", types[dun[i].type]);
		fprintf(fp, "\n");
	}

	// close the file
	fclose(fp);
}

//***********************************************************************************************************
//
//	writeDungeonToFile
//
//	some code borrowed from: https://cboard.cprogramming.com/c-programming/165757-using-process-id-name-file-directory.html
//
//***********************************************************************************************************
void writeDungeonToFolder(struct Room* dun, int numRooms)
{
	// write dungeon to a folder with process id
	int pid = getpid();
	char prefix[] = "walkertu.rooms.";
	char dirName[20];
	sprintf(dirName, "%s%d", prefix, pid);
	mkdir(dirName, 0755);
	
	// write rooms to file
	writeRoomToFile(dun, numRooms, dirName);
}

//***********************************************************************************************************
//
//	main
//
//***********************************************************************************************************
void main()
{
	// seed random number generator
	srand(time(0));

	// create dungeon
	struct Room* dungeon = makeDungeon(NUMROOMS);
	assert(dungeon !=0);
	
	// print dungeon
	// printDungeon(dungeon, NUMROOMS);

	// make directory for dungeon
	writeDungeonToFolder(dungeon, NUMROOMS);

	// garbage collection
	free(dungeon);
}
