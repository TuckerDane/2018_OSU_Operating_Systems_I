/***********************************************************************************************************
//
//	[x] Presents an interface to the player
//		[x] lists where the player currently is
//		[x] lists possible connections that can be followed
//		[x] prompts the user: WHERE TO? >
//			[x] cursor is placed just after the >
//			[x] punctuation is correct
//		[x] When the user types in the exact name of a connection and hits return, program writes a new line and then continues running as before.
//		[x] if the user types any invalid input...
//			[x] there is an error line that says "HUH? I DON't UNDERSTAND THAT ROOM. TRY AGAIN"
//			[x] the program repeats the current location and prompt
//		[x] trying to go to an incorrect location does not increment the path history or step count
//		[x] once the user reaches the end room...
//			[x] the game indicates that it has been reached
//			[x] it prints out the path the user took to get there
//			[x] it prints the number of steps taken (not the number of rooms visited)
//			[x] a congratulatory message is printed
//			[x] program exists with a status code of 0
//	[x] Room data is read back into the program from the room files
//	[x] Game uses the most recently created room files
//		[x] performs a stat() function call on rooms directories and opens the one with the most recent st+mtime component of the returned stat struct
//	
//	[x] TIME KEEPING
//		[x] game returns current time of day
//			[x] uses a second thread
//			[x] uses mutex(es)
//			[x] requires use of -lpthread on compilation with gcc
//			[x] when player types command "time"
//			[x] writes to a file "currentTime.txt"
//				[x] program reads value from file
//				[x] file is not deleted
//				[x] file is located in same directory as the game
//
/ *********************************************************************************************************/

#include <fcntl.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

/**********************************************************
// DEFINITIONS
// ********************************************************/
#define MAX_CONNECTIONS 6
#define MIN_CONNECTIONS 3
#define NUMNAMES 10
#define NUMROOMS 7
#define TRUE 1
#define FALSE 0
#define START_ROOM 0
#define END_ROOM 1
#define MID_ROOM 2

/**********************************************************
// STRUCTS
// ********************************************************/

// Player Structure
//..........................................................
struct Player {
	int locationIndex;				// the index of the room's name for the current player's location in the Dungeon
	int numSteps;					// the number of steps the player has taken
	int path[100];					// the iindeces of the room's name for each step the player has taken along his path, up to 100 steps
};

// Room Structure
//..........................................................
struct Room {
	int nameIndex;					// the index of the room's name in the Dungeon
	int type;	 					// room type 0=START_ROOM, 1=END_ROOM, 2=MID_ROOM
	int numConnections;				// total number of connections for the room
	int connections[6];				// the index of the room names for each connection in the room
};

// Dungeon Structure
//..........................................................
struct Dungeon {
	char roomNames[NUMROOMS][10];	// an array of strings containing the names of the rooms
	struct Room rooms[NUMROOMS];	// rooms inside of the dungeon
	struct Player player;			// the player inside of the dungeon
};

//*********************************************************
// INITIALIZATIONS
// ********************************************************/

// makePlayer
//
// description: creates a player structure that has taken
// 				no steps, whose path is set to null, and
// 				whose location is the start room.
// 
// @return:	"p" - a player structure
//
//..........................................................
struct Player makePlayer(){

	struct Player p;						// define a player

	p.locationIndex = -1;					// set player location to null
	p.numSteps = 0;							// set number of steps is 0

	int i;									// set all steps in the path array to null
	for (i=0; i<(sizeof(p.path)/sizeof(int)); i++)
	{
		p.path[i] = -1;
	}
	return p;								// return initialized player	
};

// makeRoom
//
// description: initializes a room structure and returns it
// 
// @return:	"room" - the initialized room
// 
//..........................................................
struct Room makeRoom()
{
	struct Room room;				// define a room

	room.nameIndex = -1;			// set the room name to null
	room.type = -1;					// set the room type to null
	room.numConnections = 0;		// give the room no connections
	int i;
	for (i=0; i<NUMROOMS; i++)		// set all possible connections to null
	{
		room.connections[i] = -1;
	}
	
	return room;					// return the room
}

// makeDungeon
//
// description: initializes a dungeon with 7 rooms and
// 		returns it
//
// @return:	"dungeon" - the initialized dungeon
// 
//..........................................................
struct Dungeon makeDungeon()
{
	struct Dungeon dungeon;				// define a dungeon

