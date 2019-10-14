#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>

#define min(a,b) (a>b?b:a)
#define buffer_size 65536
int transmission_type = 3;
char data[42 << 23];

typedef struct {
    long mtype;
    char mtext[buffer_size];
} message_buffer;


int size = 0;
int main() {
    int input = open("input.mp4", O_RDONLY);
    int buffer;
    while((buffer = read(input, &data[size], buffer_size)) > 0)
        size += buffer;
    close(input);
    srand(time(NULL));
    int key1 = rand(), key2 = rand();

    int pid = fork();
    int msgqid;
    char* data_shared;
    int shmid;
    switch(transmission_type) {
    case 1:    
        mknod("fifo", S_IFIFO | 0644, 0);
        if(pid) {
            int fd = open("fifo", O_WRONLY);
            for(int i = 0; i < size; i+=buffer_size)
                write(fd, &data[i], min(buffer_size, size - i));
            return 0;
        } else {
            for(int i = 0; i < size; i++)
                data[i] = 0;
            int fd = open("fifo", O_RDONLY);
            for(int i = 0; i < size; i+=buffer_size)
                read(fd, &data[i], buffer_size);
            break;
        }
    case 2:
        msgqid = msgget(key1, IPC_CREAT | 0644);
        message_buffer m_buffer;
        m_buffer.mtype = 5;
        if(pid) {
            for(int i = 0; i < size; i += buffer_size) {
                memcpy(m_buffer.mtext, &data[i], min(size - i, buffer_size));
                msgsnd(msgqid, &m_buffer, buffer_size, 0);
            }   
            return 0;
        } else {
            for(int i = 0; i < size; i++)
                data[i] = 0;
            for(int i = 0; i < size; i += buffer_size) {
                msgrcv(msgqid, &m_buffer, min(size - i, buffer_size), 5, 0);
                memcpy(&data[i], m_buffer.mtext, min(size - i, buffer_size));
            }            
            break;
        }
    case 3:
        shmid = shmget(key2, buffer_size + 1, IPC_CREAT | 0666);
        data_shared = shmat(shmid, NULL, 0);
        data_shared[buffer_size] = 0;
        if(pid) {
            for(int i = 0; i < size; i += buffer_size) {
                memcpy(data_shared, &data[i], min(size - i, buffer_size));
                data_shared[buffer_size] = 1;
                while(data_shared[buffer_size] != 0) usleep(1);
            }
            return 0;
        }
        else {
            for(int i = 0; i < size; i++)
                data[i] = 0;
            for(int i = 0; i < size; i += buffer_size) {
                while(data_shared[buffer_size] != 1) usleep(1);
                memcpy(&data[i], data_shared, min(size - i, buffer_size));
                data_shared[buffer_size] = 0;
            }
            break;
        }
    }
    int output = open("output.mp4", O_WRONLY | O_CREAT, 0644);
    for(int i = 0; i < size; i+=buffer_size)
        write(output, &data[i], min(buffer_size, size - i));
    return 0;
}