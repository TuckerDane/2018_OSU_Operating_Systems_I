#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>


/**********************************************************
// GLOBAL VARIABLES
// ********************************************************/
int foregroundOnly;		// 1 = foregroundOnly mode, 0 = background mode
int exitStat;			// holds the exit status of last process executed

#include "getUI.c"
#include "processUI.c"
#include "runC.c"

/**********************************************************
// SIGNAL HANDLERS
// ********************************************************/

// catchSIGTSTP
//
// description: catches a SIGTSTP signal, displaying an
//				informative message immediately after any
//				currently running foreground process has
//				terminated, and then enters a state where
//				subsequent commands can no longer be run
//				in the background
//
// @param:		bufferLength - a size_t length for the
//				buffer
// @return:		buffer - the 0-initialized buffer
//..........................................................
void catchSIGTSTP(int signo)
{
	if(foregroundOnly == 1)	// if currently not in foreground-only mode
	{
		// print informative message
		char* message = "Entering foreground-only mode (& is now ignored)\n";
		fflush(stdout);
		write(STDOUT_FILENO, message, 49);

		// enter foreground-only mode
		foregroundOnly = 0;
	}
	else	// else, currently in foreground-only mode
	{
		// print informative message
		char* message = "Exiting foreground-only mode\n";
		fflush(stdout);
		write(STDOUT_FILENO, message, 29);

		// exit foreground-only mode
		foregroundOnly = 1;
	}
}

/**********************************************************
// FUNCTIONS
// ********************************************************/

// makeBuffer
//
// description: takes a buffer length, and allocates
//				memory size bufferLength, setting each
//				value within the buffer to 0
//
// @param:		bufferLength - a size_t length for the
//				buffer
// @return:		buffer - the 0-initialized buffer
//..........................................................
char* makeBuffer(size_t bufferLength)
{
	char* buffer = calloc(bufferLength, sizeof(char));			// buffer that holds up to 2048 characters
	if(!buffer)
	{
		fprintf(stderr, "error allocating buffer array\n");
		exit(1);
	}

	return buffer;	
}

// makeArgumentArray
//
// description: takes a length for the number of arguments
//				and greates an array of pointers to strings
//				which represent the arguments.
//
// @param:		argumentArrayLength - an integer which
//				indicates the nubmer of arguments
// @return:		arguments - the array of pointers to args
//..........................................................
char** makeArgumentArray(int argumentArrayLength)
{
	char** arguments = calloc(argumentArrayLength, sizeof(char*));		// an array of pointers of up to 512 arguments
	if(!arguments)
	{
		fprintf(stderr, "error allocating arguments array\n");
		exit(1);
	}

	return arguments;
}

// shellLoop
//
// description: a loop that runs the shell itself. first
//				initializes all memory and variables
//				needed for the shell, then enters a loop.
//				the loop does the following:
//
//				1. GET USER INPUT - get text input from the
//				   user
//				2. PROCESS USER INPUT - converts the user
//				   input to a formatted argument array
//				3. RUN COMMANDS - runs the command and
//				   arguments in the argument array
//
//				The loop then frees memory as appropriate
//..........................................................
void shellLoop()
{
	fflush(stdin);

	// Allocate Memory
	//....................
	size_t bufferLength = 2048;
	int argumentArrayLength = 512;
	char* buffer = makeBuffer(bufferLength);
	char* processedBuffer;
	char** arguments = makeArgumentArray(argumentArrayLength);
	char* redirectVals[2] = {NULL, NULL};	// {inputPath, outputPath}
	foregroundOnly = 0;						// foreground only mode initialized to off

	int backgroundFlag = 0;					// 1 = process is to be run in background; 0 = process is to be run in foreground
	int* runInBackground = &backgroundFlag;	

	int exitStatus = 1;
	int* exitPtr = &exitStatus;


	// Handle Signals
	//....................

	// Make the shell ignore SIGINTs - reference @368 on Piazza Board
	struct sigaction SIGINT_action;				// make structs for SIGINT
    SIGINT_action.sa_handler = SIG_IGN;			// set the sa_handler of SIGINT_action to SIG_IGN
    sigaction(SIGINT, &SIGINT_action, NULL);	// ignore a SIGINT signal

	// Handle SIGTERM
  	struct sigaction SIGTSTP_action;				// make structs for SIGTSTP
    SIGTSTP_action.sa_handler = catchSIGTSTP;		// set the sa_handler of SIGTSTP_action to the catchSIGTSTP function
    sigfillset(&SIGTSTP_action.sa_mask);
  	SIGTSTP_action.sa_flags = 0;
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);		// on SIGTSTP, use the SIGTSTP_action handler

	// Shell Loop
	//....................
	do{
		// CONTROLLER: get input from the user
		getUserInput(buffer, bufferLength);

		// MODEL: process user input
		processedBuffer = processUserInput(buffer, arguments, redirectVals, runInBackground, exitPtr);	

		// VIEW: update user
		runCommands(arguments, redirectVals, runInBackground, exitPtr);

		free(processedBuffer);
	} while(exitStatus);						// while the exit command has not been called

	// Garbage Collection
	//....................
	free(buffer);
	free(arguments);
}

// main
//
// description: main function of the shellsh program. This
//				simply runs the shellLoop defined above and
//				returns 0 when complete.
//..........................................................
int main ()
{
	shellLoop();
	return 0;
}