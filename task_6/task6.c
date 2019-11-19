#define _GNU_SOURCE
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>

void daemon_routine(int out, const char* name) {
    FILE* fout = fdopen(out, "r");
    DIR *dir = opendir(name);
    if(dir) {
        char path[4096];
        char *end_ptr = path;
        strcpy(path, name);
        
        end_ptr = &path[strlen(path)]; 
        *(end_ptr++) = '/';
          
        struct dirent *e;
        while (e = readdir(dir)) { 
            char* dname = e -> d_name;
            if(e->d_name[0] == '.' || !strcmp("backup", e->d_name))
                continue;
            strcpy(end_ptr, e -> d_name);

            char buff[8192];
            struct stat info;           
            stat(path, &info);
            if (S_ISDIR(info.st_mode)) {
                sprintf(buff, "backup/%s", path);
                mkdir(buff, 0755);
                daemon_routine(out, path);
            }
            else {
                sprintf(buff, "file \"%s\"\n", path);
                system(buff);
                dprintf(STDERR_FILENO, buff);
                fgets(buff,8191,fout);
                dprintf(STDERR_FILENO, buff);
                if(strstr(buff, "text") == NULL)
                    continue;
                
                sprintf(buff, "diff -s -q -N \"backup/%s\" \"%s\"\n", path, path);
                system(buff);
                dprintf(STDERR_FILENO, buff);
                fgets(buff,8191,fout);
                dprintf(STDERR_FILENO, buff);
                if(strstr(buff, "identical"))
                    continue;
                
                sprintf(buff, "cp \"%s\" \"backup/%s\"\n", path, path);
                system(buff);
                dprintf(STDERR_FILENO, buff);
                
                time_t now = time(0);
                struct tm tstruct;
                char buf[80];
                tstruct = *localtime(&now);
                strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
                sprintf(buff, "cp \"backup/%s\" \"backup/%s_%s\"\n", path, path, buf);
                system(buff);
                dprintf(STDERR_FILENO, buff);
            }
        }
    }    
}

int main() {
    if(fork())
        return 0;
    printf("Starting the daemon. PID: %d\n", getpid());
    
    mkdir("backup", 0755);
    errno = 0;

    setsid();
    
    sigset_t set;
    sigfillset(&set);
    sigprocmask(SIG_BLOCK, &set, NULL);
    
    umask(0022);
    
    int p[2];
    pipe(p);
    close(STDIN_FILENO);
    dup2(p[1], STDOUT_FILENO);    
    if(errno || (dup2(open("backup/daemon.log", O_CREAT | O_WRONLY | O_APPEND, 0644), STDERR_FILENO) && errno)) {
        dprintf(STDERR_FILENO, "ERROR! errno == %d\n", errno);
        return 0;
    }
    

    while(1) {
        daemon_routine(p[0], ".");
        sleep(10);
    }
}
