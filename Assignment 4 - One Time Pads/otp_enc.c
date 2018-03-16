/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CS344 - Operating Systems I
//
// Name:		Tucker Dane Walker
// Date:		13 March 2018
// Description:	
//
//		[x]		otp_enc.c - This program connects to otp_enc_d, and asks it to perform a one-time pad style 
//				encryption as detailed above. By itself, otp_enc doesn’t do the encryption - otp_end_d does. The 
//				syntax of otp_enc is as follows: otp_enc plaintext key port
//
//		[x]		In this syntax, plaintext is the name of a file in the current directory that contains the 
//				plaintext you wish to encrypt. Similarly, key contains the encryption key you wish to use to 
//				encrypt the text. Finally, port is the port that otp_enc should attempt to connect to otp_enc_d 
//				on.
//
//		[x]		When otp_enc receives the ciphertext back from otp_enc_d, it should output it to stdout. Thus, 
//				otp_enc can be launched in any of the following methods, and should send its output appropriately:
//
//		[x]		$ otp_enc myplaintext mykey 57171
//		[x]		$ otp_enc myplaintext mykey 57171 > myciphertext
//		[x]		$ otp_enc myplaintext mykey 57171 > myciphertext &
//
//		[x]		If otp_enc receives key or plaintext files with ANY bad characters in them, or the key file is 
//				shorter than the plaintext, then it should terminate, send appropriate error text to stderr, and 
//				set the exit value to 1.
//
//		[x]		otp_enc should NOT be able to connect to otp_dec_d, even if it tries to connect on the correct 
//				port - you'll need to have the programs reject each other. If this happens, otp_enc should report 
//				the rejection to stderr and then terminate itself. In more detail: if otp_enc cannot connect to 
//				the otp_enc_d server, for any reason (including that it has accidentally tried to connect to the 
//				otp_dec_d server), it should report this error to stderr with the attempted port, and set the exit 
//				value to 2. Otherwise, upon successfully running and terminating, otp_enc should set the exit value 
//				to 0.
//
//		[x]		Again, any and all error text must be output to stderr (not into the plaintext or ciphertext 
//				files).
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
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <sys/ioctl.h>
#include <sys/stat.h>

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
//
// @param		msg - an error message to report
//..........................................................
void error(const char *msg)
{ 
	fprintf(stderr, "%s", RED);		// change text color to red
	perror(msg); 					// send an error message to standard error
	fprintf(stderr, "%s", NRM);		// change text color to normal
	exit(1); 
}

// checkFile
//
// description: checks to ensure file exists, can be
//				opened, and for ANY bad characters. if
//				there are any, it sends an error text to 
//				stderr, sets the exit value to 1, and
//				terminates the program. Also returns the
//				size of the file.
//
// @param		fileName - the file to check
// @return 		fileSize - the size of the file
//..........................................................
int checkFile(char* fileName)
{ 
	// initialize variables
	FILE* fp;											// file pointer
	struct stat st;										// stat struct
	int i;												// iterator
	char testChar;										// character to test against accepted values

	// load file into stat structure
	if (stat(fileName, &st) < 0)						// send error if the file is non-existant
	{
		fprintf(stderr,"%sCLIENT: ERROR %s does not exist%s\n", RED, fileName, NRM);
		exit(1);
	}

	// open the file
	fp = fopen(fileName, "r");							// send error if file does not open
	if (fp == NULL)
	{
		fprintf(stderr,"%sCLIENT: ERROR opening %s%s\n", RED, fileName, NRM);
		exit(1);
	}

	// validate that the file contains only accepted characters
	for(i=0; i<st.st_size-1; i++)
	{
		testChar = fgetc(fp);

		if( ((testChar < 'A') || (testChar > 'Z')) && (testChar != ' ') )
		{
			fprintf(stderr,"%sCLIENT: ERROR invalid characters in %s%s\n", RED, fileName, NRM);
			exit(1);
		}
	}

	// close the file
	fclose(fp);

	// return the file size
	return st.st_size;
}

/**********************************************************
// MAIN FUNCTION
// ********************************************************/

