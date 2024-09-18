#include "systemcalls.h"

/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
 */
bool do_system(const char *cmd)
{
    if (cmd == NULL)
    {
        printf("do_system: Command is NULL\n");
        return false;
    }
    int ret = system(cmd);

    if (ret == -1)
    {
        perror("system()");
        return false;
    }
    if (WIFEXITED(ret))
    {
        int exit_status = WEXITSTATUS(ret);
        printf("do_system: Command exited with status: %d\n", exit_status);
        if (exit_status == 0)
            return true;
        else
            return false;
    }
    else
    {
        printf("do_system: Command did not terminate normally\n");
        return false;
    }
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
    char *command[count + 1];
    int i;
    for (i = 0; i < count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];
    va_end(args);

    printf("do_exec: Forking process\n");
    pid_t pid = fork();
    if (pid == -1)
    {
        // Fork failed
        printf("fork() FAILED\n");
        perror("fork()");
        return false;
    }
    else if (pid == 0)
    {
        printf("do_exec: In child process, executing command\n");
        // Check if the command is executable
        // if (access(command[0], X_OK) != 0) {
        //     printf("do_exec: COMMAND ACCESS ERROR\n");
        //     perror("access()");
        //     _exit(EXIT_FAILURE);
        // }
        execv(command[0], command);
        printf("do_exec: COMMAND ACCESS ERROR\n");
        perror("execv()");
        _exit(EXIT_FAILURE); // Exit child process if execv fails
    }
    else
    {
        int status;
        if (waitpid(pid, &status, 0) == -1)
        {
            printf("do_exec: waitpid() ERROR\n");
            perror("waitpid()");
            return false;
        }
        if (WIFEXITED(status))
        {
            int exit_status = WEXITSTATUS(status);
            printf("do_exec: Child exited with status: %d\n", exit_status);
            if (exit_status == 0)
            {

                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            printf("do_exec: Child did not terminate normally\n");
            return false;
        }
    }
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
    char *command[count + 1];
    int i;
    for (i = 0; i < count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];

    va_end(args);
    // Fork the process
    pid_t pid = fork();
    if (pid == -1)
    {
        // Fork failed
        perror("fork");
        return false;
    }
    else if (pid == 0)
    {

        int fd = open(outputfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1)
        {
            perror("open");
            _exit(EXIT_FAILURE);
        }

        if (dup2(fd, STDOUT_FILENO) == -1)
        {
            perror("dup2");
            close(fd);
            _exit(EXIT_FAILURE);
        }
        close(fd);
        if (access(command[0], X_OK) != 0)
        {
            printf("do_exec_redirect: COMMAND ACCESS ERROR\n");
            perror("access()");
            _exit(EXIT_FAILURE);
        }
        execv(command[0], command);
        perror("execv");
        _exit(EXIT_FAILURE);
    }
    else
    {

        int status;
        if (waitpid(pid, &status, 0) == -1)
        {
            perror("waitpid");
            return false;
        }

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
        {
            printf("do_exec_redirect: Command executed successfully\n");
            return true;
        }
        else
        {
            printf("do_exec_redirect: Child exited with non-zero status or did not terminate normally\n");
            return false;
        }
    }
}
