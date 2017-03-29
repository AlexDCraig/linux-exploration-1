/* smallsh.c by Alex Hoffer
 * Description: Creates a small shell that has three built-in commands: cd, exit, and status.
 * Can also accept any valid bash command and up to 512 arguments to use as input to these commands.
 * The user has the option to choose the command, the arguments, whether the command is redirected to
 * an input file or output file.
 * The user can choose to make the command run as a background process, but it runs as a foreground 
 * process by default. The small shell waits for a child process of the foreground to complete
 * before it continues, but doesn't wait for background child processes- it periodically checks
 * on them. Only child processes watched by the foreground can be terminated with SIGINT.
 */

#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_CHARS 2048 // maximum # of chars per arg as established by brewster
#define MAX_ARGS 512 // maximum # of arguments to the command

// this global variable will be set to 0 when a command is successful and 1 if not
// it will be set to the signal value if a signal is caught
// it will be used in the built in status command and in my background processes checker function
int shellStatusValue = 0;

// this will hold the pids of the background child processes the shell should periodically check up on
struct backgroundChildProcesses
{
	pid_t* bcpArray; // contains all pid_ts that are active
	int bcpArraySize;
	int cur_size;
};

/* This struct will be used to establish some basic information
 * about the command the user gave and will be passed through
 * the functions so the shell can properly accomodate.
 */
struct commandInformation
{
	int foregroundProcess; // by default process is run in the foreground, set to 1
	int backgroundProcess; // set to 0 unless the user's command has a &
	int isOutputRedirected; // set to 0 unless the user uses the >
	int isInputRedirected; // set to 0 unless the user uses the <
	int numArguments; // starts at 0 and increases when we parse the input based on whether there is another argument or not
	char* output; // if an output file is provided, it will be stored in this c string
	char* input; // if an input file is provided, it will be stored in this c string
};

// allocate the memory and set each member of the struct to default
struct commandInformation* setupStruct()
{
	struct commandInformation* comInf;
	comInf = malloc(sizeof(struct commandInformation));
	comInf->foregroundProcess = 1; // by default a process is foreground
	comInf->backgroundProcess = 0; // not background until we know from parsing
	comInf->isOutputRedirected = 0; // redirection does not occur until we know from parsing
	comInf->isInputRedirected = 0;
	comInf->numArguments = 0; // how many arguments will be passed to the command
	comInf->output = 0; // a c string that represents the output file the user wants to redirect output to
	comInf->input = 0;
	return comInf;
}

// create the struct that holds the background child processes
struct backgroundChildProcesses* initializeBCP()
{
	struct backgroundChildProcesses* b1;
	b1 = malloc(sizeof(struct backgroundChildProcesses));
	b1->bcpArray = malloc(sizeof(pid_t) * 10); // start with 10 but if a very large test script this will grow by a factor of 2 when its full
	b1->bcpArraySize = 10;
	b1->cur_size = 0;

	return b1;
}

// returns 1 if full
// 0 if not	
// to be used to resize the array if necessary
int isBCPArrayFull(struct backgroundChildProcesses* arr)
{
	if (arr->cur_size == arr->bcpArraySize)
		return 1;
	else
		return 0;	
}

// make a new struct, retain old values, make array bigger
struct backgroundChildProcesses* resizeBCPArray(struct backgroundChildProcesses* arr)
{
	int newSize = arr->bcpArraySize * 2; // increase size by a factor of 2
	struct backgroundChildProcesses* bcp; // temporary struct, we'll set our actual struct to this address after processing is done
	bcp = malloc(sizeof(struct backgroundChildProcesses));  
	bcp->cur_size = arr->cur_size;
	bcp->bcpArraySize = newSize;
	bcp->bcpArray = malloc(newSize * sizeof(pid_t));
	
	// copy over the old values to the fresh, bigger array
	int i = 0;

	for (i; i < arr->cur_size; i++)
	{
		bcp->bcpArray[i] = arr->bcpArray[i];
	}

	return bcp;
}

// if the array is full make it twice as big then add the new background child process's pid_t to the end of the array
// else just add it to the end and inc current size
void addBCP(pid_t bcp, struct backgroundChildProcesses* arr)
{
	if (isBCPArrayFull(arr) == 1)
	{
		struct backgroundChildProcesses* bcp;
		bcp = resizeBCPArray(arr);
		arr = bcp;
	}

	arr->bcpArray[arr->cur_size] = bcp;	
	arr->cur_size += 1;
}

