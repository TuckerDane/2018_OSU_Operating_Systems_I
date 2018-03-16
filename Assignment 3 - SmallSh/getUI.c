// getUserInput
//
// description: takes input from the user and places it
//				into a character array for processing
//
// reference: 	https://oregonstate.instructure.com/courses/1662153/pages/3-dot-3-advanced-user-input-with-getline
//
// @param:      buffer - a character array that holds the
//				user input for processing
// @param:		bufferLength - the size of the character
//				array
//..........................................................
void getUserInput(char* buffer, size_t bufferLength)
{
	int numCharsEntered = -5;												// initialize counter to see if getline caught anything from user
	memset (buffer, 0, bufferLength);										// clear the buffer
	while(1)
    {
		printf(": ");														// write the prompt colon to the creen
		numCharsEntered = getline(&buffer, &bufferLength, stdin);			// read input from the user
		if (numCharsEntered == -1)
		{
			clearerr(stdin);    											// clear if no stdin
		}
		else
		{
		break; 																// exit loop; input obtained
		}
    }
}	