// The MIT License (MIT)
// 
// Copyright (c) 2016, 2017, 2021 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.


/*
Name: Biraj Singh G C
ID: 1001867347
*/



#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 10    // Mav shell only supports ten arguments

#define MAX_RECORD_SIZE 15      // The maximum pid and command stored in history

#define false 0                 // Boolean value for false  

#define true 1                  // Boolean value for true


/*FUNCTION DECLATARIONS*/

/*Record the command line*/
void record_history(char command_history[MAX_RECORD_SIZE][MAX_COMMAND_SIZE],
                    int* history_index, char* command_input );

/*Record the pid of process created*/
void record_pid(int pid_history[MAX_RECORD_SIZE], int* pid_index, int pid );

/*Print the command history*/
void print_history(char command_history[MAX_RECORD_SIZE][MAX_COMMAND_SIZE],
                   int* history_index);

/*Print the pid of process created*/
void print_pids(int pid_history[MAX_RECORD_SIZE],
                int* pid_index);

/*Acces command from history*/
void acess_history(char command_history[MAX_RECORD_SIZE][MAX_COMMAND_SIZE],
                   int *history_accessed, char* cmd_str, int* history_index);

/*Change the Directory*/
void change_directory(char *dir_path);                  

/*MAIN FUNCTION*/
int main(){ 

  /*VARIABLE DECLARATIONS*/

  //List to store history of command line
  char command_history[MAX_RECORD_SIZE][MAX_COMMAND_SIZE]; 

  //List to store the history of pid of processes executed from command line
  int pid_history[MAX_RECORD_SIZE];

  //Track the position of the command_history
  //Helps to find the next position to store the command
  //initialize it to 0, since no history has been stored yet
  int history_index =0;

  //Track the position of the pid_history
  //Helps to find the next position to store the pid of process
  //initialize it to 0, since no process has been created yet
  int pid_index = 0;

  //boolean value to check if the history was acessed
  int history_accessed = false;

  // Char string to store the commands before parsing
  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
  
  //used to execute waitpid function.
  int status; 


  while( 1 ){

    //if history acessed, dont prompt the user
    //use the command from the command history
    if(!history_accessed){
      // Print out the msh prompt
      printf ("msh> ");

      // Read the command from the commandline.  The
      // maximum command that will be read is MAX_COMMAND_SIZE
      // This while command will wait here until the user
      // inputs something since fgets returns NULL when there
      // is no input
      while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );
    }
    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int   token_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *argument_ptr;                                         
                                                           
    char *working_str  = strdup( cmd_str );                

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input strings with whitespace used as the delimiter
    while ( ( (argument_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS)){

      token[token_count] = strndup( argument_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
        token[token_count] = NULL;
      
        token_count++;
    }
    
    /* SHELL FUNCTIONALITY */
  
    // Continue to print "msh" in new line and recive new input.  
    if(token[0] == NULL)
      continue;

    //STORE COMMAND INTO MEMORY
    //Use function Recordhistory();
    //only recored history if the command is not null of does not start with "!"
    //if command is not null, user has not typed anything
    //if command starts with !, the user has repeated the command 
    if(cmd_str[0] != '!')
      record_history(command_history, &history_index, cmd_str);
      
    else{
      //ACESS HISTORY
      //if user begins command with '!', means user wants to
      //use previous commands.
      //use function acess_history to acces the history of the user.
      acess_history(command_history, &history_accessed, cmd_str, &history_index);
      continue;
    }
    
    //EXIT OR QUIT MSH
    //Compare the token with key words "exit and quit"
    //return the program, if the true.
    if(strcmp(token[0], "exit") == 0 || strcmp(token[0], "quit") == 0)
      return 0;

    //PRINTING HISTORY
    //if the user commands "history"
    //use function print_history to print the last 15 history till date
    else if(strcmp(token[0], "history") == 0)
      print_history(command_history, &history_index);

    //PRINT PID HISTORY
    //if the user commands "showpids"
    //use function print_pids to print the last 15 history till date
    else if(strcmp(token[0], "showpids") == 0)
      print_pids(pid_history, &pid_index);

    //CHANGE DIRECTORY CD
    //if user commands cd 'path' change dir to path
    //if user commands cd .. move one dir back
    //use change_directory functon
    else if(strcmp(token[0], "cd") == 0)
      change_directory(token[1]);
    
    else{
      //SYSTEM CALLS
      //we will now implement exec calls
      //commands shall be searched in following order
      //Current working directory
      // /usr/local/bin
      // /usr/bin
      // /bin

      //Fork the process and store it in pid_t --> could land into 
      //child process or parent process   
      pid_t pid_0 = fork();

      //if pid_0 lands on child it will have a value of 0

      if(pid_0 == 0){
        //execute the exec function using arguments token[0] for the commands
        //token includes all the arguments following command
        //execvp takes the command and an array of argumens 
        //it returns -1 if it could not execute or invalid command
        if(execvp(token[0], token) == -1){
          //print the error and offer help 
          //perror is a global variable that stores the recent error
          perror(*token);
        }
        //make sure we kill runnin child process before we continue into the parent  
        return 0;
      }
      //else it will have the value of the child process
      else{

        //record the pid value.
        //if the pid is not zero than it will contain the pid for
        //the child process that is why we use parent process to store child pid.
        record_pid(pid_history, &pid_index, pid_0);

        //make sure child process executes first
        //wait pid waits for the input pid to execute
        waitpid(pid_0, &status, 0);
      }

    }
    //set hitory accesed to false to continue msh input
    history_accessed = false;
    free( working_root );
  }
  return 0;
}

/*FUNCTION DEFINITIONS*/

//Record the history of input of command line into command_history
//Keep track of history_index and increase it by one at the end of the function
void record_history(char command_history[MAX_RECORD_SIZE][MAX_COMMAND_SIZE],
                    int* history_index, char* command_input ){
  //copy the command into history directly if history recorded is less thab max size                  
  if(*history_index < MAX_RECORD_SIZE)
    strcpy(command_history[*history_index], command_input);

  //move the history of records by one and add the new record at the end.
  else{
    //Say i is row of records and j is column of records.
    int i, j;
    for (i = 0; i < MAX_RECORD_SIZE; i++)
      for (j = 0; j < MAX_COMMAND_SIZE; j++)
        command_history[i][j] = command_history[i + 1][j];

    strcpy(command_history[MAX_RECORD_SIZE - 1], command_input);
  }

  //increase the tracker by one for next record.
  *history_index = *history_index + 1;
}


//Record the history of pid of processed created in msh
//Keep track of pid_index and increase it by one at the end of the function
void record_pid(int pid_history[MAX_RECORD_SIZE], int* pid_index, int pid ){

  //copy the pid into pid_history directly if pid created is less thab max size 
  if(*pid_index < MAX_RECORD_SIZE)
    pid_history[*pid_index] = pid;
  
  //move the pid record by one and add the new record at the end.
  else{
    int i;
    for (i = 0; i < MAX_RECORD_SIZE; i++)
    {
        pid_history[i] =  pid_history[i + 1];
    }
    pid_history[MAX_RECORD_SIZE - 1] = pid;
  }
  //increase the tracker by one for next record.
  *pid_index = *pid_index + 1;    
}

//print the history of the commands inputed.
//if the history if less the Max allowed it must only print whats in there
// if the history is greater than Max allowd, only print the max allowed
void print_history(char command_history[MAX_RECORD_SIZE][MAX_COMMAND_SIZE],
                  int* history_index){

  int i, max_index = *history_index;;

  //Make sure the history index is less than 15 before printing hstory
  if(*history_index > 15)
    *history_index = 15;

  for (i = 0; i < *history_index; i++)
    printf("%d: %s", i + 1, command_history[i]);
  
  //keep track of all the command executed
  *history_index = max_index;
}

//print the history of the pids of process created.
//if the history is less the Max allowed it must only print whats in there
//if the history is greater than Max allowd, only print the max allowed
void print_pids(int pid_history[MAX_RECORD_SIZE],
                  int* pid_index){

  int i, max_index = *pid_index;

  //Make sure the history index is less than 15 before printing hstory
  if(*pid_index > 15)
    *pid_index = 15;

  for (i = 0; i < *pid_index; i++)
    printf("%d: %d\n", i + 1, pid_history[i]);
  
  //keep track of all the process executed 
  *pid_index = max_index;
}

//parse the input begining with ! to find the index in history
//Use the comman_history array to find the index pointed command
//return if given index is greater than the record
//copy the record into cmd_str if the index is valid.
void acess_history(char command_history[MAX_RECORD_SIZE][MAX_COMMAND_SIZE],
                   int *history_accessed, char* cmd_str, int* history_index){

  //parse the string
  int index = atoi(&cmd_str[1]);


  //Invalid command if index is greater than the size of the 2d array
  if(index > MAX_RECORD_SIZE || index > *history_index || index <= 0){
    printf("Command not in history.\n");
    *history_accessed = false;
    return;
  }
  //if valid use the command by coppying it into cmd_str
  else
    strcpy(cmd_str, command_history[index-1]);

  //update the history accesed boolean value
  *history_accessed = true;
}

//Change the directory using chdir function
//Make sure empty path is not provided
//Prompt the user that directory is not found if given path is not valid
void change_directory(char *dir_path){

  //check if path is provided
  if(dir_path == NULL)
    printf("Invalid Directory Path.\n");

  //change the directory --> works with cd ..
  else if(chdir(dir_path) == -1)
    printf("%s: Directory not found.\n", dir_path);
}