// used to delete a bcp from the array
// index is the element to ignore when making a new struct
void adjustBCP(int index, struct backgroundChildProcesses* arr)
{
	struct backgroundChildProcesses* bcp1 = malloc(sizeof(struct backgroundChildProcesses)); // temporary array, we'll set our real struct to this address after processing
	bcp1->bcpArraySize = arr->bcpArraySize;
	bcp1->bcpArray = malloc(sizeof(pid_t) * arr->bcpArraySize);
	int bcp1Count = 0;	

	int i = 0;

	for (i; i < arr->cur_size; i++)
	{
		// if the loop counter is the index we've been told to ignore, the pid_t at that address won't be copied
		if (i != index)
		{
			bcp1->bcpArray[bcp1Count] = arr->bcpArray[i];
			bcp1Count++;
		}
	}

	arr->cur_size--;
	bcp1->cur_size = arr->cur_size;

	// our struct now points to the struct we made
	arr = bcp1;
}

// remove a background child process from the list to be checked on
void removeBCP(pid_t bcp, struct backgroundChildProcesses* arr)
{
	int i = 0;
	int indexToRemove;
	
	for (i; i < arr->cur_size; i++)
	{
		// loop counter i marks the pid_t we must erase
		if (arr->bcpArray[i] == bcp)
			indexToRemove = 1;
	}

	adjustBCP(indexToRemove, arr);
}			

void printIntro()
{
	// print out the required :
	printf(": ");
	fflush(stdout); // make sure all text is outputted
}

// returns user input as a single line
void getUserInput(char* inputCharacters)
{
	fgets(inputCharacters, MAX_CHARS, stdin);
	fflush(stdin);	// flush standard in for good measure
}

// parse the line by using tokens and place them into the arguments
// and check for redirecting input/output, running in the background
// commands. Change the details of the struct if any of the fields are detected
char** parser(char* inputCharacters, char** arguments, struct commandInformation* comInf, int* isComment, int* isBlankLine)
{
	char* tok; // use with strtok
	char* delimiter = " \t\n"; // where the token will stop its trimming: at spaces, tabs, and newline
	char** futureArguments = malloc(sizeof(char*) * MAX_ARGS); // cut up the tokens and put them into array of c strings 	
	memset(futureArguments, '\0', sizeof(futureArguments));

	tok = strtok(inputCharacters, delimiter); // tok now contains the first contiguous substring before a \t or \n or ' ', should be a command
	
	if (inputCharacters[0] == '\n') // command is an empty line, and should be ignored
	{
		*(isBlankLine) = 1; // isBlankLine is a pointer int that holds whether true (1) or false (0)
	}

	else if (strcmp(tok, "#") != 0) // if the command is not a comment 
	{
		while (tok != NULL) // while there is still some of the command to be parsed
		{
			if (strcmp(tok, "&") == 0) // if tok holds &. & must be the last thing received. We break because there's nothing to read after it
			{
				comInf->backgroundProcess = 1;
				comInf->foregroundProcess = 0;
				break;
			}

			else if (strcmp(tok, ">") == 0) // the user wants us to do output redirection
			{
				comInf->isOutputRedirected = 1;
			}

			else if (strcmp(tok, "<") == 0) // user wants input redirection
			{
				comInf->isInputRedirected = 1;
			}

			else // none of our special characters, since tok is not null we have a filename OR argument
			{
				if (comInf->isOutputRedirected == 1)
				{
					comInf->output = tok; // we have an output file to redirect it to
				}

				else if (comInf->isInputRedirected == 1)
				{
					comInf->input = tok; // input file to redirect it to
				}

				else // we have an argument because the user has already entered a command but hasn't entered a <, >
				{ 
					futureArguments[comInf->numArguments] = malloc(strlen(tok) * sizeof(char)); // our next argument is equal to the tokenized string
					strcpy(futureArguments[comInf->numArguments], tok);
					comInf->numArguments += 1; // we move one step forward in the array
					futureArguments[comInf->numArguments] = NULL; // we make sure the next element is set to 0 for when we loop 
				}
							
			}

			tok = strtok(NULL, delimiter); // continue parsing the command, if the next tokenized string is null the loop terminates	
		}
		printf("\n");
		fflush(stdout);
	}

	else if (strcmp(tok, "#") == 0)
	{
		*(isComment) = 1;
	}

	return futureArguments;

}

