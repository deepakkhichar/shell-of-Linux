#include<sys/types.h>
#include<sys/wait.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<readline/history.h>
#include<readline/readline.h>
#include<unistd.h>


#define MAX_INPUT_STR_LENGTH 1000 // max number of letters to be supported
#define MAX_INPUT_CMD_LENGTH 100 // max number of commands to be supported
#define num_max_process 1000 // max number of process for ps_history
#define clear() printf("\033[H\033[J")

char cmd_history_1[MAX_INPUT_STR_LENGTH]="";
char cmd_history_2[MAX_INPUT_STR_LENGTH]="";
char cmd_history_3[MAX_INPUT_STR_LENGTH]="";
char cmd_history_4[MAX_INPUT_STR_LENGTH]="";
char cmd_history_5[MAX_INPUT_STR_LENGTH]="";


int pid_of_processes[num_max_process];
int numprocesses=0;

void cmdHistoryAdd(char newstr[])
{
	strcpy(cmd_history_5,cmd_history_4);
	strcpy(cmd_history_4,cmd_history_3);
	strcpy(cmd_history_3,cmd_history_2);
	strcpy(cmd_history_2,cmd_history_1);
	strcpy(cmd_history_1,newstr);
	return;
}

int parseEqual(char* str, char** parsed)
{
	int i=0;
	for (i = 0; i < MAX_INPUT_CMD_LENGTH; i++) {
		parsed[i] = strsep(&str, "=");

		if (parsed[i] == NULL)
			break;
		if (strlen(parsed[i]) == 0)
			i--;
	}
	for(int j=0; j<strlen(parsed[0]); j++)
	{
		if(!isalnum(parsed[0][j]) && parsed[0][j]!='-' && parsed[0][j]!='_')
			return 0;
	}
	return i;
}

void cmdHistoryPrint()
{
	printf("Last 5 executed commands are as follows:\n\n");
	printf("%s\n",cmd_history_1);
	printf("%s\n",cmd_history_2);
	printf("%s\n",cmd_history_3);
	printf("%s\n",cmd_history_4);
	printf("%s\n\n",cmd_history_5);
	return;
}

void psHistoryPrint()
{
	printf("PID    STATUS\n");
	int status;
	for(int i=0;i<numprocesses;i++){
		if(waitpid(pid_of_processes[i],&status,WNOHANG)==0){
			printf("%d RUNNING\n",pid_of_processes[i]);
		}else{
			printf("%d STOPPED\n",pid_of_processes[i]);
		}
	}
	return;
}

int takeInput(char* str)
{
	char* buf;

	buf = readline(" ");
	if (strlen(buf) != 0) {
		add_history(buf);
		strcpy(str, buf);
		return 0;
	} else {
		return 1;
	}
}

void printShellPrompt()
{
	char cwd[1024];
	getcwd(cwd, sizeof(cwd));
	printf("%s~$", cwd);
	fflush( stdout );
}

void execArgs(char** parsed)
{
	char *parsedArgs[MAX_INPUT_CMD_LENGTH];
	int ss = parseEqual(parsed[0], parsedArgs);
	if(ss == 2)
	{
		setenv(parsedArgs[0], parsedArgs[1], 1);
		return;
	}
	// Forking a child
	pid_t pid = fork();
	pid_of_processes[numprocesses]=pid;
	numprocesses=numprocesses+1;
	int background = 0;
	if(parsed[0][0] == '&')
	{
		background = 1;
		*parsed = parsed[0]+1;
	}
	
	if (pid == -1)
	{
		printf("\nFailed forking child..");
		return;
	} else if (pid == 0) 
	{
		
		if (execvp(parsed[0], parsed) < 0) 
		{
			printf("\n\nError: Command not found...\n\n");
		}
		exit(0);
	} else 
	{
		if(background == 1)
		{
			printf("The child process with pid: %d is running in background.\n", pid);
			return;
		}
		wait(NULL);
		return;
	}
}

void execute_piped_args(char** parsed, char** parsedpipe)
{
	int pipefd[2];
	pid_t p1, p2;

	if (pipe(pipefd) < 0) {
		printf("\nCannot Initialize the Pipe");
		return;
	}
	p1 = fork();
	pid_of_processes[numprocesses]=p1;
	numprocesses=numprocesses+1;
	if (p1 < 0) {
		printf("\nFailed forking child");
		return;
	}

	if (p1 == 0) {
		close(pipefd[0]);
		dup2(pipefd[1], STDOUT_FILENO);
		close(pipefd[1]);

		if (execvp(parsed[0], parsed) < 0) {
			printf("\n Error in executing command 1..");
			exit(0);
		}
	} else {
		p2 = fork();
		pid_of_processes[numprocesses]=p2;
		numprocesses=numprocesses+1;
		if (p2 < 0) {
			printf("\nFailed forking child");
			return;
		}

		if (p2 == 0) {
			close(pipefd[1]);
			dup2(pipefd[0], STDIN_FILENO);
			close(pipefd[0]);
			if(strcmp(parsedpipe[0],"cmd_history")==0){
				cmdHistoryPrint();
				exit(0);
			}
			if(strcmp(parsedpipe[0],"ps_history")==0){
				psHistoryPrint();
				exit(0);
			}
			if (execvp(parsedpipe[0], parsedpipe) < 0) {
				printf("\n Error in executing command 2..");
				exit(0);
			}
		} else {
			wait(NULL);
			wait(NULL);
		}
	}
}