	int i;
	for(i=0; i<NUMROOMS; i++)			// initialize empty room names
	{
		memset(dungeon.roomNames[i], '\0', (sizeof(dungeon.roomNames[i]) / sizeof(char)));
	}

	for(i=0; i<NUMROOMS; i++)			// define blank rooms within the dungeon
	{
		dungeon.rooms[i]=makeRoom();
	}

	dungeon.player=makePlayer();		// define a blank player within the dungeon

	return dungeon;						// return the dungeon
}

/*********************************************************
// GET FUNCTIONS
// ********************************************************/

// GET PLAYER ATTRIBUTES
//..........................................................

// getPlayerLocationIndex
int getPlayerLocationIndex(struct Player* player)
{
	return player->locationIndex;
}

// getPlayerNumSteps
int getPlayerNumSteps(struct Player* player)
{
	return player->numSteps;
}

// getPlayerPath
int getPlayerPath(struct Player* player, int index)
{
	return player->path[index];
}

// GET ROOM ATTRIBUTES
//..........................................................

// getRoomNameIndex
int getRoomNameIndex(struct Room* room)
{
	return room->nameIndex;
}

// getRoomType
int getRoomType(struct Room* room)
{
	return room->type;
}

// getNumConnections
int getNumConnections(struct Room* room)
{
	return room->numConnections;
}

// getRoomConnections
int getConnections(struct Room* room, int index)
{
	return room->connections[index];
}

// GET DUNGEON ATTRIBUTES
//..........................................................

// getRoomName
char* getRoomName(struct Dungeon* dungeon, int index)
{
	return dungeon->roomNames[index];
}

// getRoom
struct Room* getRoom(struct Dungeon* dungeon, int index)
{
	struct Room* roomPtr = &dungeon->rooms[index];
	return roomPtr;
}

// getPlayer
struct Player* getPlayer(struct Dungeon* dungeon)
{
	struct Player* playerPtr = &dungeon->player;
	return playerPtr;
}

/*********************************************************
// SET FUNCTIONS
// ********************************************************/

// SET PLAYER ATTRIBUTES
//..........................................................

// setPlayerLocationIndex
void setPlayerLocationIndex(struct Player* player, int locationIndex)
{
	player->locationIndex = locationIndex;
}

// setPlayerNumSteps
void setPlayerNumSteps(struct Player* player, int steps)
{
	player->numSteps = steps;
}

// setPlayerPath
void setPlayerPath(struct Player* player, int index, int roomNameIndex)
{
	player->path[index] = roomNameIndex;
}

// SET ROOM ATTRIBUTES
//..........................................................

// setRoomNameIndex
void setRoomNameIndex(struct Room* room, int nameIndex)
{
	room->nameIndex = nameIndex;
}

// setRoomType
void setRoomType(struct Room* room, int type)
{
	room->type = type;
}

// setNumConnections
void setNumConnections(struct Room* room, int numCon)
{
	room->numConnections = numCon;
}

// setRoomConnection
void setConnection(struct Room* room, int conIndex, int nameIndex)
{
	room->connections[conIndex] = nameIndex;
}

// SET DUNGEON ATTRIBUTES
//..........................................................

// setRoomName
void setRoomName(struct Dungeon* dungeon, int index, char* name)
{
	strcpy(dungeon->roomNames[index], name);
}

// setRoom
void setRoom(struct Dungeon* dungeon, int index, int nIndex, int type, int numCon)
{	
	setRoomNameIndex(getRoom(dungeon, index), nIndex);	// set the room's name index
	setRoomType(getRoom(dungeon, index), type);			// set the room's type
	setNumConnections(getRoom(dungeon, index), numCon);	// set the room's number of connections

	int i;
	for(i=0; i<numCon; i++)
	{
		setConnection(getRoom(dungeon, index), i, -2);		// set the connections of the room
	}

}

// setPlayer
void setPlayer(struct Dungeon* dungeon, int plIndex, int nSteps)
{
	setPlayerLocationIndex(getPlayer(dungeon), plIndex);
	setPlayerNumSteps(getPlayer(dungeon), nSteps);

	int i;
	for(i=0; i<nSteps; i++)
	{
		setPlayerPath(getPlayer(dungeon), i, -3);
	}
}

/*********************************************************
// PRINT FUNCTIONS
// ********************************************************/

