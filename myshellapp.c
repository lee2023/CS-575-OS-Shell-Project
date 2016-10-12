/* 
* Developed by: Rosemary Espinal
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

#define MAXLINE 1024
#define MAXARGS 20
#define HISTSIZE 10 //history size
#define MY_PROMPT "centos_shell> "

int parseCommand(const char *cmdline, char *listOfArgs[]);
void buildCommandHistory(char *command);
void printCommandHistory();

struct COMMAND_HISTORY{
    char *command;
    struct COMMAND_HISTORY *next;
};

int counter = 0;
struct COMMAND_HISTORY *headListNode;

int main(int argc, char *argv[]){
    pid_t childPid, parentPid, waitReturn;
    int returnErrorCode, exeCmdReturn, status, returnVal;
    char cmdline[MAXLINE], *cmdList[MAXARGS];

    parentPid = getpid();
    printf("The parent process pid is: %d\n", parentPid);

    //Start out with an history list for the new shell
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
            printf("Try again, no commands were entered!");
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
            exeCmdReturn = execvp(cmdList[0], cmdList);
            if(exeCmdReturn == -1){
                returnErrorCode = errno;
                printf("Failed to execute the program. Return code: %d\n", returnErrorCode);
                perror("ERROR");
                exit(1);
            }
        }
        else {
            //The parent process will wait for the child to complete
            waitReturn = waitpid(childPid, &status, 0);
            if(waitReturn == -1){
                returnErrorCode = errno;
                printf("Failed to wait for the child process. Return code: %d\n", returnErrorCode);
                perror("ERROR");
            }
            while(waitReturn != childPid);
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
  int bg;                     /* background job? */
  
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

//TODO: Figure out how to run a process in the background
    if ((delim = strchr(buf, '&')) != NULL)
    {
        printf("Found %c in the command to run as a background process.\n", delim);    
    } 
  }

  arglist[argc] = NULL;
  return argc;
}

void buildCommandHistory(char *command){
    if(counter > HISTSIZE){
        printf("Maximum history size (%d) has been reached.", HISTSIZE);
        return;
    }

    char bufferCmd[MAXLINE];
    strncpy(bufferCmd, command, strlen(command)+1);

    struct COMMAND_HISTORY *commandNode;
    commandNode = (struct COMMAND_HISTORY*)malloc(sizeof(struct COMMAND_HISTORY));
    commandNode->command = bufferCmd;
    commandNode->next = headListNode;
    headListNode = commandNode;
    counter++;
}

void printCommandHistory(){
    struct COMMAND_HISTORY *currentCommand;
    currentCommand = (struct COMMAND_HISTORY*)malloc(sizeof(struct COMMAND_HISTORY));
    currentCommand = headListNode;

    while(currentCommand != NULL){
        printf("%s", currentCommand->command);
        currentCommand = currentCommand->next;
    }
}
