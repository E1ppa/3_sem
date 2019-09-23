#include <unistd.h> 
#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

#define Meslen __UINT16_MAX__

void s(int** a, int** b)
{
    int* c = *a;
    *a = *b;
    *b = c;
}

typedef struct
{
  int* in;
  int* out;
} dpipe_t;
void out(dpipe_t* dpipe, char* M, __uint16_t len)
{
    write(dpipe->out[1], M, len);
}
int rec(dpipe_t* dpipe, char* M)
{
    return read(dpipe->in[0], M, Meslen);
}
void cleanup(dpipe_t* dpipe)
{
    close(dpipe->in[0]);
    close(dpipe->out[1]);
    free(dpipe->in);
    free(dpipe->out);
}

int is_al(int ppid, int cpid)
{
    if(cpid == 0)
        return ppid == getppid();
    else
        return waitpid(cpid, NULL, WNOHANG) == 0;
}

int main()
{
    dpipe_t dpipe;
    dpipe.in = malloc(2 * sizeof(int*));
    dpipe.out = malloc(2 * sizeof(int*));
    pipe(dpipe.in);
    pipe(dpipe.out);
    int cpid = fork();
    int ppid = getppid();
    if(cpid == 0)
        s(&dpipe.in, &dpipe.out);
    close(dpipe.in[1]);
    close(dpipe.out[0]);
    char* M = malloc(Meslen);
    char* Mb = malloc(Meslen);
        
    if(cpid == 0)
    {

        while(is_al(ppid, cpid))
        {
            int len = rec(&dpipe, M);

            if(len == 0) 
                continue;

            if(!strcmp(M,"deth"))
                break;

            sprintf(Mb, "recd M: %s; Length: %i", M, len);

            out(&dpipe, Mb, strlen(Mb) + 1);
        }
        cleanup(&dpipe);
        printf("ch is ded\n");
        return 0;
    }
    else
    {
        while(is_al(ppid, cpid))
        {
            printf("Message is: ");
            int len = strlen(fgets(M, Meslen, stdin));
            M[--len] = 0;
            if(!strcmp("exit",M))
                break;

            out(&dpipe, M, len + 1);
            sleep(1);
            len = rec(&dpipe, Mb);
            if(len)
                printf("Response is: %s\n", Mb);
            else
                printf("No Reponse\n");
        }
        cleanup(&dpipe);
        printf("parent terminated\n");
        return 0;
    }
}