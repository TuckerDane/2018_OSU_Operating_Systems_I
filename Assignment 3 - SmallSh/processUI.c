// replaceString
//
// description: takes a string and replaces all instances
//              of a substring with a new string inside of
//              that original string. Then returns the
//              original string with the replacements made
// references:  https://stackoverflow.com/questions/779875/what-is-the-function-to-replace-string-in-c
//
// @param:      origStr - the original string that needs
//              to have substrings within it replaced
// @param:      oldStr - the old substring that needs to be
//              replaced with a new string inside of the
//              original string
// @param:      newStr - a new substring that will replace
//              the old string within the original string
// @return:     returnStr - a string to be returned that
//              has all instances of the old string within
//              the original string replaced with the new
//              string
//
//..........................................................
char* replaceString(char* origStr, char* oldStr, char* newStr) {

    // initialize variables
    char* tempStr = origStr;    // temporary string for processing the original
    char* returnStr;            // a return string
    int rCount = 0;             // number of times the old string repeats
    int newCharNum = 0;         // number of characters in the complete new string after replacements
    int i = 0;                  // generic looping variable
    char* token;                // token string for tokenizing based off of the oldStr

    // find the number of times that the oldString occurs in the originalString while a substring exsists
    while(strstr(tempStr, oldStr) != 0)
    {
        rCount++;                                                   // increment the count
        tempStr = (strstr(tempStr, oldStr) + strlen(oldStr));       // move the start point of tempStr past the substring
    }

    // allocate memory for the new string
    newCharNum = (strlen(origStr) + ((strlen(newStr) - strlen(oldStr)) * rCount)+1);

    returnStr = (char*)malloc(newCharNum);
    memset(returnStr, '\0', newCharNum);

    if(rCount != 0)                                                         // if there is a substring within the original...
    {
        tempStr = origStr;                                                  // set the tempStr to the original for processing
        strncpy(returnStr, origStr, strstr(tempStr, oldStr)-origStr);       // copy the first part of the original string, up to the substring, into the return string
        while(strstr(tempStr, oldStr) != 0)                                 // while substrings still exist...
        {
            strcat(returnStr, newStr);                                      // add the new substring to the return string
            tempStr = (strstr(tempStr, oldStr) + strlen(oldStr));           // set the temp string to just past the old substring
            strncat(returnStr, tempStr, strstr(tempStr, oldStr)-tempStr);   // and add the next section of the original string into the return string
        }    
    }
    else                                            // otherwise there is not a substring within the original
    {
        strcpy(returnStr, origStr);                 // therefore copy the original string into the return value
    }

    return returnStr;                               // return the updated string stored in tempStr
}

// addPID
//
// description: takes a string and replaces all instances
//              of the substring '$$' with the proccessID
//              of the function's current process.
//
// @param:      buffer - a character array containing the
//              string to have '$$' replaced with the
//              processID
// @return:     newBuffer - a new character array that has
//              all instances of '$$' replaced with the
//              processID of the funciton's current process
//..........................................................
char* addPID(char* buffer)
{
	char pidBuffer[10];								     // get a buffer to hold the process id
	memset (pidBuffer, '\0', 10);					     // clear the pidBuffer
	snprintf(pidBuffer, 10, "%d", getpid());		     // convert the process id to a string and store in the buffer

	char* newBuffer = replaceString(buffer, "$$", pidBuffer);	// replace instances of $$ with the process id

	return newBuffer;
}

// tokenize
//
// description: takes a character array and tokenizes it
//              into separate and distinct arguments
//              wherever substrings are separated by a space
//              or newline character
//
// @param:      buffer - a character array which contains
//              the string to tokenize
// @param:      arguments - an array of pointers to
//              substrings of buffer which represent the
//              arguments being tokenized
//..........................................................
void tokenize(char* buffer, char** arguments)
{
    // tokenize the buffer into separate arguments
    char* token;
    token = strtok (buffer, " \n");     // spaces and newlines are what separate each argument
    int i = 0;
    while (token != NULL)               // while there are still tokens, add each substring token to the argument array
    {
        arguments[i] = token;
        token = strtok (NULL, " \n");

        i++; 
    }
}

// processSpecialOperators
//
// description: takes the argument list and removes any
//              special variables, storing the redirect
//              paths in a separate array and setting the
//              runInBackground flag if necessary
//
// @param:      arguments - an array containing the argument
//              list
// @param:      redirectVals - an array containing the input
//              and/or output paths if redirection is passed
//              as an argument
//..........................................................
void processSpecialOperators(char** arguments, char** redirectVals, int* runInBackground)
{
    // Clear the redirect Vals buffers
    redirectVals[0] = NULL;
    redirectVals[1] = NULL;

    // reset runInBackground to 0 (false)
    *runInBackground = 0;

    int j=0;
    int i=0;
    while(arguments[i] != NULL)                         // iterate through all arguments
    {
        if (strcmp(arguments[i], "<") == 0)             // if the argument is an input operator "<"
        {
            redirectVals[0] = arguments[i+1];           // add the following argument to redirectVals[0] (INPUT PATH)

            j=i;                                        // starting from i, shorten the arguments list for the "<" operator and path
            do
            {
                arguments[j] = arguments[j+2];
                j++;
            }while (arguments[j] != NULL);
        }
        else if (strcmp(arguments[i], ">") == 0)        // otherwise if the argument is the output operator ">"
        {
            redirectVals[1] = arguments[i+1];           // add the following argument to redirectVals[1] (OUTPUT PATH)

            j=i;                                        // starting from i, shorten the arguments list for the ">" operator and path
            do
            {
                arguments[j] = arguments[j+2];
                j++;
            }while (arguments[j] != NULL);
        }
        else if (strcmp(arguments[i], "&") == 0 && arguments[i+1] == NULL)        // otherwise if the last argument is the "&" operator
        {
            *runInBackground = 1;                                                  // set runInBackground to 1 (true)
            j=i;                                                                  // starting from i, shorten the arguments list for the "<" operator and path
            do
            {
                arguments[j] = arguments[j+1];
                j++;
            }while (arguments[j] != NULL);
            i++;
        }
        else
        {
            i++;
        }
    }
}

// processUserInput
//
// description: takes a character array representing the
//              user's input and inserts the processID in
//              any location that '$$' is found, then 
//              separates the string into distinct arguments
//              wherever substrings are separated by a space
//              or newline character
//
// @param:      buffer - a character array which contains
//              the string to process
// @param:      arguments - an array of pointers to
//              substrings of buffer which represent the
//              arguments that are separated out
// @return:     newBuffer - the processed buffer is sent
//              back to the calling funciton so that its
//              memory can be freed
//..........................................................
char* processUserInput(char* buffer, char** arguments, char** redirectVals, int* runInBackground, int* exitPtr)
{
    // add the process ID to the buffer anywhere that $$ is encountered
    char* newBuffer;
    newBuffer = addPID(buffer);               // replace all instances of '$$' with the process ID
    int i = 0;
    while(arguments[i] != NULL)               // set all values within the arguments array to NULL so
    {                                         // that there are no garbage values in the array
        arguments[i] = NULL;
        i++;
    }

    tokenize(newBuffer, arguments);           // process each argument in the newBuffer string into the arguments array
    processSpecialOperators(arguments, redirectVals, runInBackground);

    return newBuffer;
}