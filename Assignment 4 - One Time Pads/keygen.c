/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CS344 - Operating Systems I
//
// Name:		Tucker Dane Walker
// Date:		11 March 2018
// Description:	keygen - This program creates a key file of specified length. The characters in the file 
// 				generated will be any of the 27 allowed characters, generated using the standard UNIX 
//				randomization methods. Do not create spaces every five characters, as has been historically 
//				done. Note that you specifically do not have to do any fancy random number generation: we’re 
//				not looking for cryptographically secure random number generation! rand() is just fine. The 
//				last character keygen outputs should be a newline. All error text must be output to stderr, 
//				if any.
//
//				[x] The syntax for keygen is as follows: keygen keylength
//					[x] Where keylength is the length of the key file in characters. 
//					[x] keygen outputs to stdout. Here is an example run, which redirects stdout to a 
//						key file of 256 characters called “mykey” (note that mykey is 257 characters long 
//						because of the newline): $ keygen 256 > mykey
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>

/**********************************************************
// GLOBAL DEFINITIONS
// ********************************************************/
#define RED  "\x1B[31m"		// red text
#define GRN  "\x1B[32m"		// green text
#define CYN  "\x1B[36m"		// cyan text
#define NRM  "\x1B[0m"		// normal text
#define TRUE 1				// TRUE == 1
#define FALSE 0				// FALSE == 0

/**********************************************************
// HELPER FUNCTIONS
// ********************************************************/

// usageMessage
//
// description: provides the user with the syntax for using
//				the keygen program
//	
//..........................................................
void usageMessage()
{
	fprintf(stderr, "%sUsage\n-----\n%skeygen [keylength]\n%swhere %s[keylength] %sis the length of the key file in characters%s\n\n", GRN, CYN, GRN, CYN, GRN, NRM);
}

// genRandCharacter
//
// description: generates a random character in the range
//				A-Z (inclusive) plus the SPACE character
//	
//	@return		randC - a random character in the range
//				A-Z (inclusive) plus the SPACE character
//..........................................................
char genRandCharacter()
{
	char randC = 'b';				// initialize a random character variable

	randC = 'A' + (rand() %27);		// generate a random character in the range A-[ (inclusive)
	if(randC == '[')
	{
		randC = ' ';				// if the character generated is '[', replace it with a SPACE
	}

	return randC;					// return the random character
}

// validateArguments
//
// description: validates that a single argument is passed
//				to keygen and that it is an integer in the
//				range 1-10000 (inclusive)
//
// @param:		argc - the number of arguments passed to
//				keygen
// @param:		argv[] - an array holding the arguments
//				passed to keygen
// @param:		correctNum - the correct number of
//				arguments expected
// @return:		TRUE - if the arguments are valid
//				FALSE - if the arguments are invalid
//..........................................................
int validateArguments(int argc, char* argv[], int correctNum)
{
	assert(argv);	// ensure argv exists

	// validate the number of arguments passed (argc) against the correct number of arguments expected (correctNum)
	if(argc != correctNum)
	{
		if (argc < correctNum)	// let the user know that they passed too few arguments
		{
			fprintf(stderr, "\n%sERROR: too few arguments passed!%s\n\n", RED, NRM);
		}
		else // (argc > correctNum)	let the user know they passed too many arguments
		{
			fprintf(stderr, "\n%sERROR: too many arguments passed!%s\n\n", RED, NRM);
		}

		usageMessage();	// provide the function usage message so the user knows what to type in

		return FALSE;
	}

	// validate that the argument passed (argv) is greater than 0 and less than 2,000,000,000 (just under the size of an int)
	if(atoi(argv[1]) < 1 || atoi(argv[1]) > 2000000000)
	{
		fprintf(stderr, "\n%sERROR: enter an integer between 1 and 2,000,000,000 (inclusive)!%s\n\n", RED, NRM);
		usageMessage();
		return FALSE;
	}

	return TRUE;
}

/**********************************************************
// MAIN FUNCTION
// ********************************************************/

// main
//
// description: This program creates a key file of specified
//				length
//
// @param:		argc - the number of arguments passed
// @param:		argv[] - an array of passed arguments
// @return:		0 - indicates a successfully run program
//				!0 - indicates an error
//..........................................................
int main(int argc, char* argv[])
{
	// initialize rand
	srand(time(NULL));

	// validate that a single, numeric argument was passed to keygen
	if(validateArguments(argc, argv, 2) == 0)
	{
		// if the arguments passed are invalid, exit the program and return 1
		return 1;
	}

	// initialize variables
	int keyLength = atoi(argv[1]);					// set keyLength to the argument passed to keygen
	int i = 0;										// a looping variable
	char* key = calloc(keyLength+1, sizeof(char));	// holds a keyLength size key plus a null terminator
	char randC[2] = "a";							// holds a random character to add to key

	// generate random characters in the range A-Z (inclusive) and SPACE to enter into the key
	for(i=0; i<keyLength; i++)
	{
		randC[0] = genRandCharacter();
		key = strcat(key, randC);
	}

	// print the key, plus a newline, to stdout
	printf("%s\n", key);

	// garbage collection
	free(key);

	return 0;
}