// This is used with the built in exit(). Go through the background child processes
// and mercilessly kill them all off
void killProcesses(struct backgroundChildProcesses* bcp)
{
	int i = 0;

	// SIGKILL kills the processes off. this function is only used upon exit()
	for (i; i < bcp->cur_size; i++)
	{
		kill(bcp->bcpArray[i], SIGKILL); // note i used SIGKILL. the background processes will not be waited to complete.
	}
}

/* Built in 3 commands, exit, cd, and status */
// SHELLEXIT: frees our memory up and kills off all active background processes and then exits
// Syntax allowed: exit [no arguments]
void shellExit(char** parsedInput, struct commandInformation* comInf, struct backgroundChildProcesses* bcp)
{
	// there can't be any arguments to exit
	if (parsedInput[1] != 0)
	{
		fprintf(stderr, "Error: Too many arguments provided for the exit command.\n");
		fflush(stdout);
		shellStatusValue = 1;
	}

	// no arguments given, so free what was used and exit the program
	else
	{
		free(parsedInput);
		free(comInf);
		killProcesses(bcp);
		free(bcp);
		shellStatusValue = 0;
		exit(0);
	}
}	

// SHELLCD: change the directory 
// set shellStatusValue to 0 if successful, 1 if unsuccessful
// Syntax allowed: cd [optional arg: directory]
void shellCd(char** parsedInput)
{
	// cd accepts only 1 arg or none. 
	if (parsedInput[2] != 0)
	{
		fprintf(stderr, "ERROR: Too many arguments provided for the cd command.\n");
		fflush(stdout);
		shellStatusValue = 1;
	}

	// no directory has been provided so go to home directory
	else if (parsedInput[1] == 0)
	{
		char* homePath = getenv("HOME"); // nifty getenv function grabs the Home environmental variable. for me this will take me to the hoffera directory
		chdir(homePath); // chdir accepts a path and changes the working directory to that path if it exists
		shellStatusValue = 0;
	}

	// cd has been given 1 arg, change directory to that arg
	else
	{
		char* newDirectory = parsedInput[1];
		chdir(newDirectory);
		shellStatusValue = 0;
	}
		
}

// SHELLSTATUS: print the status of the shell which consists of the manner in which the last
// command exited OR the manner in which the last command was terminated
void shellStatus(char** parsedInput)
{
	// WIFEXITED will set shellstatusvalue to nonzero value if child process terminated normally
	if (WIFEXITED(shellStatusValue))
		printf("Exit status: %d.\n", WEXITSTATUS(shellStatusValue)); // this will print the exact exit status 

	else
		printf("Terminated by signal: %d.\n", shellStatusValue); // shellStatusValue contains the termination signal int identifier because I set it to that when a signal was caught

	fflush(stdout);

	shellStatusValue = 0; // this function returned normally, so set shellStatusValue to 0
}

// The user wanted output redirection but provided no output file.
// Rather than write to stdout, we need to redirect standard output to write to /dev/null, a black hole in the middle of the universe
int redirectOutputFromSTD()
{
	int STDFileDescriptor;
	int devNullFileDescriptor;

	fflush(stdout);

	STDFileDescriptor = dup(1); // dup(1) is the file descriptor of stdout
	devNullFileDescriptor = open("/dev/null", O_WRONLY); // open /dev/null for writing
	dup2(devNullFileDescriptor, 1); // dup2() responsible for redirection, pass it 1 to indicate stdout. Change stdout to write instead to /dev/null
	close(devNullFileDescriptor);

	return STDFileDescriptor;
}

// The reverse of the above function. use dup2() to redirect STDOUT back to STDOUT from /dev/null
void redirectOutputToSTD(int STDFileDescriptor)
{
	fflush(stdout);
	dup2(STDFileDescriptor, 1);
	close(STDFileDescriptor);
}

// the user wanted input redirection but provided no input file
// Rather than write to /dev/null which is a bleak and desolate womb in the vacant heart of the uncaring cosmos,  we need to redirect standard input to stdin
void redirectInputToSTDin()
{
	int STDFileDescriptor;
	fflush(stdin);

	STDFileDescriptor = dup(0); // dup(0) is stdin

	dup2(STDFileDescriptor, 0); // set stdin back to stdin
	close(STDFileDescriptor);
}
	

