// The MIT License (MIT)
//
// Copyright (c) 2024 Trevor Bakker
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

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <ctype.h>

#define WHITESPACE " \t\n" // We want to split our command line up into tokens
                           // so we need to define what delimits our tokens.
                           // In this case  white space
                           // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255 // The maximum command-line size

#define MAX_NUM_ARGUMENTS 32

void errorFun();
void builtInCommands(char **token, int token_count);
void otherCmds(char **token, int token_count);
char *cmdPath(char *cmd);
void handleRedir(char **token, int token_count, int redrtSymPosi, char *fileName);

void errorFun()
{
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

void builtInCommands(char **token, int token_count)
{
    // token_count >= 2 means there is something else as well along with the command
    if (strcmp(token[0], "exit") == 0)
    {
        if (token_count >= 2)
        {
            errorFun();
        }
        else
        {
            exit(0);
        }
    }
    // token_count >= 2 means there is something else as well along with the command
    if (strcmp(token[0], "quit") == 0)
    {
        if (token_count >= 2)
        {
            errorFun();
        }
        else
        {
            exit(0);
        }
    }
    // token_count > 2 means there is something else as well along with the directory
    if (strcmp(token[0], "cd") == 0)
    {
        if (token_count == 2)
        {
            if (chdir(token[1]) == -1)
            {
                errorFun();
            }
        }
        else
        {
            errorFun();
        }
    }
}

void otherCmds(char **token, int token_count)
{
    int redrtSymPosi = -1;

    for (int i = 0; i < token_count; i++)
    {
        // Check if there is no command
        if (strcmp(token[0], ">") == 0)
        {
            errorFun();
            exit(0);
        }

        // Try to find the > so we know its a redirection command
        if (strcmp(token[i], ">") == 0)
        {
            // This means we are at the end of the token list max i will execute the statement
            // As there 1 extra token and i is the Idx of >
            if (token[i] == NULL)
            {
                break;
            }

            // This means no filename given ; > by itself or Ex. "ls >"
            if (i == token_count - 1)
            {
                errorFun();
                exit(0);
            }
            // This means there exits a entry after the fileName, hence invalid input with
            // two fileNames
            // i is the Idx of >
            /*Ex.   ls      >           a.1     b.2
              Idx   0       1(i)        2       3(i+2)
              Count 1       2           3       4(count)
            */
            if (token_count > (i + 2))
            {
                errorFun();
                exit(0);
            }
            redrtSymPosi = i;
            break;
        }
    }

    // If the Symbol position is not -1, means we have a redirection command
    if (redrtSymPosi != -1)
    {
        token[redrtSymPosi] = NULL;
        handleRedir(token, token_count, redrtSymPosi, token[redrtSymPosi + 1]);
    }
    // Else stay in the terminal and run as normal with the given input
    else
    {
        pid_t pid = fork();

        // Child Process
        if (pid == 0)
        {
            char *cmd = token[0];

            // Array to hold the command and its arguments
            char *args[MAX_NUM_ARGUMENTS];
            for (int i = 0; i < token_count; i++)
            {
                args[i] = token[i];
            }

            // Make it NULL terminated for execvp
            args[token_count] = NULL;
            // Try to execute the command
            if (execvp(cmd, args) == -1)
            {
                errorFun();
                exit(1);
            }
        }
        // Parent process
        else if (pid > 0)
        {
            // Wait for the child process to finish
            int status;
            waitpid(pid, &status, 0);
        }
        // Fork failed
        else
        {
            perror("Fork Failed!\n");
        }
    }
}

void handleRedir(char **token, int token_count, int redrtSymPosi, char *fileName)
{
    pid_t pid = fork();

    // Child process
    if (pid == 0)
    {
        FILE *fp = fopen(fileName, "w");
        if (fp == NULL)
        {
            errorFun();
            exit(1);
        }

        int fd = fileno(fp);

        // dup takes the opened file and writes what goes to the terminal
        // into the file. 1 is used as its second parameter as 1 is a define
        // value for what goes to std I/O
        if (dup2(fd, 1) == -1)
        {
            errorFun();
            fclose(fp);
            exit(1);
        }

        // dup takes the opened file and writes what goes to the error channel
        // into the file. 2 is used as its second parameter as 2 is a define
        // value for what goes to std error
        if (dup2(fd, 2) == -1)
        {
            errorFun();
            fclose(fp);
            exit(1);
        }

        // Make a copy to pass to execv as it may modify it
        char *args[MAX_NUM_ARGUMENTS];
        for (int i = 0; i < token_count; i++)
        {
            args[i] = token[i];
        }

        fclose(fp);

        // Make it NULL terminated for execv
        args[token_count] = NULL;
        // Try to execute the command
        if (execvp(token[0], args) == -1)
        {
            errorFun();
            exit(EXIT_FAILURE); // Exit child process on failure
        }
    }
    // Parent process
    else if (pid > 0)
    {
        int status;
        waitpid(pid, &status, 0);
    }
    // Fork fails
    else
    {
        perror("Fork Failed!\n");
    }
}

int main(int argc, char *argv[])
{
    ////////////////////  BATCH MODE    ////////////////////
    if (argc == 2)
    {
        // File handling code
        FILE *inputFileH = fopen(argv[1], "r");
        if (inputFileH == NULL)
        {
            errorFun();
            exit(1);
        }

        // Token string ; raw input
        char *cmdStr = (char *)malloc(MAX_COMMAND_SIZE);

        while (inputFileH != NULL)
        {
            if (fgets(cmdStr, MAX_COMMAND_SIZE, inputFileH) == NULL)
            {
                break;
            }

            // Skip leading spaces/tabs
            cmdStr += strspn(cmdStr, WHITESPACE);

            // If the command is empty after trimming, continue to next iteration
            if (strlen(cmdStr) == 0)
            {
                continue;
            }
            else
            {
                int len = strlen(cmdStr);
                // Iterate from the end of the string until we see a charater other than
                // the ones mentioned below and put each one as null
                for (int i = len - 1; i >= 0; i--)
                {
                    // Already null terminated
                    if (cmdStr[i] == '\0')
                    {
                        break;
                    }
                    else if ((cmdStr[i] == ' ') || (cmdStr[i] == '\t') || (cmdStr[i] == '\n'))
                    {
                        cmdStr[i] = '\0';
                    }
                    else
                    {
                        break;
                    }
                }
            }

            /* Parse input */
            char *token[MAX_NUM_ARGUMENTS];

            int token_count = 0;

            // Pointer to point to the token
            // parsed by strsep
            char *argument_pointer;

            char *working_string = strdup(cmdStr);

            // we are going to move the working_string pointer so
            // keep track of its original value so we can deallocate
            // the correct amount at the end

            char *head_ptr = working_string;

            // Tokenize the input with whitespace used as the delimiter
            while (((argument_pointer = strsep(&working_string, WHITESPACE)) != NULL) &&
                   (token_count < MAX_NUM_ARGUMENTS))
            {
                token[token_count] = strndup(argument_pointer, MAX_COMMAND_SIZE);
                if (strlen(token[token_count]) == 0)
                {
                    token[token_count] = NULL;
                }

                token_count++;
            }

            // Check if it's a built-in command or other command
            // Call appropriate function
            if (token[0] != NULL)
            {
                if (strcmp(token[0], "exit") == 0 || strcmp(token[0], "quit") == 0 || strcmp(token[0], "cd") == 0)
                {
                    builtInCommands(token, token_count);
                }
                else
                {
                    otherCmds(token, token_count);
                }
            }

            free(head_ptr);
        }
    }
    // Checks if there are more than 1 file in the input
    else if (argc > 2)
    {
        errorFun();
        exit(1);
    }
    //////////////////// INTERACTIVE MODE ////////////////////
    else
    {
        // Tokenize the input and run interactive mode
        char *cmdStr = (char *)malloc(MAX_COMMAND_SIZE);

        while (1)
        {
            // Print out the msh prompt
            printf("msh> ");

            // Read the command from the commandi line.  The
            // maximum command that will be read is MAX_COMMAND_SIZE
            // This while command will wait here until the user
            // inputs something.
            while (!fgets(cmdStr, MAX_COMMAND_SIZE, stdin))
                ;

            // Skip leading spaces/tabs
            cmdStr += strspn(cmdStr, WHITESPACE);

            // If the command is empty after trimming, continue to next iteration
            if (strlen(cmdStr) == 0)
            {
                continue;
            }
            else
            {
                int len = strlen(cmdStr);
                for (int i = len - 1; i >= 0; i--)
                {
                    if (cmdStr[i] == '\0')
                    {
                        break;
                    }
                    else if ((cmdStr[i] == ' ') || (cmdStr[i] == '\t') || (cmdStr[i] == '\n'))
                    {
                        cmdStr[i] = '\0';
                    }
                    else
                    {
                        break;
                    }
                }
            }

            /* Parse input */
            char *token[MAX_NUM_ARGUMENTS];

            int token_count = 0;

            // Pointer to point to the token
            // parsed by strsep
            char *argument_pointer;

            char *working_string = strdup(cmdStr);

            // we are going to move the working_string pointer so
            // keep track of its original value so we can deallocate
            // the correct amount at the end

            char *head_ptr = working_string;

            // Tokenize the input with whitespace used as the delimiter
            while (((argument_pointer = strsep(&working_string, WHITESPACE)) != NULL) &&
                   (token_count < MAX_NUM_ARGUMENTS))
            {
                token[token_count] = strndup(argument_pointer, MAX_COMMAND_SIZE);
                if (strlen(token[token_count]) == 0)
                {
                    token[token_count] = NULL;
                }

                token_count++;
            }

            // Check if it's a built-in command or other command
            // Call appropriate function
            if (token[0] != NULL)
            {
                if (strcmp(token[0], "exit") == 0 || strcmp(token[0], "quit") == 0 || strcmp(token[0], "cd") == 0)
                {
                    builtInCommands(token, token_count);
                }
                else
                {
                    otherCmds(token, token_count);
                }
            }

            free(head_ptr);
        }
        return 0;
    }
}
