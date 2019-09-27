#include <unistd.h> 
#include <stdio.h>
#include <string.h> 
#include <stdlib.h> 
#include <sys/wait.h>


int main() {
    int argM = sysconf(_SC_ARG_MAX); 
    char* cmd = malloc(argM); 
    
    while(1)
    {    
        printf("Enter command: ");
        int cmdl = strlen(fgets(cmd, argM, stdin));
        cmd[--cmdl] = '\0';
        if(!strcmp(cmd, "exit")) 
            return 0;
        int c_pid = fork();
        int st = c_pid;
        if(c_pid == 0)
        {
            int argN = (cmd[0] != ' ');

            for(int i = 0; cmd[i]; i++)
                if(cmd[i] == ' ' && cmd[i+1] != ' ' && cmd[i+1] != 0)
                    argN++;

            char** argv = malloc((argN+1) * sizeof(char*));

            char* name = strtok(cmd, " ");
            argv[0] = name;

            for(int i = 1; i < argN; i++) 
                argv[i] = strtok(NULL," ");
            argv[argN]=NULL;
            execvp(name, argv);
            return 42;
        }
        else
        {
            waitpid(c_pid, &st, 0);
            st = WEXITSTATUS(st);
        }
        if(st == 42)
            printf("Incorrect\n");
        else
            printf("Exit status: %d\n", st);
    }
    return 0;
}