// do not wait for these to complete
// parent shell needs to periodically check for background child processes
// to complete
// returns -1 if error or 0 if successful
void executeBackgroundProcess(char** parsedInput, struct commandInformation* comInf, struct backgroundChildProcesses* bcp)
{
	shellStatusValue = 0;
	pid_t backgroundPID; // holds process id of background process
	int exitValue; // passed as pointer to the waitpid function which waits for the background process

	backgroundPID = fork(); // fork returns pid of child process
	
	if (backgroundPID < 0) // fork returns neg number if fork was a failure
	{
		printf("ERROR: Forking into background process failed.\n");
		fflush(stdout);
		shellStatusValue = 1;
	}

	// fork returns 0 if child process, backgroundPID is executing
	else if (backgroundPID == 0)	
	{	
		int inputFileDescriptor, outputFileDescriptor, stdOutFileDescriptor;
		
		// before exec'ing, check to see if input/output is redirected
		if (comInf->isOutputRedirected == 1 && comInf->output != 0)
		{
			// if the file exists, open it and truncate to it. 
			// if file doesn't exist, create it
			// the other flags are to enable proper writing
			outputFileDescriptor = open(comInf->output, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWRITE | S_IWGRP | S_IWOTH);
			if (outputFileDescriptor != -1) // the file can be opened
			{
				dup2(outputFileDescriptor, 1); // set stdout instead to the valid file
				close(outputFileDescriptor);
			}

			else
			{
				printf("Can't open %s for output", comInf->output);
				shellStatusValue = 1;
			}

		}
	
		// redirect output to /dev/null because no specified output file
		else if (comInf->isOutputRedirected == 1 && comInf->output == 0)  
		{
			stdOutFileDescriptor = redirectOutputFromSTD();
		}

		// input is redirected and we have an input file
		if (comInf->isInputRedirected == 1 && comInf->input != 0)
		{
			// open the file for reading on readonly setting
			inputFileDescriptor = open(comInf->input, O_RDONLY);

			if (inputFileDescriptor != -1) // file opened successfully
			{
				dup2(inputFileDescriptor, 0); // set stdin to instead read from a valid file, 0 indicates stdin
				close(inputFileDescriptor);
			}

			else
			{
				printf("Input file %s cannot be opened", comInf->input);
				shellStatusValue = 1;
			}
		}

		// redirect input from /dev/null to stdin because no input file provided
		else if (comInf->isInputRedirected == 1 && comInf->input == 0)
		{
			redirectInputToSTDin();
		} 	
		
		if (shellStatusValue == 0)
		{
			// this command will evaluate the execvp()
			// execvp() takes the first element of a char** as the command to use and passes the remainder of the null terminated
			// char** as arguments
			// execvp returns negative value if there's an error, such as too many arguments for the command or not a valid command
			if ((execvp(parsedInput[0], parsedInput)) < 0 && (shellStatusValue == 0))
			{
				printf("ERROR: Text not understood.\n");
				fflush(stdout);
				shellStatusValue = 1;
			}
		}

		// redirect output back to std from /dev/null
		if (comInf->isOutputRedirected == 1 && comInf->output == 0)
		{
			redirectOutputToSTD(stdOutFileDescriptor);
		}

		// The child process has executed the command from execvp() and can now return to its parent
		exit(EXIT_FAILURE);
	}

	// This is the parent process because fork returned a positive value
	else
	{
		// the child didn't return an issue
		if (shellStatusValue == 0)
		{
			printf("Process ID of background process: %d\n", backgroundPID);
			addBCP(backgroundPID, bcp);

			// waitpid() with the WNOHANG flag checks on the process but does not wait for it,
			// and passing it -1 means waitpid() checks for any completed process
			// it's a background process so we don't wait on it but we do periodically check on it
			// waitpid() returns 0 if no child processes have completed
			// returns pid of completed process if a process has completed
			pid_t waitPID = -1;
			waitPID = waitpid(waitPID, &exitValue, WNOHANG);

			if (waitPID != 0) // if at least one child process has completed i.e. waitPID has changed and it hasn't changed to 0
			{
				printf("Background process %d is complete with the exit value %d.\n", waitPID, exitValue);
				removeBCP(waitPID, bcp);
				fflush(stdout);
			}
	
			// no reason to have an if-else construct here but have no time to edit it out
			// waitpid will copy 0 into exitvalue if the process is done
			if (exitValue == 0)
			{
				shellStatusValue = exitValue;
			}

			else
			{
				shellStatusValue = exitValue;
			}
		}
	}
}		
			