// print player
void printPlayer(struct Player* player)
{
	printf("PLAYER LOCATION INDEX: %d\n", getPlayerLocationIndex(player));	// print the index of the player's location
	printf("PLAYER NUMSTEPS: %d\n", getPlayerNumSteps(player));				// print the number of steps the player has taken
	printf("PLAYER PATH: ");												// print, sequentially, the indeces of the player's path

	int i;
	for(i=0; i<100; i++)
	{
		printf("%d ", getPlayerPath(player, i));
	}
	printf("\n");
}

// print room
void printRoom(struct Dungeon* dungeon, struct Room* room)
{
	printf("ROOM NAME INDEX:\t%d\n", getRoomNameIndex(room));		// print the index of the room's name

	printf("ROOM TYPE:\t\t");										// print the room's type
	if(getRoomType(room) == 0)
	{
		printf("START_ROOM");
	}
	else if(getRoomType(room) == 1)
	{
		printf("END_ROOM");
	}
	else if(getRoomType(room) == 2)
	{
		printf("MID_ROOM");
	}
	printf("\n\n");

	printf("NUM CONNECTIONS:\t%d\n", getNumConnections(room));		// print the room's number of connections

	int i;
	for(i=0; i<6; i++)												// print the the indeces listed for connections in the room's connections array
	{
		printf("CONNECTION %d:\t\t%s\n", i+1, getRoomName(dungeon, getConnections(room, i)));
	}
}

// print dungeon
void printDungeon(struct Dungeon* dungeon)
{
	printf("+---------+\n");
	printf("+ DUNGEON +\n");
	printf("+---------+\n\n");

	printf("PLAYER\n");
	printf("------\n\n");

	// print the player attributes
	printPlayer(getPlayer(dungeon));
	printf("\n");
	printf("NAMES\n");
	printf("-----\n\n");

	// print out the names of the rooms in the dungeon
	int i;
	for (i=0; i<NUMROOMS; i++)
	{
		printf("ROOM %d: ", i+1);
		printf("%s\n", getRoomName(dungeon, i));
	}
	printf("\n");

	// print out the room attributes
	printf("ROOMS\n");
	printf("-----\n\n");
	for(i=0; i<NUMROOMS; i++)
	{
		printf("ROOM NAME:\t\t%s\n", getRoomName(dungeon, i));
		printRoom(dungeon, getRoom(dungeon, i));
		printf("\n");
	}
	printf("\n");
}

/*********************************************************
// FILE INPUT/OUTPUT FUNCTIONS
// ********************************************************/

// FILE OUTPUT DUNGEON ATTRIBUTES
//..........................................................

// getRecentDungeon (code ripped straight from Benjamin Brewster: Block II Reading Section 2.4 - Manipulating Directories)
char* getRecentDungeon()
{
	int newestDirTime = -1; // Modified timestamp of newest subdir examined
  	char targetDirPrefix[32] = "walkertu.rooms."; // Prefix we're looking for
  	char newestDirName[256]; // Holds the name of the newest dir that contains prefix
  	memset(newestDirName, '\0', sizeof(newestDirName));

 	DIR* dirToCheck; // Holds the directory we're starting in
  	struct dirent *fileInDir; // Holds the current subdir of the starting dir
  	struct stat dirAttributes; // Holds information we've gained about subdir

  	dirToCheck = opendir("."); // Open up the directory this program was run in

	  if (dirToCheck > 0) // Make sure the current directory could be opened
	  {
	    while ((fileInDir = readdir(dirToCheck)) != NULL) // Check each entry in dir
	    {
	      if (strstr(fileInDir->d_name, targetDirPrefix) != NULL) // If entry has prefix
	      {

	        stat(fileInDir->d_name, &dirAttributes); // Get attributes of the entry

	        if ((int)dirAttributes.st_mtime > newestDirTime) // If this time is bigger
	        {
	          	newestDirTime = (int)dirAttributes.st_mtime;
	          	memset(newestDirName, '\0', sizeof(newestDirName));
	          	strcpy(newestDirName, fileInDir->d_name);
	        }
	      }
	    }
	  }

	closedir(dirToCheck); // Close the directory we opened

    char *dungeonName = malloc(256 * sizeof(char));	// allocate memory so that a character array can be returned
    memset(dungeonName, '\0', sizeof(dungeonName)); // set the values to null so that there are no weird errors
    strcpy(dungeonName, newestDirName);				// put the most recent directory name into dungeonName
    return dungeonName;								// return the directory path
}

