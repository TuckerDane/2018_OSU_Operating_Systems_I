/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CS344 - Operating Systems I
//
// Name:		Tucker Dane Walker
// Date:		13 March 2018
// Description:	
//
//		[x]		otp_enc_d.c - This program will run in the background as a daemon. Upon execution, otp_enc_d 
//				must output an error if it cannot be run due to a network error, such as the ports being 
//				unavailable. Its function is to perform the actual encoding, as described above in the Wikipedia 
//				quote. This program will listen on a particular port/socket, assigned when it is first ran (see 
//				syntax below). When a connection is made, otp_enc_d must call accept() to generate the socket used 
//				for actual communication, and then use a separate process to handle the rest of the transaction 
//				(see below), which will occur on the newly accepted socket.
//
//		[x]		This child process of otp_enc_d must first check to make sure it is communicating with otp_enc 
//				(see otp_enc, below). After verifying that the connection to otp_enc_d is coming from otp_enc, 
//				then this child receives from otp_enc plaintext and a key via the communication socket (not the 
//				original listen socket). The otp_enc_d child will then write back the ciphertext to the otp_enc 
//				process that it is connected to via the same communication socket. Note that the key passed in 
//				must be at least as big as the plaintext.
//
//		[x]		Your version of otp_enc_d must support up to five concurrent socket connections running at the 
//				same time; this is different than the number of processes that could queue up on your listening 
//				socket (which is specified in the second parameter of the listen() call). Again, only in the child 
//				process will the actual encryption take place, and the ciphertext be written back: the original 
//				server daemon process continues listening for new connections, not encrypting data.
//
//		[x]		In terms of creating that child process as described above, you may either create with fork() a 
//				new process every time a connection is made, or set up a pool of five processes at the beginning 
//				of the program, before connections are allowed, to handle your encryption tasks. As above, your 
//				system must be able to do five separate encryptions at once, using either method you choose.
//
//		[x]		Use this syntax for otp_enc_d: otp_enc_d listening_port
//
//		[x]		listening_port is the port that otp_enc_d should listen on. You will always start otp_enc_d in 
//				the background, as follows (the port 57171 is just an example; yours should be able to use any 
//				port): $ otp_enc_d 57171 &
//
//		[x]		In all error situations, this program must output errors to stderr as appropriate (see grading 
//				script below for details), but should not crash or otherwise exit, unless the errors happen when 
//				the program is starting up (i.e. are part of the networking start up protocols like bind()). If 
//				given bad input, once running, otp_enc_d should recognize the bad input, report an error to 
//				stderr, and continue to run. Generally speaking, though, this daemon shouldn't receive bad input, 
//				since that should be discovered and handled in the client first. All error text must be output 
//				to stderr.
//
//		[x]		This program, and the other 3 network programs, should use "localhost" as the target IP 
//				address/host. This makes them use the actual computer they all share as the target for the 
//				networking connections.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

/**********************************************************
// HEADER 1
// ********************************************************/

// header 2
// ......................

// functionComment
//
// description: some description
//
// @param		parameter - 
// @return		return - 
//..........................................................

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <sys/ioctl.h>

/**********************************************************
// GLOBALS
// ********************************************************/

// message colors
// ......................
#define RED  "\x1B[31m"		// red text
#define GRN  "\x1B[32m"		// green text
#define CYN  "\x1B[36m"		// cyan text
#define NRM  "\x1B[0m"		// normal text

// transmission variables
// ......................
#define CON_FAIL 		"*"		// a sentinel which indicates that the client-server handshake was unsuccessful
#define ENC_CLIENT 		"!!"	// a sentinel which indicates that the client requests encryption
#define DEC_CLIENT 		"$$"	// a sentinel which indicates that the client requests decryption
#define MID_SENTINEL	"##"	// a sentinel which separates the plaintext from the key
#define END_SENTINEL	"@@"	// a sentinel which indicates the end of transmission
#define BUFFERSIZE		1024	// size of the buffer
#define	MAX_TRANS		262144	// maximum size of a transmission

/**********************************************************
// HELPER FUNCTIONS
// ********************************************************/

// error
//
// description: Error function used for reporting issues
//				that can be shown with perror
//
// @param		msg - an error message to report
//..........................................................
void error(const char *msg)
{ 
	fprintf(stderr, "%s", RED);		// change text color to red
	perror(msg);					// send an error message to standard error
	fprintf(stderr, "%s", NRM);		// change text color to normal
	exit(1); 
}

/**********************************************************
// MAIN FUNCTION
// ********************************************************/

int main(int argc, char *argv[])
{
	// initialize variables
	// ......................
	int listenSocketFD;
	int establishedConnectionFD;
	int portNumber;
	int charsRead;
	int i;								// iterator
	int message;
	int key;
	int result;
	char* keyStr;
	socklen_t sizeOfClientInfo;
	char transmission[MAX_TRANS];
	char buffer[BUFFERSIZE];
	struct sockaddr_in serverAddress, clientAddress;
	pid_t spawnPid;
	int childExitMethod;

	// input validation
	// ......................
	if (argc < 2)
	{ 
		fprintf(stderr,"%sUSAGE: %s%s port%s\n", RED, CYN, argv[0], NRM);  // Check usage & args
		exit(1); 
	}

	// set up address struct
	// ......................
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); 	// Clear out the address struct
	portNumber = atoi(argv[1]); 									// Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; 							// Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); 					// Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; 					// Any address is allowed for connection to this process

	// set up the socket
	// ......................
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); 				// Create the socket
	if (listenSocketFD < 0)											// ensure socket is working
	{
		error("ERROR opening socket");
	}

	// enable the socket to begin listening
	// ......................
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
	{
		error("ERROR on binding");															// send error message if unsuccessful
	}
	listen(listenSocketFD, 5); 																// Flip the socket on - it can now receive up to 5 connections