// there can only be one foreground process so this child must be waited for
// the only difference between this function and executeBackgroundProcess() is
// that this function uses a do while loop to make the shell do nothing until
// the child process returns
void executeForegroundProcess(char** parsedInput, struct commandInformation* comInf, struct sigaction* CtrlCStopper)
{
	shellStatusValue = 0;
	pid_t foregroundPID;
	int exitValue; // passed as pointer to the waitpid function which waits for the background process

	foregroundPID = fork(); // fork returns pid of child process
	
	if (foregroundPID < 0) // fork returns neg number if fork was a failure
	{
		printf("ERROR: Forking into background process failed.\n");
		fflush(stdout);
		shellStatusValue = 1;
	}

	// fork returns 0 if child process, foregroundPID is executing
	else if (foregroundPID == 0)	
	{
		// re set SIGINT to default just for this child process
		CtrlCStopper->sa_handler = SIG_DFL;
		CtrlCStopper->sa_flags = 0;
		sigaction(SIGINT, (&(*CtrlCStopper)), NULL);
	
		// before exec'ing, check to see if input/output is redirected
		if (comInf->isOutputRedirected == 1)
		{
			int outputFileDescriptor = -1;

			// open file for WR_ONLY (write only) O_TRUNC (erase file and put new stuff in it if it exists) and O_CREATE (create the file if it doesn't exist)
			outputFileDescriptor = open(comInf->output, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWRITE | S_IWGRP | S_IWOTH);
			if (outputFileDescriptor != -1) // valid output file
			{
				dup2(outputFileDescriptor, 1);
				close(outputFileDescriptor);
			}

			else // not openable
			{
				printf("Can't open %s for output", comInf->output);
				shellStatusValue = 1;
			}
		}

		if (comInf->isInputRedirected == 1)
		{
			
			if (comInf->input != 0) // a char* was provided as input file
			{	
				int inputFileDescriptor = -1;

				inputFileDescriptor = open(comInf->input, O_RDONLY); // open the file readonly
				
				if (inputFileDescriptor == -1) // file not readable
				{
					printf("Can't open %s for input\n", comInf->input);
					fflush(stdout);
					shellStatusValue = 1;
				}

				else
				{
					dup2(inputFileDescriptor, 0);
					close(inputFileDescriptor);
				}
			}
		}

		// error with forking such that parsedInput
		// no longer holds the command necessary for
		// execvp to execute 
		// execvp returns negative value if there's an error
		if (shellStatusValue == 0)
		{
			if (execvp(parsedInput[0], parsedInput) < 0)
			{
				printf("ERROR: Text not understood.\n");
				fflush(stdout);
				shellStatusValue = 1;
			}
		}

		// The child process has executed the command from execvp() and can now return to its parent
		exit(EXIT_FAILURE);
	}

	// This is the parent process because fork returned a positive value
	else
	{
		// child process was given a valid input
		if (shellStatusValue == 0)
		{
			pid_t waitPID;

			// LOGIC: waitpid returns with the process id of the completed process
			// waitpid copies 0 to exitvalue when the process we're waiting for has completed
			// the flag WUNTRACED checks whether the child process has stopped
			// perform this wait (do) unless WIFEXITED or WIFSIGNALED have been summoned (while)
			// WIFEXITED returns nonzero if child process is terminated with an exit
			// WIFSIGNALED returns nonzero if child process is terminated with an unhandled signal
			do
			{
				waitPID = waitpid(foregroundPID, &exitValue, WUNTRACED);
			} while (!WIFEXITED(exitValue) && !WIFSIGNALED(exitValue));

			shellStatusValue = exitValue;

			if (WIFSIGNALED(shellStatusValue))
			{
				int sigNumber = WTERMSIG(shellStatusValue);
				printf("Foreground process %d has been terminated by signal %d", waitPID, shellStatusValue);
				fflush(stdout);
			}

			// return sigint to having no effect
			CtrlCStopper->sa_handler = SIG_IGN;
			sigaction(SIGINT, (&(*CtrlCStopper)), NULL);
		}
	}

}