int main(int argc, char *argv[])
{
	// initialize variables
	// ......................
	FILE* plainTextFP;
	FILE* keyFP;
	int socketFD;
	int portNumber;
	int charsWritten;
	int charsRead;
	int plainTextFileSize;
	int keyFileSize;
	int transmissionSize;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char* buffer;

	// input validation
	// ......................
	if (argc < 4) 
	{ 
		fprintf(stderr,"%sUSAGE: %s%s plaintext key port%s\n", RED, CYN, argv[0], NRM);	 // Check usage & args
		exit(1); 
	}

	// file validation
	// ......................
	plainTextFileSize = checkFile(argv[1]);										// get the size of the plain text file
	// printf("THE PLAIN TEXT FILE SIZE IS: %d\n", plainTextFileSize-1);		// print file size

	keyFileSize = checkFile(argv[2]);											// get the size of the key file
	// printf("THE KEY FILE SIZE IS: %d\n", keyFileSize-1);						// print file size

	if(plainTextFileSize > keyFileSize)											// exit with error if the key is too short to encrypt the message
	{
		fprintf(stderr,"%sCLIENT: ERROR, key is too short to encrypt message%s\n", RED, NRM);
		exit(1);
	}

	// allocate buffer:
	// ......................

	// 	Buffer Contents:
	//	[ code word 	| plain text 	| middle sentinel 	| key 			| end sentinel  | null terminator	]
	//	[ 2 chars   	| file size  	| 2 chars         	| file size 	| 2 chars 		| 1 char 			]
	//	(2*(plainTextFileSize) + 7) BYTES

	transmissionSize = (2*plainTextFileSize)+7;					// get the buffer size needed to send a full transmission
	// printf("SIZE OF FILE: %d\n", plainTextFileSize);
	buffer = calloc(transmissionSize, sizeof(char));			// allocate memory for and 0-initialize the buffer
	// printf("SIZE OF BUFFER: %d\n", sizeof(*buffer));

	// set up server address struct
	// ......................
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); 								// Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; 						// Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); 				// Store the port number
	serverHostInfo = gethostbyname("localhost"); 				// Convert the machine name into a special form of address
	if (serverHostInfo == NULL)
	{ 
		fprintf(stderr, "%sCLIENT: ERROR, no such host%s\n", RED, NRM); 
		exit(1); 
	}
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// set up the socket
	// ......................
	socketFD = socket(AF_INET, SOCK_STREAM, 0); 				// Create the socket
	if (socketFD < 0)											// ensure socket is working
	{
		error("CLIENT: ERROR opening socket");
	}
	
	// connect to server
	// ......................
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
	{
		error("CLIENT: ERROR connecting");
	}

	// Open Files
	// ......................
	plainTextFP = fopen(argv[1], "r");				// open plaintext file
	if (plainTextFP == NULL)
	{
		error("CLIENT: ERROR opening plainText file");
	}
	keyFP = fopen(argv[2], "r");					// open key file
	if (keyFP == NULL)
	{
		error("CLIENT: ERROR opening key file");
	}

	// Put the full transmission into the buffer
	// ......................
	memset(buffer, '\0', transmissionSize);							// Clear out the buffer array
	strcat(buffer, ENC_CLIENT);										// Put the ENC_CLIENT sentinel into the buffer for handshake
	fgets(&buffer[2], plainTextFileSize, plainTextFP); 				// Put the plain text into the buffer
	buffer[strcspn(buffer, "\n")] = '\0'; 							// Remove the trailing \n that fgets adds
	strcat(buffer, MID_SENTINEL);									// Put the MID_SENTINEL into the buffer
	fgets(&buffer[3+plainTextFileSize], plainTextFileSize, keyFP); 	// Put the key into the buffer
	buffer[strcspn(buffer, "\n")] = '\0'; 							// Remove the trailing \n that fgets adds
	strcat(buffer, END_SENTINEL);									// Put the END_SENTINEL into the buffer
	// printf("WHATS IN THE BUFFER: %s\n", buffer);					// See what's in the buffer (TESTING)

	// Close Files
	// ......................
	fclose(plainTextFP);							// close plaintext file
	fclose(keyFP);									// close key file

	// Send message to server
	// ......................
	charsWritten = send(socketFD, buffer, transmissionSize, 0); 				// Write to the server
	if (charsWritten < 0) 														// error message if no characters written
	{
		error("CLIENT: ERROR writing to socket");
	}
	if (charsWritten < transmissionSize)										// error message if size is too small
	{
		printf("CLIENT: WARNING: Not all data written to socket!\n");
	}

	// check to make sure full message is sent
	// ......................
	int checkSend = -5;  							// Holds amount of bytes remaining in send buffer
	do
	{
		ioctl(socketFD, TIOCOUTQ, &checkSend);  	// Check the send buffer for this socket
		// printf("checkSend: %d\n", checkSend);  	// Out of curiosity, check how many remaining bytes there are
	}
	while (checkSend > 0);  						// Loop forever until send buffer for this socket is empty
	if (checkSend < 0)  							// Check if we actually stopped the loop because of an error
	{
		error("CLIENT: ioctl error");
	}

	// Get return message from server
	// ......................
	memset(buffer, '\0', transmissionSize); 									// Clear out the buffer again for reuse
	charsRead = recv(socketFD, buffer, transmissionSize - 1, 0); 				// Read data from the socket, leaving \0 at end
	if (charsRead < 0)
	{	
		error("CLIENT: ERROR reading from socket");
	}
	else if (strcmp(buffer, CON_FAIL) == 0)
	{
		fprintf(stderr, "%sCLIENT: ERROR, connection rejected on port %d%s\n", RED, portNumber, NRM); 
		exit(2);
	}
	printf("%s\n", buffer);														// encrypted message from the server

	// free buffer
	// ......................
	free(buffer);

	// close socket
	// ......................
	close(socketFD); 															// Close the socket
	
}