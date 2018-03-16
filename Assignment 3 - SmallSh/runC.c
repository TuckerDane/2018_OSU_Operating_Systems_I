// run
//
// description: spawns a child process to run the commands
//				indicated in the argument list formatted
//				as seen below:
//
//				command [arg1 arg2 ...] [< input_file] [> output_file] [&]
//
// references:	https://brennan.io/2015/01/16/write-a-shell-in-c/
//				Lecture 3.1 Processes
//
// @param:		arguments - the array of arguments to
//				run
// @param:		exitPtr - a pointer to an integer which
//				holds the exit status of the run funciton
//..........................................................
int run(char **arguments, char** redirectVals, int* runInBackground, int *exitPtr)
{
  pid_t spawnPid = -5;
  int childExitStatus = -5;
  int sourceFD = -5;
  int targetFD = -5;
  int result = -6;

  spawnPid = fork();

  switch(spawnPid)	{

  	// if there is an error forking
  	case -1:	{
  		perror("Error spawning new process\n");
  		exit(1);
  		break;
  	}

  	// if it is the child process
  	case 0:		{

      // Handle SIGINT
      //..................

      // Change SIGINT back to default
      // Make the shell ignore SIGINTs - reference @368 on Piazza Board
      struct sigaction SIGINT_action;           // redefine struct for SIGINT in child process
      SIGINT_action.sa_handler = SIG_DFL;       // changes the child process' sa handler to defualt
      sigaction(SIGINT, &SIGINT_action, NULL);  // resets SIGINT to the SIGINT_action funciton

      struct sigaction SIGTSTP_action;            // make structs for SIGTSTP
      SIGTSTP_action.sa_handler = SIG_IGN;        // set the sa_handler of SIGTSTP_action be ignore
      sigaction(SIGTSTP, &SIGTSTP_action, NULL);  // ignore a SIGTSTP signal in the foreground process

      // Handle input redirection
      // reference: Lecture 3.4 More UNIX IO
      //..................
      if(redirectVals[0] != NULL)
      {
        sourceFD = open(redirectVals[0], O_RDONLY);       // set redirectVals[0] to the input filepath filedecriptor
        if (sourceFD == -1)                               // if the filepath is invalid, throw error
        {
          perror("source open()");
          exit(1);
        }
        result = dup2(sourceFD, 0);                       // set FD 0 (stdin) to point to the sourceFD filedescriptior
        if(result == -1)                                  // if redirection didn't work, throw error
        {
          perror("source dup2()");
          exit(2);
        }
        fcntl(sourceFD, F_SETFD, FD_CLOEXEC);             // set close on exec
      }

      // handle output redirection
      // reference: Lecture 3.4 More UNIX IO
      //..................
      if(redirectVals[1] != NULL)
      {
        targetFD = open(redirectVals[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);   // set redirectVals[1] to the output filepath filedesciptior
        if (targetFD == -1)                                                     // if there is a problem, throw error
        {
          perror("target open()");
          exit(1);
        }
        result = dup2(targetFD, 1);                                             // set FD 0 (stdin) to point to the sourceFD filedecriptor
        if(result == -1)                                                        // if redirection didn't work, throw an error
        {
          perror("target dup2()");
          exit(2);
        }
        fcntl(sourceFD, F_SETFD, FD_CLOEXEC);                                   // set close on exec
      }

  		// run the arguments from the child process
  		if (execvp(arguments[0], arguments) == -1)
  		{
  			// if execvp fails to execute, print error
  			perror("Error running child process\n");
  		}
  		exit(2);
  		break;
  	}

  	// if it is the parent process
  	default:	{

      pid_t actualPid = -5;
      // if the child is to run in the background...
      if(*runInBackground == 1)
      {
          // continue while the child runs
          actualPid = waitpid(spawnPid, &childExitStatus, WNOHANG);
          exitPtr = 0;
      }
      else  // else allow the child to run in the foreground.
      {
          do
          {
            // wait for child to exit before continuing
            actualPid = waitpid(spawnPid, &childExitStatus, 0);
          }
          while(actualPid == -1 && errno == EINTR);

          exitPtr = 0;
      }

      if(WIFEXITED(childExitStatus))  // if process exited normally
      {
        int exitStatus = WEXITSTATUS(childExitStatus);
        exitStat = exitStatus;
        //printf("Terminated normally, exit status: %d\n", exitStatus);  
      }
      else if (WIFSIGNALED(childExitStatus)) // process terminated by signal
      {
        if(actualPid != -1)
        {
          int termSignal = WTERMSIG(childExitStatus);
          printf("The process was terminated by signal: %d\n", termSignal);
        }
      }

		break;
  	}

  }
}

// runCommands
//
// description: takes an array of arguments and executes
//				them. The format of these arguments are
//				as follows:
//
//				command [arg1 arg2 ...] [< input_file] [> output_file] [&]
//
// @param:		arguments - the array of arguments to
//				run
// @param:		exitPtr - a pointer to an integer which
//				holds the exit status of the runCommands
//				function
//..........................................................
void runCommands(char** arguments, char** redirectVals, int* runInBackground, int* exitPtr)
{
	// if the command is a simple return
    if (arguments[0] == NULL)
    {
        // do nothing, there are no arguments to run
    }
    // if the command is a comment (starts with #)
    else if (arguments[0][0] == '#')
    {
        // do nothing, this is a comment
    }
    // if command is exit
    else if(strcmp(arguments[0], "exit") == 0)
    {
        // set the exit status to 0 to exit the shellLoop
        *exitPtr = 0;
    }
    // if command is cd
    else if (strcmp(arguments[0], "cd") == 0)
    {
        // change the working directory to the argument stored in arguments[1]
        printf("changing directory\n");
        if(chdir(arguments[1]) != 0)
        {
            fprintf(stderr, "error changing directory!\n");
        }
    }
    // if command is status
    else if (strcmp(arguments[0], "status") == 0)
    {
        printf("exit status of last command: %d\n", exitStat);
    }
    else // the command is something else
    { 
        // run the given argument list
        run(arguments, redirectVals, runInBackground, exitPtr);
    }
}