int exec_cd_or_exit(char** parsed)
{
	int num_cd_exit_cmd = 2, i, switchOwnArg = 0;
	char* ListOfOwnCmds[num_cd_exit_cmd];

	ListOfOwnCmds[0] = "exit";
	ListOfOwnCmds[1] = "cd";
	

	for (i = 0; i < num_cd_exit_cmd; i++) {
		if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0) {
			switchOwnArg = i + 1;
			break;
		}
	}

	switch (switchOwnArg) {
	case 1:
		printf("\nTerminating\n");
		exit(0);
	case 2:
		chdir(parsed[1]);
		return 1;
	
	default:
		break;
	}

	return 0;
}

int check_piping(char* str, char** strpiped)
{
	int i;
	for (i = 0; i < 2; i++) {
		strpiped[i] = strsep(&str, "|");
		if (strpiped[i] == NULL)
			break;
	}

	if (strpiped[1] == NULL)
		return 0; 
	else {
		return 1;
	}
}

// function for parsing command words
void parseSpace(char* str, char** parsed)
{
	int i;

	for (i = 0; i < MAX_INPUT_CMD_LENGTH; i++) {
		parsed[i] = strsep(&str, " ");

		if (parsed[i] == NULL)
			break;
		if (strlen(parsed[i]) == 0)
			i--;
	}
}

int processInput(char* str, char** parsed, char** parsedpipe)
{

	char* strpiped[2];
	int piped = 0;

	piped = check_piping(str, strpiped);

	if (piped) {
		parseSpace(strpiped[0], parsed);
		parseSpace(strpiped[1], parsedpipe);

	} else {

		parseSpace(str, parsed);
	}
	if (exec_cd_or_exit(parsed))
		return 0;
	else
		return 1 + piped;
}
 

char* updateEnv(char* inputString)
{
	int i;
	int j;
	int size = 0;
	char newStr[MAX_INPUT_STR_LENGTH];
	for(i=0; i<strlen(inputString); i++)
	{
		if(inputString[i] == '$')
		{
			
			for(j=i+1; j<strlen(inputString); j++)
			{
				if (!isalnum(inputString[j]) && inputString[j]!='-' && inputString[j]!='_')
					break;
				// if(!(inputString[j] > 'a' && inputString[j] < 'z') || (inputString[j] > 'A' && inputString[j] < 'Z') || (inputString[j] > '0' && inputString[j] < '9'))
					// break;
				newStr[j-i-1] = inputString[j];
				size++;
			}
			newStr[size] = '\0';
			break;
		}
	}
	char* ppath = getenv(newStr);
	if(ppath==NULL)return inputString;
	if(i != strlen(inputString))
	{
		char*  ret = malloc(MAX_INPUT_STR_LENGTH*sizeof(char) );
		for(int k=0; k<i; k++)
		{
			ret[k] = inputString[k];
		}
		size = strlen(ppath);
		for(int k=0; k<size; k++)
		{
			ret[i+k] = ppath[k];
		}
		for(int k=j; k<strlen(inputString); k++)
		{
			ret[i+size+k-j] = inputString[k];
		}
		return ret;
	}
	return inputString;
}


int main()
{
    char inputString[MAX_INPUT_STR_LENGTH], *parsedArgs[MAX_INPUT_CMD_LENGTH];
    char* parsedArgsPiped[MAX_INPUT_CMD_LENGTH];
    int execFlag = 0;
	clear();
    char* username = getenv("USER");
    printf("Hello %s \nWelcome to my shell.\n", username);
    while (1) {
        printShellPrompt();
        if (takeInput(inputString))
            continue;
		
		if(strcmp(inputString,"cmd_history")==0){
			cmdHistoryPrint();
			cmdHistoryAdd(inputString);
			continue;
		}
		
		if(strcmp(inputString,"ps_history")==0){
			psHistoryPrint();
			cmdHistoryAdd(inputString);
			continue;
		}

		cmdHistoryAdd(inputString);
        char* inputString1 = updateEnv(inputString);

        execFlag = processInput(inputString1,
        parsedArgs, parsedArgsPiped);
        
        if (execFlag == 1)
            execArgs(parsedArgs);
  
        if (execFlag == 2)
            execute_piped_args(parsedArgs, parsedArgsPiped);
    }
    return 0;
}