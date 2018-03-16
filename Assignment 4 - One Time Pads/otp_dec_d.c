/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CS344 - Operating Systems I
//
// Name:		Tucker Dane Walker
// Date:		13 March 2018
// Description:	otp_dec_d.c
//
//		[x]		This program performs exactly like otp_enc_d, in syntax and usage. In this case, however, 
//				otp_dec_d will decrypt ciphertext it is given, using the passed-in ciphertext and key. Thus, 
//				it returns plaintext again to otp_dec.
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
#define MID_SENTINEL	"##"	// a sentinel which separates the ciphertext from the key
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
			if(strstr(transmission, DEC_CLIENT) == NULL)										// if the client is not otp_enc, end the communication
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

			// decrypt the ciphertext
			// ......................
			keyStr = strstr(transmission, MID_SENTINEL) + 2;						// find the key address string in the tranmission buffer
			i=2;																	// starting after the sentinel,
			while(transmission[i] != '#')											// iterate through the message and decrypt it
			{
				message = transmission[i];											// set the next ciphertext message character
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
								
				result = (message-key+27)%27+'A';									// calculate the decryption of characters
				//printf("message: %d\t-\tkey: %d\tmod27(%d)\t+\t65\t=\tresult:%c\n", message, key, (message-key+27)%27, result);

				if (result == '[')
				{
					result = ' ';
				}

				// printf("DEC: (%c - %c)mod27 = %c\n", message, key, result);

				transmission[i] = result;											// set the transmission to be sent back to the decrypted character
				// printf("%c + %c = %c\n", message, key, result);					// TESTING print out the decryption
				i++;																// move to the next character
			}
			transmission[i] = '\0';													// add a null terminator to the tranmission to be sent back
			// printf("i = %d\n", i);												// TESTING print the total number of characters decrypted

			// Send the decrypted message back to the client
			// ......................
			charsRead = send(establishedConnectionFD, &transmission[2], i, 0);		// Send decrypted message back
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