////////////////////BEGINWHILELOOP???//////////////////////////////////////////	

	while(1)
	{
		// Accept a connection, blocking if one is not available until one connects
		// ......................
		sizeOfClientInfo = sizeof(clientAddress); 																// Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0)																		// send error message if unsuccessful
		{
			error("ERROR on accept");
		}

	////////////////////FORK???//////////////////////////////////////////	
		// Fork new process
		// ......................
		spawnPid = -5;
		childExitMethod = -5;
		spawnPid = fork();
		if(spawnPid < 0)
		{
			error("SERVER: Fork Error");
		}
		else if (spawnPid == 0)																	// Child Process is spawned
		{
			// Get the message from the client
			// ......................
			memset(transmission, '\0', MAX_TRANS);												// Clear the transmission buffer

			// while the end sentinel is not found, keep reading into transmission
			while ( strstr(transmission, END_SENTINEL) == NULL )
			{
				memset(buffer, '\0', BUFFERSIZE);												// Clear the buffer
				charsRead = recv(establishedConnectionFD, buffer, BUFFERSIZE - 1, 0); 			// Read the client's message from the socket
				if (charsRead < 0)																// Send error message if nothing is read
				{
					error("ERROR reading from socket");
				}
				strcat(transmission, buffer);													// add the contents of the buffer into the transmission
				// printf("SERVER: Transmission is %d chars long\n", strlen(transmission));		// TESTING report length of transmission
			}
			// printf("SERVER: I received this from the client: \"%s\"\n", transmission);		// TESTING report what was read

			// verify that the client is otp_enc
			// ......................
			if(strstr(transmission, ENC_CLIENT) == NULL)										// if the client is not otp_enc, end the communication
			{
				// Send unsuccessful message to the client
				// ......................
				charsRead = send(establishedConnectionFD, CON_FAIL, 1, 0);	// Send connection fail sentinel to client
				if (charsRead < 0)											// send error message if nothing is read
				{
					error("ERROR writing to socket");
				}

				// check to make sure full message is sent
				// ......................
				int checkSend = -5;  										// Holds amount of bytes remaining in send buffer
				do
				{
					ioctl(establishedConnectionFD, TIOCOUTQ, &checkSend);  	// Check the send buffer for this socket
					// printf("checkSend: %d\n", checkSend);  				// Out of curiosity, check how many remaining bytes there are
				}
				while (checkSend > 0);  									// Loop forever until send buffer for this socket is empty
				if (checkSend < 0)											// Check if we actually stopped the loop because of an error
				{
					error("ioctl error");
				}

				close(establishedConnectionFD); 							// Close the existing socket which is connected to the client			

				fprintf(stderr, "%sSERVER: ERROR, client handshake unsuccessful%s\n", RED, NRM); 	// have the server send an error message
				exit(1);																			// exit the child process
			}

			// encrypt the plaintext
			// ......................
			keyStr = strstr(transmission, MID_SENTINEL) + 2;						// find the key address string in the tranmission buffer
			i=2;																	// starting after the sentinel,
			while(transmission[i] != '#')											// iterate through the message and encrypt it
			{
				message = transmission[i];											// set the next plaintext message character
				key = keyStr[i-2];													// set the next key character

				if (message == ' ')
				{
					message = '[';
				}
				if (key == ' ')
				{
					key = '[';
				}

				message = message - 'A';
				key = key - 'A';

				result = (message+key)%27+'A';										// calculate the encryption of characters

				if (result == '[')
				{
					result = ' ';
				}

				// printf("ENC: (%c + %c)mod27 = %c\n", message, key, result);

				transmission[i] = result;											// set the transmission to be sent back to the encrypted character
				// printf("%c + %c = %c\n", message, key, result);					// TESTING print out the encryption
				i++;																// move to the next character
			}
			transmission[i] = '\0';													// add a null terminator to the tranmission to be sent back
			// printf("i = %d\n", i);												// TESTING print the total number of characters encrypted

			// Send the encrypted message back to the client
			// ......................
			charsRead = send(establishedConnectionFD, &transmission[2], i, 0);		// Send encrypted message back
			if (charsRead < 0)														// send error message if nothing is read
			{
				error("ERROR writing to socket");
			}

			// check to make sure full message is sent
			// ......................
			int checkSend = -5;  										// Holds amount of bytes remaining in send buffer
			do
			{
				ioctl(establishedConnectionFD, TIOCOUTQ, &checkSend);  	// Check the send buffer for this socket
				// printf("checkSend: %d\n", checkSend);  				// Out of curiosity, check how many remaining bytes there are
			}
			while (checkSend > 0);  									// Loop forever until send buffer for this socket is empty
			if (checkSend < 0)											// Check if we actually stopped the loop because of an error
			{
				error("ioctl error");
			}

			close(establishedConnectionFD); 							// Close the existing socket connecting the client to the child
			exit(0);													// exit the child process
		}
		waitpid(spawnPid, &childExitMethod, WNOHANG);			// don't wait for the child to exit before setting up new sockets
		close(establishedConnectionFD); 						// Close the existing socket connecting the client to the parent
	}
	close(listenSocketFD); 										// Close the listening socket
	return 0; 
}