// getRoomNamePath
char* getRoomPath(int roomNum)
{
	// found out how to do much of this here: https://tinyurl.com/ycf8kov9
	char roomPath[256];

	char* directory = getRecentDungeon();	// the most recent directory of rooms
	char filePrefix[] = "/room";			// prefix for each room
	int fileNo = roomNum;					// the room number

	snprintf(roomPath, sizeof roomPath, "%s%s%d", directory, filePrefix, fileNo);	// combine the filepath, room prefix, and number into one filepath

    char *path = malloc(256 * sizeof(char));	// allocate memory so that a character array can be returned
    memset(path, '\0', sizeof(path));			// set the values to null so that there are no weird errors
    strcpy(path, roomPath);						// put the room path into the path string
    free(directory);							// free the memory associated with the most recent directory path
    return path;								// return the filepath for the room given
}	

// loadRoomNames
void loadRoomNames(struct Dungeon* dungeon, char* roomPath, int roomNum)
{
	// much of this taken from the lecture 2.4 File Access in C
	int file_descriptor;
	char readBuffer[200];

	file_descriptor = open(roomPath, O_RDONLY);

	if (file_descriptor < 0)
	{
		fprintf(stderr, "Could not open %s\n", roomPath);
		exit(1);
	}

	memset(readBuffer, '\0', sizeof(readBuffer)); 				// clear out the array before use
	lseek(file_descriptor, 0, SEEK_SET); 						// reset the file pointer to the beginning of the file
	read(file_descriptor, readBuffer, sizeof(readBuffer)-1 );
	
																// used https://www.tutorialspoint.com/c_standard_library/c_function_strtok.htm
	const char s[2] = "\n";										// newline
	const char a[2] = " ";										// room name
	char *token;

	// get the first token
	token = strtok(readBuffer, s);	// First string in room file (ROOM NAME: ?)
	token = strtok(token, a);		// ROOM
	token = strtok(NULL, a);		// NAME
	token = strtok(NULL, a);		// ?

	strcpy(dungeon->roomNames[roomNum], token);				// set the name of the room
	setRoomNameIndex(getRoom(dungeon, roomNum), roomNum);	// set the name index of the room

	//	printf("File Contents:\n%s\n", readBuffer);
	close(file_descriptor);
}

// FILE OUTPUT ROOM ATTRIBUTES
//..........................................................

// loadRoomFile
void loadRoomAttributes(struct Dungeon* dungeon, char* roomPath, int rNum)
{
	// much of this taken from the lecture 2.4 File Access in C
	int file_descriptor;
	char readBuffer[200];

	file_descriptor = open(roomPath, O_RDONLY);

	if (file_descriptor < 0)
	{
		fprintf(stderr, "Could not open %s\n", roomPath);
		exit(1);
	}

	memset(readBuffer, '\0', sizeof(readBuffer)); 				// clear out the array before use
	lseek(file_descriptor, 0, SEEK_SET); 						// reset the file pointer to the beginning of the file
	read(file_descriptor, readBuffer, sizeof(readBuffer)-1 );
	
																// used https://www.tutorialspoint.com/c_standard_library/c_function_strtok.htm
	const char s[2] = "\n";										// newline
	const char a[2] = " ";										// room name
	char *token;

	// get the first token

	// First line in room file (ROOM NAME: ?)
	token = strtok(readBuffer, a);	// ROOM
	token = strtok(NULL, a);		// NAME
	token = strtok(NULL, a);		// ?

	// Second Line and Beyond
	while (token !=NULL) 
	{
		token = strtok(NULL, a);
		token = strtok(NULL, a);
		token = strtok(NULL, a);

		// if the token is null...
		if (token == NULL)
		{
			// do nothing
		}
		// check to see if the token refers to a room type and set it
		else if (strcmp(token, "START_ROOM") == 0)
		{
			setRoomType(getRoom(dungeon, rNum), START_ROOM);	// 0
		}
		else if (strcmp(token, "END_ROOM") == 0)
		{
			setRoomType(getRoom(dungeon, rNum), END_ROOM);		// 1
		}
		else if (strcmp(token, "MID_ROOM") == 0)
		{
			setRoomType(getRoom(dungeon, rNum), MID_ROOM);		// 2
		}
		// else the token refers to the room name of a connection - set it
		else
		{
			int i;
			for (i=0; i<NUMROOMS; i++)
			{
				// if the token is one of the rooms in the dungeon
				if (strcmp(token, getRoomName(dungeon, i)) == 0)
				{

					// void setConnection(struct Room* room, int conIndex, int nameIndex)
					// set the connection's index number to the room name index in dungeon
					setConnection(getRoom(dungeon, rNum), getNumConnections(getRoom(dungeon, rNum)), i);
					
					// increment connections
					setNumConnections(getRoom(dungeon, rNum), getNumConnections(getRoom(dungeon, rNum))+1);		
				}
			}

		}

	}

	//	printf("File Contents:\n%s\n", readBuffer);
	close(file_descriptor);
}