// simple controller: take the parsed input, check if its our built in commands, else check if background or foreground process
// and pass the parsedinput over to those functions to get cookin
void analyzeParsedInput(char** parsedInput, struct commandInformation* comInf, struct sigaction* CtrlCStopper, struct backgroundChildProcesses* bcp)
{
	if (strcmp(parsedInput[0], "exit") == 0)
	{
		shellExit(parsedInput, comInf, bcp);
	}	

	else if (strcmp(parsedInput[0], "cd") == 0)
	{
		shellCd(parsedInput);
	}

	else if (strcmp(parsedInput[0], "status") == 0)
	{
		shellStatus(parsedInput);
	}

	else
	{
		if (comInf->backgroundProcess == 1)
			executeBackgroundProcess(parsedInput, comInf, bcp);

		else
			executeForegroundProcess(parsedInput, comInf, CtrlCStopper);
	}
	
}

// this is how i check to see if a background process has completed or has been terminated from the parent shell
// it loads right before the : prompt
void checkBackgroundProcesses(struct backgroundChildProcesses* bcp)
{
	if (bcp->cur_size > 0) // if theres stuff in the bcp
	{
		shellStatusValue = 0;
		int exitValue;
		pid_t waitPID = -1;
		waitPID = waitpid(waitPID, &exitValue, WNOHANG); // WNOHANG: wait until any 1 process has completed

		if (waitPID != 0) // if at least one child process has completed i.e. waitPID has changed and it hasn't changed to 0
		{
			shellStatusValue = exitValue; // exitValue contained information about the nature of the wait() i.e. did it exit successfully

			if (WIFSIGNALED(shellStatusValue)) // if it received a signal
			{
				int sigNumber = WTERMSIG(shellStatusValue); // return the signal number thanks to WTERMSIG
				shellStatusValue = sigNumber; // make it so shellStatus still works
				printf("Background process %d has been terminated by signal %d", waitPID, shellStatusValue);
				fflush(stdout);
				removeBCP(waitPID, bcp); // get rid of that pid from our struct
			}
		
			else // no signal received, how did it exit?
			{
	 			printf("Background process %d is complete with the exit value %d.\n", waitPID, exitValue);
				fflush(stdout);
				removeBCP(waitPID, bcp);
				shellStatusValue = exitValue;
			}
		} 

		shellStatusValue = 0;
	}
}	

// main loop
void smallShell(struct sigaction* CtrlCStopper)
{
	int foreverLoop = 1;
	struct backgroundChildProcesses* bcp = initializeBCP();

	while (foreverLoop == 1) // loop goes until we exit or a horrible signal is received
	{
		// limit to 2048 chars for inputChars variable, limit to 512 arguments
		char inputCharacters[MAX_CHARS];
		char* arguments[MAX_ARGS]; // array of c strings
		memset(inputCharacters, '\0', sizeof(inputCharacters));
		memset(arguments, '\0', sizeof(arguments));	

		struct commandInformation* comInf = setupStruct();

		int* isComment = malloc(sizeof(int));
		int* isBlankLine = malloc(sizeof(int));
		*(isComment) = 0; // holds whether the command is a comment #
		*(isBlankLine) = 0; // holds whether command is a blank line

		checkBackgroundProcesses(bcp); // right before prompt, check if any background processes have completed

		printIntro();
		getUserInput(inputCharacters);
		char** parsedInput = parser(inputCharacters, arguments, comInf, isComment, isBlankLine);

		if (((*isComment) == 0) && ((*isBlankLine) == 0)) // not a command or blank line, try and execute
			analyzeParsedInput(parsedInput, comInf, CtrlCStopper, bcp);
	}
}

int main()
{
	// ignore SIGINT except make it so foreground processes can be stopped
	// only foreground child processes should be allowed to be stopped
	struct sigaction* CtrlCStopper = malloc(sizeof(struct sigaction));
	CtrlCStopper->sa_handler = SIG_IGN; // ignore the ctrl-c termination
	CtrlCStopper->sa_flags = 0;
	sigfillset(&(CtrlCStopper->sa_mask));
	sigaction(SIGINT, (&(*CtrlCStopper)), NULL);

	smallShell(CtrlCStopper);

	return 0;
}
