#define _GNU_SOURCE
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#define min(a,b) (a < b ? a : b)

const char* inputfile = "task_5.c"; //source
const char* outputfile = "copy.c"; //destination

unsigned char* data = NULL;
int size = 0;
int bitnum = 0;
int pid = 0;
int flag = 1;
void sigf(int ind) {
    flag = 1;
}
void sig1(int ind) {
    if(++bitnum == 8) {
        bitnum = 0;
        size++;
    }
    kill(pid, SIGUSR1);
}
void sig2(int ind) {
    data[size] |= 1 << bitnum;
    sig1(ind);
}
void sigi(int ind) {
    int output = open(outputfile, O_CREAT | O_WRONLY, 0644);
    for(int i = 0; i < size; i += 0x4000)
        write(output, &data[i], min(0x4000, size - i));
    close(output);
    exit(0);
}

int main() {
    data = malloc(0x1000000);
    memset(data, 0, 0x1000000);
    pid = fork();
    if(pid) {
        int input = open(inputfile, O_RDONLY);
        for(int delta = 0; (delta = read(input, &data[size], 0x4000)) > 0; size += delta);
        signal(SIGUSR1, sigf);
        sleep(1);
        for(int i = 0; i < size; i++)
            for(int j = 0; j < 8 ; j++) {
                flag = 0;
                if((data[i] >> j) & 1)
                    kill(pid, SIGUSR2);
                else
                    kill(pid, SIGUSR1);
                while(!flag) usleep(1);
            }
        kill(pid, SIGINT);
    } else {
        pid = getppid();
        signal(SIGUSR1, sig1);
        signal(SIGUSR2, sig2);
        signal(SIGINT, sigi);
        while(1) usleep(1);
    }
    return 0;
}