// FILE OUTPUT TIME FUNCTIONS
//..........................................................
void printTime(char* timePath)
{
	// much of this taken from the lecture 2.4 File Access in C
	int file_descriptor;
	char readBuffer[200];

	file_descriptor = open(timePath, O_RDONLY);

	if (file_descriptor < 0)
	{
		fprintf(stderr, "\nCould not open %s\n", timePath);
		exit(1);
	}

	memset(readBuffer, '\0', sizeof(readBuffer)); 				// clear out the array before use
	lseek(file_descriptor, 0, SEEK_SET); 						// reset the file pointer to the beginning of the file
	read(file_descriptor, readBuffer, sizeof(readBuffer)-1 );	// read into the buffer
	
	printf("\n%s\n", readBuffer);									// print the contents of the buffer

	//	printf("File Contents:\n%s\n", readBuffer);
	close(file_descriptor);
}

// FILE INPUT TIME FUNCTIONS
//..........................................................
void* makeTimeFile(void* argument)
{
	pthread_mutex_t mutex;
	mutex = *((pthread_mutex_t *) argument);

	// update the time file every 60 seconds with the new time
	while (1==1)
	{
		// lock mutex using timeKeeper thread
		pthread_mutex_lock(&mutex);

		FILE *fp;
		fp=fopen("currentTime.txt", "w");

		// taken from https://linux.die.net/man/3/strftime
	    char outstr[200];
	    time_t t;
	    struct tm *tmp;

	   	t = time(NULL);
	    tmp = localtime(&t);
	    if (tmp == NULL) {
	        perror("localtime");
	        exit(EXIT_FAILURE);
	    }

	   if (strftime(outstr, sizeof(outstr), "%I:%M%P, %A, %B %d, %Y", tmp) == 0) {
	        fprintf(stderr, "strftime returned 0");
	        exit(EXIT_FAILURE);
	    }

	    fprintf(fp, outstr);

		fclose(fp);

		// unlock mutex using timeKeeper thread
		pthread_mutex_unlock(&mutex);

		// wait 60 seconds before next update
		sleep(60);
	}
}

/*********************************************************
// GAME FUNCTIONS
// ********************************************************/

// INPUT / OUTPUT
//..........................................................

// validateInput
//
// description: validates input provided by the user and 
// 	 
// @param:	"input" - the user's input to be validated
// @return:	"inputValid" a true or false flag
// 			true == valid input
// 			false == invalid input
//..........................................................
int validateInput(struct Dungeon* dungeon, struct Player* player, char* input)
{
	// "time" is a valid input
	if(strcmp(input, "time") == 0)
	{
		return TRUE;
	}

	// if any input is valid for the current room, return true
	int playerIndex = getPlayerLocationIndex(player);
	int numConnections = getNumConnections(getRoom(dungeon, playerIndex));
	int i;
	for (i=0; i<numConnections; i++)
	{
		if (strcmp(input, getRoomName(dungeon, getConnections(getRoom(dungeon, getPlayerLocationIndex(player)), i))) == 0)
		{
			return TRUE;
		}
	}
	return FALSE;	// otherwise, return false
}

// errorMessage 
//
// description: prints the standard error message in the
// 		casee that input is invalid. 
// 	 
//..........................................................
void errorMessage()
{
	printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN\n");
}

