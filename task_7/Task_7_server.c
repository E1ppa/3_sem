#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/select.h>
#include <poll.h>
#include <string.h> 
//Needless comm
#define Server "fifo.txt"
#define cl_sz 5
#define file1 "file1"
#define file2 "file2"
#define file3 "file3"

typedef struct
{
	int cl_serv_fd;
	int serv_cl_fd;
} comm;

typedef struct
{
	int cl_numb;
	int request;
} Req;

comm* arr_com;
int* free_to_commun;    
struct pollfd clients_fds[cl_sz]; 
int Clients_Requests;

void *Trans(void* args)
{
	Req req_data = *(Req*)args;
	int cl_numb;
	int req;
			cl_numb = req_data.cl_numb;
			req = req_data.request;
			char FNM[10];
			switch(req)
			{
				case 1: strcpy(FNM, file1);break;
				case 2: strcpy(FNM, file2);break;
				case 3: strcpy(FNM, file3);break;
			}
			int file_descriptor = open(FNM, O_RDONLY);
			char send[1000];
			int size = read(file_descriptor, send, sizeof(send)-1);
			send[size] = '\0';
			char numb[5];
			sprintf(numb, "%d", cl_numb);
			mknod(numb, S_IFIFO | 0666, 0);
		        int temp_flow_fd = open(numb, O_WRONLY);


			sleep(10); 


			write(temp_flow_fd, send, strlen(send));
			close(temp_flow_fd);
}


int main()
{
	Clients_Requests=0;
	char buf[100];
	mknod(Server, S_IFIFO | 0666, 0);
	int server_fd = open(Server, O_RDONLY);
	
	arr_com = (comm*)malloc(sizeof(comm) * cl_sz);
	free_to_commun = (int*)malloc(sizeof(int) * cl_sz);



	struct pollfd Main[1];
        int timeout = 0;
        Main[0].fd = server_fd;
        Main[0].events = 0 | POLLIN;


	while(1)
	{
		int ret_Main = poll(Main, 1, timeout);
		if(ret_Main >0)
                {
			read(server_fd, buf, sizeof(buf));
			int end = 0;
			int i=1;
			while(!end)
			{
				int k=0;
				char forward_fd_name[100];
				while(buf[i]!='*')
					forward_fd_name[k++] = buf[i++];
				i++;
				forward_fd_name[k]='\0';
				k=0;
				char backward_fd_name[100];
				while(buf[i]!='*')
					backward_fd_name[k++] = buf[i++];
				backward_fd_name[k] = '\0';
				i++;
				
				
				comm new_client;
				mknod(forward_fd_name, S_IFIFO | 0666, 0);
			        new_client.cl_serv_fd = open(forward_fd_name, O_RDONLY);
				mknod(backward_fd_name, S_IFIFO | 0666, 0);
				new_client.serv_cl_fd = open(backward_fd_name, O_WRONLY);
				int found = 0;
				for(int j=0; j<cl_sz; j++)
				{
					if(free_to_commun[j] == 0)
					{
						found = 1;
						free_to_commun[j] = 1;
						arr_com[j] = new_client;
						clients_fds[j].fd = new_client.cl_serv_fd;
						clients_fds[j].events = 0|POLLIN;
						char response_buf[100];
                                        	strcpy(response_buf, "Connected");
						write(new_client.serv_cl_fd, response_buf, strlen(response_buf)+1);
						break;
					}
				}
				if(!found)
				{
					char response_buf[100];
					strcpy(response_buf, "No free sockets");
					write(new_client.serv_cl_fd, response_buf, strlen(response_buf)+1);
					close(new_client.cl_serv_fd);
					close(new_client.serv_cl_fd);
				}

				

				if(strlen(buf) == i)
					end = 1;
				else
					i++;
			}
                }
	        int ret_Clients = poll(clients_fds, cl_sz, timeout);
		if (ret_Clients > 0)
		{
			for(int cl_numb=0; cl_numb < cl_sz; cl_numb++)	
			{
				if(clients_fds[cl_numb].revents & POLLIN)
				{
					char Message[100];
					read(clients_fds[cl_numb].fd, Message, 100);
					int n = 1;
						char token[100];	
						while(Message[n] != '*')
						{
							token[n-1] = Message[n];
							n++;
						}
						token[n-1] = '\0';
						if(strcmp(token, "exit") == 0)
						{
							free_to_commun[cl_numb] = 0;
							close(arr_com[cl_numb].cl_serv_fd);
							close(arr_com[cl_numb].serv_cl_fd);
							clients_fds[cl_numb].fd=0;
						}
						else if(strcmp(token, "file1")==0)
						{
                                                        Req* req = malloc(sizeof(req));
                                                        req->cl_numb = cl_numb;
                                                        req->request = 1;
														pthread_create(malloc(sizeof(pthread_t)), NULL, Trans, req);
                                                        char numb[5];
                                                        sprintf(numb, "%d", cl_numb);
                                                        write(arr_com[cl_numb].serv_cl_fd,numb, sizeof(numb));

						}
						else if(strcmp(token, "file2")==0)
                                                {
                                                        Req* req = malloc(sizeof(req));
                                                        req->cl_numb = cl_numb;
                                                        req->request = 2;
														pthread_create(malloc(sizeof(pthread_t)), NULL, Trans, req);
                                                        char numb[5];
                                                        sprintf(numb, "%d", cl_numb);
                                                        write(arr_com[cl_numb].serv_cl_fd,numb, sizeof(numb));

                                                }
						else if(strcmp(token, "file3")==0)
												{
                                                        Req* req = malloc(sizeof(req));
                                                        req->cl_numb = cl_numb;
                                                        req->request = 3;
														pthread_create(malloc(sizeof(pthread_t)), NULL, Trans, req);
                                                        char numb[5];
                                                        sprintf(numb, "%d", cl_numb);
                                                        write(arr_com[cl_numb].serv_cl_fd,numb, sizeof(numb));
                                                }

					}
				
			}
		}
	}
}
