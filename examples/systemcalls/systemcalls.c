#include "systemcalls.h"

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>


/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{
    return system(cmd) == 0 ? true : false;
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    // command[count] = command[count];
    
    pid_t child_pid = fork();
    
    // child branch
    if(child_pid == 0) {
        execv(command[0], command);
        perror("failed to execv");
        exit(EXIT_FAILURE); //only reached in case of error
    }
    
    //parent branch
    if(child_pid == -1) {
        perror("fork failed");
        return false;
    }
    
    int wstatus = 0;
    pid_t result = waitpid(child_pid, &wstatus, 0);
    if(result <= 0 || WEXITSTATUS(wstatus) != 0)
        return false;


/*
 * TODO:
 *   Execute a system command by calling fork, execv(),
 *   and wait instead of system (see LSP page 161).
 *   Use the command[0] as the full path to the command to execute
 *   (first argument to execv), and use the remaining arguments
 *   as second argument to the execv() command.
 *
*/

    va_end(args);

    return true;
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    // command[count] = command[count];

    int fd = open(outputfile, O_WRONLY|O_TRUNC|O_CREAT, 0644);
    if(fd < 0) {
        perror("opening file"); 
        return false;
    }

    pid_t child_pid = fork();

    // child branch
    if(child_pid == 0) {
        if(dup2(fd, 1) < 0) {
            perror("dup failed");
            exit(EXIT_FAILURE);
        }
        close(fd);
        execv(command[0], command);
        perror("execv'ing failed");
        exit(EXIT_FAILURE); //only reached in case of error
    }

    close(fd);
    
    //parent branch
    if(child_pid == -1) {
        return false;
    }
    
    int wstatus = 0;
    pid_t result = waitpid(child_pid, &wstatus, 0);
    if(result <= 0 || WEXITSTATUS(wstatus) != 0)
        return false;



/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/

    va_end(args);

    return true;
}
