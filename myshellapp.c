/* 
* Developed by: Leo Espinal
* Date: 10/1/2016
* Course: CS 575 Operating Systems
* Programming Assignment 1: Shell
*/

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#define MAXLINE 1024
#define MAXARGS 20
#define HISTSIZE 10 //history size
#define MY_PROMPT "centos_shell> "
#define TRUE 1
#define FALSE 0

struct COMMAND_HISTORY{
    char command[MAXLINE];
    struct COMMAND_HISTORY *next;
};

int parseCommand(const char *cmdline, char *listOfArgs[]);
void buildCommandHistory(char command[]);
void deleteCommandHistory();
void printCommandHistory();
int handleCommandPipe(const char *commandLineBuffer);

int counter = 0;
struct COMMAND_HISTORY *headListNode;
int parentProcessWaitFlag = TRUE;

int main(int argc, char *argv[]){
    pid_t childPid, parentPid, waitReturn;
    int returnErrorCode, exeCmdReturn, status, returnVal;
    char cmdline[MAXLINE], *cmdList[MAXARGS];

    parentPid = getpid();
    printf("The parent process pid is: %d\n", parentPid);

    //Start out with an empty history list for the new shell
    headListNode = (struct COMMAND_HISTORY*)malloc(sizeof(struct COMMAND_HISTORY));
    headListNode->next = NULL;

    while(1){
        printf("%s", MY_PROMPT);
        fgets(cmdline, MAXLINE, stdin);

        if(cmdline == NULL){
            exit(-1);
        }

        returnVal = parsecmd(cmdline, cmdList);
        if (returnVal == 0)
        {
            continue;
        }

        //build the command history
        buildCommandHistory(cmdline);

        //Handle the exit condition
        if(strcmp(cmdList[0], "exit") == 0)
            return 0;

        //Handle change of directory command
        if(strcmp(cmdList[0], "cd") == 0){
            returnVal = chdir(cmdList[1]);
            if(returnVal != 0){
                returnErrorCode = errno;
                printf("Failed to change directory to: %s. Return code is %d\n", cmdList[0], returnErrorCode);
                perror("ERROR");
            }
        }

        //Handle printing of command history
        if(strcmp(cmdList[0], "history") == 0)
            printCommandHistory();

        childPid = fork();
        if(childPid == -1){
            returnErrorCode = errno;
            printf("Failed to fork a new process. Return code: %d\n", returnErrorCode);
            perror("ERROR");
            exit(1);
        } 
        else if (childPid == 0)
        {
            //Have the child process execute the program
            if(strcmp(cmdList[0], "history") != 0){
                exeCmdReturn = execvp(cmdList[0], cmdList);
                if(exeCmdReturn == -1){
                    perror("ERROR");
                    exit(1);
                }
            }
        }
        else {
            //The parent process will wait for the child to complete if the parentProcessWaitFlag is set to TRUE
            if(parentProcessWaitFlag == TRUE){
                waitReturn = waitpid(childPid, &status, 0);
                if(waitReturn == -1){
                    returnErrorCode = errno;
                    printf("Failed to wait for the child process. Return code: %d\n", returnErrorCode);
                    perror("ERROR");
                }
                while(waitReturn != childPid);
            }
        }
    }
    return 0;
}

int parsecmd(const char *cmdline, char *arglist[]) 
{
  static char array[MAXLINE]; /* holds local copy of command line */
  char *buf = array;          /* ptr that traverses command line */
  char *delim;                /* points to first space delimiter */
  int argc;                   /* number of args */
  
  strncpy(buf, cmdline, strlen(cmdline)+1);
  buf[strlen(buf)-1] = ' ';  /* replace trailing '\n' with space */
  while (*buf && (*buf == ' ')) /* ignore leading spaces */
    buf++;

  /* Build the arglist list */
  argc = 0;
  // based on delimiter " "(space), separate the commandline into arglist
  while ((delim = strchr(buf, ' ')) && (argc < MAXARGS - 1)) {
    arglist[argc++] = buf;
    *delim = '\0';
    buf = delim + 1;
    while (*buf && (*buf == ' ')) /* ignore spaces */
      buf++;

    //If the & symbol is found in the code, set the parentProcessWaitFlag to FALSE
    if ((delim = strchr(buf, '&')) != NULL)
    {
        parentProcessWaitFlag = FALSE;    
    }

    //If the pipe symbol is found, handle pipe creations
    if ((delim = strchr(buf, '|')) != NULL)
    {
        handleCommandPipe(cmdline);
    }
  }
  arglist[argc] = NULL;
  return argc;
}

void buildCommandHistory(char command[]){
    if(counter > HISTSIZE){
        //clear the history once the max history size has been reached
        deleteCommandHistory();
        return;
    }

    struct COMMAND_HISTORY *commandNode = (struct COMMAND_HISTORY*)malloc(sizeof(struct COMMAND_HISTORY));
    strncpy(commandNode->command, command, strlen(command));
    commandNode->next = headListNode;
    headListNode = commandNode;
    counter++;
}

void deleteCommandHistory(){
    if(counter <= 0){
        return;
    }
    while(headListNode != NULL){
        headListNode = headListNode->next;
        counter--;
    }
}

void printCommandHistory(){
    struct COMMAND_HISTORY *currentCommand = (struct COMMAND_HISTORY*)malloc(sizeof(struct COMMAND_HISTORY));
    currentCommand = headListNode;

    while(currentCommand != NULL){
        printf("%s", currentCommand->command);
        currentCommand = currentCommand->next;
    }
}

int handleCommandPipe(const char *commandLineBuffer){
    int fd[2];
    pid_t pid;
    char *pCommand[MAXLINE], *childCmd[MAXLINE];
    char *bufptr, *childProcessCmdToken;
    int returnVal;

    //Copy the command passed in into the pCommand array
    strcpy(pCommand, commandLineBuffer);
    
    //Tokenize/Split the copied command string 
    childProcessCmdToken = strtok(pCommand, "|");
    if(childProcessCmdToken != NULL){
        childProcessCmdToken =strtok(NULL, "");
    }

    //Copy the second command into the childCmd buffer
    strncpy(childCmd, childProcessCmdToken, strlen(childProcessCmdToken)+1);
    printf("Child command is: %s\n", childProcessCmdToken);

    //After tokenizing the copied command string, the array should only contain parent command
    printf("Parent command is: %s\n", pCommand);

    //create the pipe
    returnVal = pipe(fd);
    if(returnVal == -1){
        //perror("ERROR");
        exit(-1);
    }

    //create a child process
    pid = fork();
    if (pid == -1)
    {
        perror("ERROR");
        exit(-1);
    }
    else if (pid == 0)
    {
        //child process will read stdout of parent process as stdin
        close(fd[1]); //close write end of the pipe
        dup(fd[1]);
        close(fd[0]);
        close(fd[1]);
        execvp(childCmd[0], childCmd);

    }
    else{
        //parent process will write to stdout
        close(fd[0]); //close read end of the pipe
        dup(fd[0]);
        close(fd[0]);
        close(fd[1]);
        execvp(pCommand[0], pCommand);
    }
    return 0;
}