// updatePlayer
//
// description:	updates the player as to their current
// 		location, possible connections, and prompts
// 		the player to enter a connection to follow
// 		next. If the player has found the end room,
// 		congratulates the player and gives the
// 		player their number of steps and path
//
// @param:	"p" - the player structure
// 
//..........................................................
void updatePlayer(struct Dungeon* dungeon, struct Player* p)
{
	if(getPlayerLocationIndex(p) != 6)					// if the player is not in the final room, update the player
	{
		int playerIndex = getPlayerLocationIndex(p);

		printf("\nCURRENT LOCATION: %s\n", getRoomName(dungeon, playerIndex));		// by printing the name of their current location
		printf("POSSIBLE CONNECTIONS: ");											// and listing possible connections

		int numConnections = getNumConnections(getRoom(dungeon, playerIndex));

		int i;
		for (i=0; i<numConnections; i++)				// for each room connection...
		{
			printf("%s", getRoomName(dungeon, getConnections(getRoom(dungeon, getPlayerLocationIndex(p)), i)));		// print the name of the room

			if(i < numConnections-1)					// and if there are more to follow...
			{
				printf(", ");							// add a comma
			}
			else										// else there are none left to follow...
			{
				printf(".\n");							// so finish with a period and a newline
			}
		}

		printf("WHERE TO? >");							// then ask the player for the new where to prompt
	}
	else												// otherwise the player has made it to the final room...
	{
		printf("\nYOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");				// congratulate the player
		printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", p->numSteps);	// and print out the player's path
		
		int i;
		for ( i=0; i<getPlayerNumSteps(p); i++)
		{
			printf("%s\n", getRoomName(dungeon, p->path[i]));
		}
		printf("\n");
	}
}

// processInput
//
// description:	receives input from the player, validates
// 		the player's input, and returns an updated
// 		player structure.
//
// @param:	"p" - the player structure
// @return:	"p" - the update player structure
//
//..........................................................
void processInput(struct Dungeon* dungeon, struct Player* p, pthread_mutex_t* mutex)
{
	// receive input from player
	char inp[10] = "\0\0\0\0\0\0\0\0\0\0";
	scanf("%s", &inp);

	char* input = inp;

	// validate input
	if(validateInput(dungeon, p, input))				// if input is valid...
	{	
		if (strcmp(input, "time") == 0)					// if the player wants the time, make the time and print the time
		{
			// lock mutex using mainThread
			pthread_mutex_lock(mutex);

			// printTime using mainThread
			printTime("currentTime.txt");				// load and print the time

			// unlock mutex using mainThread
			pthread_mutex_unlock(mutex);
		}
		else											// otherwise if the player wants to go to a room
		{
			// update the player's current room to the input
			//...............................................

			// find the index of the room in Dungeon
			int roomIndex;

			// interate through each dungeon room name, incrementing roomIndex until a match is found
			for (roomIndex=0; roomIndex<NUMROOMS; roomIndex++)
			{
				if(strcmp(input, getRoomName(dungeon, roomIndex)) == 0)	
				{
					break;
				}
			}

			// update player incormation
			setPlayerLocationIndex(p, roomIndex);		// update location uding the roomIndex found

			p->path[p->numSteps]=roomIndex;				// update path array
			p->numSteps++;								// update number of steps
		}
	}
	else											// else invalid input...
	{
		errorMessage();								// provide error message
	}
}

// gameLoop
//
// description: continuously lists the player's location,
// 		connections, and processes the player's
// 		input until the player decides to exit.
//
//..........................................................
void gameLoop(struct Dungeon* dungeon, pthread_mutex_t* mutex)
{
	struct Player* player = getPlayer(dungeon);

	do{
		// list player's location and possible connections
		updatePlayer(dungeon, player);

		// process player input
		processInput(dungeon, player, mutex);	
	
	} while(getPlayerLocationIndex(player) != 6);	// while the player is not in the END_ROOM

	updatePlayer(dungeon, player);
}

/*********************************************************
// MAIN FUNCTION
// ********************************************************/
void main() {

	// initialize mutex
	pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;

	// create timeKeeper thread; taken from lecture 2.3 Concurrency
	int resultInt;
	pthread_t timeKeeper;

	// timeKeeper updates the time file every 60 seconds
	resultInt = pthread_create( &timeKeeper, NULL, makeTimeFile, (void *) &myMutex);

	// initialize dungeon
	struct Dungeon myDungeon=makeDungeon();

	// initialize player
	setPlayer(&myDungeon, 0, 0);

	// load the names of each room from the room files into the dungeon structure
	int i;
	for (i=0; i<NUMROOMS; i++)
	{
		loadRoomNames(&myDungeon, getRoomPath(i+1), i);
	}

	// open each room file and load its attributes into the dungeon structure
	for (i=0; i<NUMROOMS; i++)
	{
		loadRoomAttributes(&myDungeon, getRoomPath(i+1), i);
	}

	// print the dungeon (this is for testing)
	// printDungeon(&myDungeon);

	// start game loop
	gameLoop(&myDungeon, &myMutex);

	// destroy mutex
	pthread_mutex_destroy(&myMutex);

	// destroy the timeKeeper thread (otherwise its an infinite loop that never ends)
	pthread_cancel(timeKeeper);

}	