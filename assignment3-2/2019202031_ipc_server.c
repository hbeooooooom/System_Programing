///////////////////////////////////////////////////////////////////////////////////////////////////
// File Name     : 2019202031_ipc_server.c                                     //
// Date          : 2023/05/22                                            //
// OS            : Ubuntu 16.04 LTS 64bits                                   //
// Author    : Jang hyung beom                                            //
// Student ID     : 2019202031                                            //
//-----------------------------------------------------------------------------------------------//
// Title : System Programming Assignment #3-2                                   //
// Description : shared memory                                //
///////////////////////////////////////////////////////////////////////////////////////////////////

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/stat.h>
#include <time.h>
#include <grp.h>
#include <glob.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<fnmatch.h>
#include<sys/wait.h>
#include<unistd.h>
#include <netdb.h>
#include <signal.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#define MAX_HISTORY 10
#define PORTNO 40000
#define BUFSIZE 1024
#define URL_LEN 256
int root_path_check = 1;
int a = 0;
int numRequests = 0;
pid_t *pids;
pid_t parent_pid;
int check_404(char* dirpath);
int request_no = 1;
int shm_id;
pthread_t tidA; // Thread Variable Declaration
int addrlen;
int socket_fd;
void sorting(char **filenames, int num_files, int r, int S, char *dirpath);
void print_file_info(int h, int total, mode_t mode, struct dirent *dp, char *dir_path, int lflag, char **filenames, int num_files, int client_fd,char* url, char* IP);
void ls(char *dirpath, int a, int l, int r, int h, int S, int client_fd, char* url, char* IP);
void no_dir(struct stat *sb, char *filename, int client_fd);
void wild_card(int argc, char **argv, int client_fd);
char* check_filetype(mode_t mode);
int is_ip_allowed(const char* client_ip);
void printHistory();
void addHistory(char *ip, int port, pid_t pid);
pid_t child_make(int i, int socketfd, int addrlen);
void child_main(int i, int socket_fd, int addrlen);
void *doit1(void *vptr);//
void *doit2(void *vptr);
void *doit3(void *vptr);
void *doit4(void *vptr);
void *doit5(void *vptr);
void *doit6(void *vptr);
void *doit7(void *vptr);
void *doit8(void *vptr);
void *doit_disconnect(void *vptr);
void *doit_dec(void *vptr);//
void *doit_nothing(void *vptr);
struct ClientInfo { //client information structure
    int number;
    char ip[16];
    int port;
    pid_t pid;
    time_t time;
};

typedef struct ClientInfo_list //shared structure
{
    int connect_process; //연결된 수
    int IDLE_process; //놀고있는 자식 수
    int total_accept; //총 몇번 연결되었는지 (출력때 연결된 순서로 사용)
    int process_counter; //프로세스가 현재 몇갠지
    struct ClientInfo client_info_list[10]; //히스토리 구조체 배열
    pid_t Idle_pid_list[10]; //놀고있는 자식 번호
} ClientInfo_list;
ClientInfo_list* shared_memory;

int MaxChilds=0, MaxIdleNum=0, MinIdleNum=0, StartProcess=0, MaxHistory=0;
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

///////////////////////////////////////////////////////////////////////////////////////////////////
// alarmHandler                                     //
//===============================================================================================//
// Input :  int signum -> signal                                                       //
// Output : print value                                  //
// Purpose : handler                                       //
///////////////////////////////////////////////////////////////////////////////////////////////////
void alarmHandler(int signum)
{
    int shm_id, val;
    void *shm_addr;
    if((shm_id = shmget((key_t)PORTNO, sizeof(struct ClientInfo), IPC_CREAT | 0666)) == -1) //Create shared memory
    {
        printf("shmget 초기화 fail"); //Output an error statement if there is a shared memory error
    }
    pid_t a = 0;
    shared_memory = (ClientInfo_list*)shmat(shm_id,NULL,0); //Connecting Shared Memory
    if (signum == SIGINT) // if parent process and signal is SIGINT
    {

        pthread_create(&tidA, NULL, doit5, NULL); //쓰레드 생성
        pthread_join(tidA, NULL); //쓰레드 종료까지 대기

        time_t curtime = time(NULL);                           // get time
        char *curtime_str = ctime(&curtime);                   // get time
        curtime_str[strcspn(curtime_str, "\n")] = '\0';        // Replace open characters with null characters
        printf("\n[%s] Server is terminated.\n", curtime_str); // print
        exit(0);                                               // parent process termination
    }
    else if (signum == SIGCHLD) //if signal is SIGCHLD
    {
        pid_t child_pid;                                  // child pid variable
        int status;                                       // status
        while ((child_pid = waitpid(-1, &status, 0)) > 0) // retrieve one's child
        {
        }
    }
    else if(signum == SIGUSR1) //if signal is SIGUSR1
    {
        for(int i = 0; i<MaxChilds; i++) //MaxChilds is 10
        {
            if(pids[i] == 0) //empty pid array 
            {
                pids[i] = child_make(i, socket_fd,addrlen); //Store child pid in empty space
                break;
            }
        }
        for(int i = 0; i<MaxChilds; i++) //MaxChilds is 10
        {
            if(pids[i] == 0) //empty pid array 
            {
                pids[i] = child_make(i, socket_fd,addrlen); //Store child pid in empty space
                break;
            }
        }
    }
    else if(signum == SIGUSR2) //if signal is SIGUSR2
    {
        pid_t k, kk;
        for(int i = 0; i< MaxChilds; i++)
        {
            if(pids[i] != 0)
            {
                for(int j = 0; j< MaxChilds; j++)
                {
                    if(pids[i] == shared_memory->Idle_pid_list[j])
                    {
                        time_t curtime = time(NULL);                           // get time
                        char *curtime_str = ctime(&curtime);                   // get time
                        curtime_str[strcspn(curtime_str, "\n")] = '\0';        // Replace open characters with null characters
                        pthread_create(&tidA, NULL, doit7, NULL); //쓰레드 생성
                        pthread_join(tidA, NULL); //쓰레드 종료까지 대기
                        printf("\n[%s] IdleProcessCount : %d\n", curtime_str, shared_memory->IDLE_process); //Idle process print
                        printf("[%s] %d Process is terminated.\n", curtime_str, pids[i]);
                        pthread_create(&tidA, NULL, doit8, NULL); //쓰레드 생성
                        pthread_join(tidA, NULL); //쓰레드 종료까지 대기
                        k = pids[i];
                        
                        pids[i] = 0;
                        break;
                    }
                }
                break;
            }
        }

        for(int i = 0; i< MaxChilds; i++)
        {
            if(pids[i] != 0)
            {
                for(int j = 0; j< MaxChilds; j++)
                {
                    if(pids[i] == shared_memory->Idle_pid_list[j])
                    {
                        time_t curtime = time(NULL);                           // get time
                        char *curtime_str = ctime(&curtime);                   // get time
                        curtime_str[strcspn(curtime_str, "\n")] = '\0';        // Replace open characters with null characters
                        pthread_create(&tidA, NULL, doit7, NULL); //쓰레드 생성
                        pthread_join(tidA, NULL); //쓰레드 종료까지 대기
                        printf("\n[%s] IdleProcessCount : %d\n", curtime_str, shared_memory->IDLE_process); //Idle process print
                        printf("[%s] %d Process is terminated.\n", curtime_str, pids[i]); 
                        pthread_create(&tidA, NULL, doit8, NULL); //쓰레드 생성
                        pthread_join(tidA, NULL); //쓰레드 종료까지 대기
                        kk = pids[i]; 
                        
                        pids[i] = 0;
                        break;
                    }
                }
                break;
            }
        }
        kill(k,SIGTERM); //SIGTERM call
        kill(kk, SIGTERM);

    }
    else if (signum == SIGALRM) // if parent process alarm
    {
        printf("============ Connection History ===========================================\nNo.\tIP\t\tPID\tPORT\tTIME\n");
        printHistory(); //call print function
        printf("===========================================================================\n\n");
        
    }
    alarm(10); // set alarm time
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// child_alarmHandler                                     //
//===============================================================================================//
// Input :  int signum -> signal                                                       //
// Output : print value                                  //
// Purpose : handler                                       //
///////////////////////////////////////////////////////////////////////////////////////////////////
void child_alarmHandler(int signum)
{
    if(signum == SIGTERM) //child process exit
    {
        
        pthread_create(&tidA, NULL, doit6, NULL); //쓰레드 생성
        pthread_join(tidA, NULL); //쓰레드 종료까지 대기
            exit(0);
    }
}

int main(int argc, char **argv)
{

    FILE *file; //FILE variable
    char line[100]; //char variable
    char key[100]; //char variable
    int value; //int variable

    file = fopen("httpd.conf", "r"); //httpd.conf file open

    if(file == NULL) //if file no exist
    {
        printf("httpd 파일이 존재하지 않습니다.\n"); //print exist
    }
    while (fgets(line, sizeof(line), file)) //Import Line by Line
    {
        sscanf(line, "%[^:]: %d", key, &value); //Share with key and value

        if (strcmp(key, "MaxChilds") == 0) //if key value is MaxChilds
        {
            MaxChilds = value;
        }
        else if (strcmp(key, "MaxIdleNum") == 0)//if key value is MaxIdleNum
        {
            MaxIdleNum = value;
        }
        else if (strcmp(key, "MinIdleNum") == 0)//if key value is MinIdleNum
        {
            MinIdleNum = value;
        }
        else if (strcmp(key, "StartProcess") == 0)//if key value is StartServers
        {
            StartProcess = value;
        }
        else if (strcmp(key, "MaxHistory") == 0)//if key value is MaxHistory
        {
            MaxHistory = value;
        }

    }
    fclose(file); //close file


    parent_pid = getpid();
    signal(SIGINT, alarmHandler); //signal setting SIGINT
    signal(SIGCHLD, alarmHandler);//signal setting SIGCHLD
    signal(SIGALRM, alarmHandler); //signal setting SIGALRM
    signal(SIGUSR1, alarmHandler); //signal setting SIGUSR1
    signal(SIGUSR2, alarmHandler); //signal setting SIGUSR2
    alarm(10); //set alarm time
    struct sockaddr_in server_addr, client_addr; //struct variable
    int client_fd;                    //int variable
    int len, len_out;                            //int variable
    int opt = 1;                                 //int variable
    if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) //socket call
    {
        printf("Server : can't open stream socket\n");
        return 0;
    }
    time_t curtime;
    curtime = time(NULL);
    char* curtime_str = ctime(&curtime);
    curtime_str[strcspn(curtime_str, "\n")] = '\0'; 
    printf("[%s] Server is started.\n", curtime_str);

    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); //setsockopt function call
    memset(&server_addr, 0, sizeof(server_addr)); //initalize
    server_addr.sin_family = AF_INET; //setting
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);//setting
    server_addr.sin_port = htons(PORTNO);//setting

    if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)//bind call
    {
        printf("Serve err\n");
        return 0;
    }

    listen(socket_fd, 10); //listen
    pids = (pid_t*)malloc(10*sizeof(pid_t));
    addrlen = sizeof(client_addr);
    pthread_create(&tidA, NULL, &doit1, NULL); //create thread
    pthread_join(tidA, NULL); //wait thread
    for(int i = 0; i<StartProcess; i++)
    {
        pids[i] = child_make(i,socket_fd, addrlen);
    }
    for(int i = 5; i<10; i++)
    {
        pids[i] = 0;
    }

    for(; ;)
        pause();

    close(socket_fd); // close socket
    return 0;         // Exit the program
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// child_make                                     //
//===============================================================================================//
// Input :  int i                                                       //
//          int socketfd -> socket number                               //
//          int addrlen -> address size                                 //
// Output : pid number                                  //
// Purpose : make child process                                       //
///////////////////////////////////////////////////////////////////////////////////////////////////
pid_t child_make(int i, int socketfd, int addrlen)
{
    pid_t pid;
    if((pid = fork()) > 0)
    return pid;

    pthread_create(&tidA, NULL, &doit2, NULL); //create thread
    pthread_join(tidA, NULL); //wait thread

    child_main(i, socketfd, addrlen);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// child_main                                     //
//===============================================================================================//
// Input :  int i                                                       //
//          int socketfd -> socket number                               //
//          int addrlen -> address size                                 //
// Output : x                                  //
// Purpose : child main code                                      //
///////////////////////////////////////////////////////////////////////////////////////////////////
void child_main(int i, int socket_fd, int addrlen)
{
    int client_fd, len_out;
    char buf[BUFSIZE]; //buf
    socklen_t chilen; //socklen_t variable
    struct sockaddr_in client_addr; //sockaddr_in variable
    signal(SIGINT, SIG_IGN); //signal setting
    signal(SIGTERM, child_alarmHandler); //signal setting

    shared_memory = (ClientInfo_list*)shmat(shm_id, NULL, 0); //공유 메모리 연결
    pthread_mutex_init(&counter_mutex, NULL); //쓰레드 초기화
    pthread_create(&tidA, NULL, &doit_nothing, NULL); //create thread
    pthread_join(tidA, NULL); //wait thread

    while(1)
    {
        
        struct in_addr inet_client_address; //struct variable
        char buf[BUFSIZE] ={0,}; //char variable
        char tmp[BUFSIZE] ={0,}; //char variable
        char response_header[BUFSIZE] ={0,}; //char variable
        char response_message[BUFSIZE] ={0,}; //char variable
        char save_message[BUFSIZE] ={0,}; //char variable
        char url[BUFSIZE] ={0,}; //char variable
        char method[20] ={0,}; //char variable
        char *tok = NULL;      // char pointer variable
        client_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &addrlen); // accept function call
        inet_client_address.s_addr = client_addr.sin_addr.s_addr;
        read(client_fd, buf, BUFSIZE);
        strcpy(tmp, buf);
        char dirpath[100];
        tok = strtok(tmp, " ");
        if (tok == NULL) // Ignore signal when mouse is raised
        {
            continue;
        }
        strcpy(method, tok);
        char *IP = NULL; // IP address variable
        char *IP1 = NULL;
        char *qqq;
        if (strcmp(method, "GET") == 0) // if method == GET
        {
            tok = strtok(NULL, " ");
            strcpy(url, tok);
            tok = strtok(NULL, "\n");
            strtok(qqq, " ");       //
            IP = strtok(NULL, ":"); // get ip address
        }
        int qu = 0;
       
        inet_client_address.s_addr = client_addr.sin_addr.s_addr;
        IP1 = inet_ntoa(inet_client_address); // IP 주소 가져오기
        printf("=============== New Client ===============\n");
        time_t curtime = time(NULL);                    // get time
        char *curtime_str = ctime(&curtime);            // get time
        curtime_str[strcspn(curtime_str, "\n")] = '\0'; // Replace open characters with null characters
        printf("[%s] \n", curtime_str);               // print
        printf("IP : %s\n", IP1);                       // print IP
        printf("Port : %d\n", client_addr.sin_port);    // print Port
        printf("==========================================\n\n");
        pthread_create(&tidA, NULL, doit3, NULL); //쓰레드 생성
        pthread_join(tidA, NULL); //쓰레드 값 가져올때까지 대기
        pthread_create(&tidA, NULL, doit_dec, NULL); //쓰레드 생성
        int *i;
        pthread_join(tidA, (void**)&i); //쓰레드 값 가져올때까지 대기
        int ii = *i; //가져온 값 넣기
        if(shared_memory->IDLE_process < MinIdleNum)//만약 3개 이하라면 fork해줌
        {
            kill(getppid(), SIGUSR1); //프로세스 생성
        }

        if (strcmp(url, "/") == 0)
        {
            root_path_check = 1; // if path is root directory
        }

        if (qu = is_ip_allowed(IP1)) // if not allowed ip
        {
            sprintf(response_message, "<link rel='icon' href='data:,'><h1>Access denied<br> Your IP : %s</h1><br> <h3>You have no permission to access this web server.<br> HTTP 403.6 - Forbidden: IP address reject</h3>", IP); // save message
            sprintf(response_header, "HTTP/1.1 403.6 OK\r\nContent-Type: text/html\r\nContent-Length:%d\r\n\r\n", 1000);                                                                                                          // save message
            write(client_fd, response_header, strlen(response_header));                                                                                                                                                           // write
            write(client_fd, response_message, strlen(response_message));
            close(client_fd);
        }
        else if (check_404(url) == 404) // if 404 error page
        {
            sprintf(response_message, "<link rel='icon' href='data:,'><h1>Not Found</h1><br> <h3>The request URL %s was not found on this server<br>HTTP 404 - Not Page Found</h3>", url); // save message
            sprintf(response_header, "HTTP/1.1 404 OK\r\nContent-Type: text/html\r\nContent-Length:%d\r\n\r\n", 10000);                                                                    // save message
            write(client_fd, response_header, strlen(response_header));                                                                                                                    // write
            write(client_fd, response_message, strlen(response_message));
            close(client_fd); // close accpet
        }
        else if (check_404(url) == 201) // 201 is file
        {
            if (fnmatch("*.jpg", url, FNM_CASEFOLD) == 0 || fnmatch("*.png", url, FNM_CASEFOLD) == 0 || fnmatch("*.jpeg", url, FNM_CASEFOLD) == 0) // if jpg, png,
            {
                char tokens[100] = ".";             // tokens variable
                strcat(tokens, url);                // strcat function
                char ap_path[100];                  // apsolute path variable
                realpath(tokens, ap_path);          // get apsolute path
                struct stat file_stat;              // stat variable
                lstat(ap_path, &file_stat);         // use lstat function
                long file_size = file_stat.st_size; //
                char *file_data = (char *)malloc(file_size);
                FILE *file = fopen(ap_path, "rb");    // file open
                fread(file_data, file_size, 1, file); // read

                fclose(file);                                                                                                  // file close
                sprintf(response_header, "HTTP/1.1 200 OK\r\nContent-Type: image/*\r\nContent-Length:%ld\r\n\r\n", file_size); // save message
                write(client_fd, response_header, strlen(response_header));                                                    // write
                int kk = 0;
                while (kk < file_size)
                {
                    int n = write(client_fd, file_data + kk, file_size - kk); // write
                    kk += n;                                                  // kk = kk + n
                }

                close(client_fd); // close accpet
            }
            else if (fnmatch("*.html", url, FNM_CASEFOLD) == 0) // if html file
            {
                char tokens[100] = ".";
                strcat(tokens, url);
                FILE *file = fopen(tokens, "rb"); // open file
                int rd;
                char tt[40000];
                rd = fread(tt, 1, 40000, file);                                                                                                                     // get data
                fclose(file);                                                                                                                                       // file close
                sprintf(response_header, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:%d\r\n\r\n<link rel='icon' href='data:,'>\r\n\r\n", 100000); // save message
                sprintf(response_message, "%s", tt);                                                                                                                // save message
                write(client_fd, response_header, strlen(response_header));                                                                                         // write
                write(client_fd, response_message, strlen(response_message));                                                                                       // write
                close(client_fd);                                                                                                                                   // close accept
            }
            else // else Normal text file, source code
            {
                int rd;
                char tt[40000];
                char tokens[100] = ".";
                strcat(tokens, url);
                FILE *file = fopen(tokens, "rb");                                                                             // open file
                rd = fread(tt, 1, 40000, file);                                                                               // get data
                fclose(file);                                                                                                 // close file
                sprintf(response_header, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length:%d\r\n\r\n", 100000); // save message
                sprintf(response_message, "%s", tt);                                                                          // save message
                write(client_fd, response_header, strlen(response_header));                                                   // write
                write(client_fd, response_message, strlen(response_message));                                                 // write
                close(client_fd);                                                                                             // close accept
            }
        }
        else if (strcmp(url, "/") == 0) // if root path
        {
            a = 0;                                                                                                       // a option 0
            sprintf(response_message, "<link rel='icon' href='data:,'><h1>Welcome to System Programming Http</h1><br>"); // save message
            sprintf(response_header, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:%d\r\n\r\n", 100000); // save message
            write(client_fd, response_header, strlen(response_header));                                                  // write
            write(client_fd, response_message, strlen(response_message));                                                // write
            strcpy(dirpath, ".");
            ls(dirpath, a, 1, 0, 0, 0, client_fd, url, IP); // ls function call
            close(client_fd);
        }
        else
        {
            a = 1;
            sprintf(response_message, "<link rel='icon' href='data:,'><h1>System Programming Http</h1><br>");            // save message
            sprintf(response_header, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:%d\r\n\r\n", 100000); // save message
            write(client_fd, response_header, strlen(response_header));                                                  // write
            write(client_fd, response_message, strlen(response_message));                                                // write
            strcpy(dirpath, ".");                                                                                        // strcpy function call
            strcat(dirpath, url);                                                                                        // strcat function call
            ls(dirpath, a, 1, 0, 0, 0, client_fd, url, IP);                                                              // ls function call
            close(client_fd);
        }
        
        strcpy(shared_memory->client_info_list[ii].ip,IP1); //공유메모리에 값 넣기
        shared_memory->client_info_list[ii].port = client_addr.sin_port;//공유메모리에 값 넣기
        shared_memory->client_info_list[ii].pid = getpid();//공유메모리에 값 넣기
        shared_memory->client_info_list[ii].time = curtime;//공유메모리에 값 넣기
        sleep(5);
        time_t curtime2 = time(NULL);                    // get time
        char *curtime_str2 = ctime(&curtime2);            // get time
        curtime_str[strcspn(curtime_str2, "\n")] = '\0'; // Replace open characters with null characters
        printf("========== Disconnected Client ===========\n");
        printf("[%s] \n", curtime_str2);  
        printf("IP : %s\n", IP1);                    // print IP number
        printf("Port : %d\n", client_addr.sin_port); // print Port number
        pthread_create(&tidA, NULL, doit_disconnect, NULL); //쓰레드 생성
        pthread_join(tidA, NULL);        //쓰레드 대기
        puts("==========================================");
        if(shared_memory->IDLE_process > MaxIdleNum) // 만약 6보다 많다면
        {
            kill(getppid(), SIGUSR2); //프로세스 없애준다
        }
        close(client_fd);                                // close client_fd
    }
    close(socket_fd); // close socket
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// printHistory                                     //
//===============================================================================================//
// Input :                                                         //
// Output : print value                                  //
// Purpose : print history structure                         //
///////////////////////////////////////////////////////////////////////////////////////////////////
void printHistory()
{
    if (shared_memory->total_accept < 10)
    {
        for (int i = 0; i < shared_memory->total_accept; i++) //
        {
            int index = (shared_memory->total_accept - 1 - i) % MaxHistory; // index number setting
            printf("%d\t%s\t%d\t%d\t%s", i+1, // print list
                   shared_memory->client_info_list[index].ip, shared_memory->client_info_list[index].pid, 
                   shared_memory->client_info_list[index].port, ctime(&(shared_memory->client_info_list[index].time)));
        }
    }

    else
    {
        for (int i = 0; i < MaxHistory; i++)
        {
            int index = (10 - 1 - i) % MaxHistory; // index number setting
            printf("%d\t%s\t%d\t%d\t%s", i+1,
                   shared_memory->client_info_list[index].ip, shared_memory->client_info_list[index].pid,
                   shared_memory->client_info_list[index].port, ctime(&(shared_memory->client_info_list[index].time)));
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// is_ip_allowed                                     //
//===============================================================================================//
// Input : const char* client_ip - > ip address                  //
// Output : true or false value                                  //
// Purpose : check IP allow                         //
///////////////////////////////////////////////////////////////////////////////////////////////////
int is_ip_allowed(const char* client_ip) {
    FILE* file = fopen("accessible.usr", "r"); //open accessible.usr file
    if (file == NULL) {
        perror("accessible.usr 파일이 없습니다.");
    }

    char line[256];
    while (fgets(line, sizeof(line), file) != NULL) { //get ip address
        line[strcspn(line, "\n")] = '\0'; // cut \n word
        if (fnmatch(line, client_ip, FNM_CASEFOLD) == 0) { //compare word
            fclose(file);
            return 0;  //return allow ip
        }
    }
    fclose(file);
    return 1;  // return not allow ip
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// check_404                                      //
//===============================================================================================//
// Input : char* dirpath -> whether it exists or not                         //
// Output : int value                                  //
// Purpose : Determine if the path exists                         //
///////////////////////////////////////////////////////////////////////////////////////////////////
int check_404(char* dirpath)
{
    DIR *dirp;                                        // DIR
    char token[100] = ".";                              //path variable
    struct stat s,s1;                                      //stat variable
    char *ss = dirpath;                                 //save dirpath
    char ap_path[100];                                  //apsolute path
    strcat(token,dirpath);
    realpath(token,ap_path);                            //get apsolute path
    if(fnmatch("*.jpg", dirpath, FNM_CASEFOLD)==0 || fnmatch("*.png", dirpath, FNM_CASEFOLD)==0 || fnmatch("*.jpeg", dirpath, FNM_CASEFOLD)==0 )
    {
        return 201;
    }
    else if ((dirp = opendir(token)) != NULL) // if open dir
    {
        closedir(dirp);
        return 200; //exist
    }
    else if (lstat(token, &s) == 0) // if file
    {       
        return 201; //exist
    }
    else   
        return 404; //not exist
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// wild_card                                  //
//===============================================================================================//
// Input : int argc -> path count                         //
//      char** argv -> path                            //
// Output : x                                  //
// Purpose : wild card check and print                         //
///////////////////////////////////////////////////////////////////////////////////////////////////
void wild_card(int argc, char **argv,int client_fd) // wild_card function
{
    char response_message[BUFSIZE] ={0,};
    glob_t result;           // glob variable
    struct stat sb;          // struct stat
    struct stat sbb;         // struct stat
    char **file_name = NULL; // file name array
    char **dir_name = NULL;  // dir name array
    int filek = 0, dirk = 0; // file , dir count
    int wild_flag = 0;       // wild flag check
    for (int i = 1; i < argc; i++)
    {
        glob(argv[i], 0, NULL, &result);          // use glob function
        for (int j = 0; j < result.gl_pathc; j++) // Obtain the number of files and directories
        {
            if (lstat(result.gl_pathv[j], &sb) == 0 && 0 == S_ISDIR(sb.st_mode)) // if file
            {
                filek++; // file count ++
            }
            if (lstat(result.gl_pathv[j], &sbb) == 0 && S_ISDIR(sbb.st_mode)) // if dir
            {
                dirk++; // dir count ++
            }
        }
        globfree(&result); // glob memory free
    }
    // Dynamically allocate memory to an array to store file names and directory names
    if (filek > 0)
    {
        file_name = malloc(sizeof(char *) * (filek + 300)); // Dynamic memory allocation
    }

    if (dirk > 0)
    {
        dir_name = malloc(sizeof(char *) * (dirk + 300)); // Dynamic memory allocation
    }

    // Save files and directories
    int file_idx = 0, dir_idx = 0; // file, dir index count
    for (int km = 1; km < argc; km++)
    {
        glob(argv[km], 0, NULL, &result); // instance glob
        for (int i = 0; i < result.gl_pathc; i++)
        {
            if (lstat(result.gl_pathv[i], &sb) == 0 && 0 == S_ISDIR(sb.st_mode)) // if file
            {
                file_name[file_idx] = result.gl_pathv[i]; // save file name
                file_idx++;
            }
            if (lstat(result.gl_pathv[i], &sbb) == 0 && S_ISDIR(sbb.st_mode)) // if dir
            {
                dir_name[dir_idx] = result.gl_pathv[i]; // save dir name
                dir_idx++;
            }
        }
    }
    struct stat sst; // stat struct
    for (int i = 1; i < argc; i++) // Receive non-wildcard paths
    {
        for (int j = 0; j < strlen(argv[i]); j++)
        {
            if (argv[i][j] == '?' || argv[i][j] == '*' || argv[i][j] == '[') // if the factor is a wildcard
            {
                wild_flag = 1; // wild flag true
            }
        }
        if (!wild_flag) // if not wild factor just path
        {
            if (lstat(argv[i], &sst) == 0) // if existence
            {
                if (S_ISDIR(sst.st_mode) == 0) // if file
                {
                    file_name[file_idx] = argv[i]; // push data
                    file_idx++;                    // index ++
                    filek++;                       // index ++
                }
                else if (S_ISDIR(sst.st_mode)) // if dir
                {
                    dir_name[dir_idx] = argv[i]; // push data
                    dir_idx++;                   // index ++
                    dirk++;                      // index ++
                }
            }
        }
    }

    if (filek > 0) // print file list
    {
        sorting(file_name, filek, 0, 0, NULL); // sort file list
        sprintf(response_message, "<div><table border = '1'><tr><th align='left' >Name</th></tr>");
        write(client_fd, response_message, strlen(response_message));
        for (int i = 0; i < filek; i++)
        {   
            char k[1000];
            getcwd(k,1000);
            strcat(k,"/");
            strcat(k,file_name[i]);
            char* am = strstr(file_name[i],"html_ls.html");
            if(am != NULL){continue;}
            struct stat sb;
            lstat(k,&sb);
            char* check_type = check_filetype(sb.st_mode);
            
            sprintf(response_message, "<tr><th align='left' style = 'color:%s' ><a href = '%s'>%s</a></th></tr>", check_type,file_name[i], file_name[i]); // print
            write(client_fd, response_message, strlen(response_message));
        }
        sprintf(response_message, "</table></div><br><br>");
        write(client_fd, response_message, strlen(response_message));
    }
    if (dirk > 0) // print dir list
    {
        sorting(dir_name, dirk, 0, 0, NULL); // sort dir list
        for (int i = 0; i < dirk; i++)
        {
            int checking = 0;
            DIR *dir;                                    // DIR
            char ap_path[1000];                          // path
            struct dirent *dp;                           // dirent struct
            sprintf(response_message,"<div><table border = '1'><tr align = 'left'><th>Directory path: %s</th></tr>", dir_name[i]); // print file path
            write(client_fd, response_message, strlen(response_message));
            if (dir_name[i][0] == '/')                   // if path
            {
                //strcpy(ap_path ,dir_name[i]);
                if ((dir = opendir(dir_name[i])) == NULL) // if not open
                {
                    continue; // continue
                }
                checking = 0;
            }
            else // if relative path
            {
                checking = 1;
                getcwd(ap_path, 1000);                // add path
                strcat(ap_path, "/");                 // add /
                strcat(ap_path, dir_name[i]);         // add path
                if ((dir = opendir(ap_path)) == NULL) // if not open
                {
                    continue; // continue
                }
            }
            while ((dp = readdir(dir)) != NULL) // read file
            {
                
                if (dp->d_name[0] == '.') // if hidden file
                {
                    continue; // coninue
                }
                if(checking == 1)
                {
                    struct stat sb;
                    char k[1000];
                    strcpy(k,ap_path);
                    strcat(k,"/");
                    strcat(k,dp->d_name);
                    lstat(k,&sb);
                    char* check_type = check_filetype(sb.st_mode);
                    
                    sprintf(response_message,"<tr align='left' style = 'color:%s' ><th><a href = '%s'>%s</a></th></tr>", check_type,k,dp->d_name); // print
                    write(client_fd, response_message, strlen(response_message));
                }
                else if(checking == 0)
                {
                    struct stat sb;
                    char k[1000];
                    strcpy(k, dir_name[i]);
                    strcat(k,"/");
                    strcat(k,dp->d_name);
                    lstat(k,&sb);
                    char* check_type = check_filetype(sb.st_mode);
                
                    sprintf(response_message,"<tr align='left' style = 'color:%s' ><th><a href = '%s'>%s</a></th></tr>", check_type,k,dp->d_name); // print
                    write(client_fd, response_message, strlen(response_message));
                }
            }
            sprintf(response_message,"</table></div><br><br>"); // print
            write(client_fd, response_message, strlen(response_message));
            closedir(dir);  // close dir
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// no_dir                                                       //
//===============================================================================================//
// Input : struct stat *sb -> struct                                         //
//       char* filename -> files list                                        //
// Output : x                                                        //
// Purpose : -a optional File output                                        //
///////////////////////////////////////////////////////////////////////////////////////////////////
void no_dir(struct stat *sb, char *filename, int client_fd)
{
    char response_message[BUFSIZE] = {0,};
    char* am = strstr(filename,"html_ls.html"); //if file name html_ls.html ignore
    if(am != NULL){return;}
    sprintf(response_message, "<table border='1'> <tr><th>Name</th> <th>Permission</th> <th>Link</th> <th>Owner</th> <th>Group</th> <th>Size</th> <th>Last Modified</th>"); //make table
    write(client_fd, response_message, strlen(response_message)); //write
    char qq[10];
    qq[0] = (S_ISDIR(sb->st_mode) ? 'd' : '-');     // d or -
    qq[1] = (sb->st_mode & S_IRUSR ? 'r' : '-');    // r or -
    qq[2] = (sb->st_mode & S_IWUSR ? 'w' : '-');    // w or -
    qq[3] = (sb->st_mode & S_IXUSR ? 'x' : '-');    // x or -
    qq[4] = (sb->st_mode & S_IRGRP ? 'r' : '-');    // r or -
    qq[5] = (sb->st_mode & S_IWGRP ? 'w' : '-');    // w or -
    qq[6] = (sb->st_mode & S_IXGRP ? 'x' : '-');    // x or -
    qq[7] = (sb->st_mode & S_IROTH ? 'r' : '-');    // r or -
    qq[8] = (sb->st_mode & S_IWOTH ? 'w' : '-');    // w or -
    qq[9] = (sb->st_mode & S_IXOTH ? 'x' : '-');    // x or -
    char *check_type = check_filetype(sb->st_mode); // file type check

    sprintf(response_message, "<tr align='left' style = 'color:%s'><td><a href='%s'> %s</a></td><th>", check_type, filename, filename); //save message
    write(client_fd,response_message,strlen(response_message));//write
    for (int i = 0; i < 10; i++)
    {
        sprintf(response_message, "%c", qq[i]); //save message
        write(client_fd, response_message,strlen(response_message)); //write
    }
    sprintf(response_message, "</th><th>%ld</th>", (long)sb->st_nlink); //save message
    write(client_fd, response_message,strlen(response_message)); //write
    struct passwd *pw = getpwuid(sb->st_uid);
    struct group *grp = getgrgid(sb->st_gid);
    sprintf(response_message, "<th>%s</th><th>%s</th>", pw->pw_name, grp->gr_name);//save message
    write(client_fd, response_message,strlen(response_message)); //write
    sprintf(response_message, "<th>%ld</th>", (long)sb->st_size); //save message
    write(client_fd, response_message,strlen(response_message)); //write

    char *access_time = ctime(&sb->st_mtime);
    access_time[strlen(access_time) - 1] = '\0';
    char timebuf[80];
    struct tm *tm_info = localtime(&sb->st_mtime);
    strftime(timebuf, 80, "%b %d %H:%M", tm_info);
    sprintf(response_message, "<th>%s</th></table>", timebuf); //save message
    write(client_fd, response_message,strlen(response_message)); //write

}
///////////////////////////////////////////////////////////////////////////////////////////////////
// ls                                                           //
//===============================================================================================//
// Input : char* dirpath -> directory path                                   //
//       int a -> a option                                               //
//       int l -> l option                                               //
// Output : x                                                        //
// Purpose : process by file option                                         //
///////////////////////////////////////////////////////////////////////////////////////////////////
void ls(char *dirpath, int a, int l, int r, int h, int S, int client_fd, char* url, char* IP)
{
    struct dirent *direntp;                           // dirent struct
    DIR *dirp;                                        // DIR
    char *filenames[1000]; // memory allocate
    struct stat stats[1000];                          // stat struct
    int total = 0;                                    // total
    struct stat statbuf;                              // stat struct
    int num_files = 0;                                // file count
    struct stat s;                                    // stat struct
    char *ss = dirpath; 
    char* token;
    char response_message[BUFSIZE] ={0,};
    if ((dirp = opendir(dirpath)) != NULL) // opendir function
    {
        while ((direntp = readdir(dirp)) != NULL) // readdir function
        {
            char* name_html = strstr(direntp->d_name,"html_ls.html");  //if html_ls.html file
            if(name_html != NULL){continue;} //continue
            struct stat k;
            lstat(direntp->d_name, &k); // stat function
            if (a == 0)
            {
                if (direntp->d_name[0] == '.') // if not a option
                {
                    continue;
                }
            }
            total += k.st_blocks;                           // total size check
            filenames[num_files] = direntp->d_name; // push data
            num_files++;                                    // num count ++
        }
        sorting(filenames, num_files, r, S, dirpath); // sorting filename list
        if (lstat(dirpath, &statbuf) == -1)            // if error
        {
            printf("cannot access %s : No such file or directory\n", direntp->d_name); // print error
        }
        else
        {
            print_file_info(h, total, statbuf.st_mode, direntp, dirpath, l, filenames, num_files, client_fd, url, IP); // instance print_file info function
            closedir(dirp);                                                               // close dir
                    
        }
    }
    else if (lstat(ss, &s) == 0) // if not directory
    {
        if (s.st_mode & S_IFREG) // if file
        {
            if (l == 1) // if l option
            {
                no_dir(&s, dirpath, client_fd); // call no_dir function
            }
            else
            {
                char* check_type = check_filetype(s.st_mode); //check type
                char copy[1000];
                getcwd(copy,1000);
                strcpy(copy,"/");
                strcpy(copy,dirpath);
                sprintf(response_message,"<table border='1'><tr><th>Name</th></tr><tr><th style = 'color:%s'><a href = '%s'>%s</a></tr></th></border>",check_type ,copy, ss); //save message
                write(client_fd, response_message, strlen(response_message)); //write
            }
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// sorting                                                        //
//===============================================================================================//
// Input : files list, the number of files                                   //
// Output : x (just sorting)                                            //
// Purpose : sort alphabetical order                                         //
///////////////////////////////////////////////////////////////////////////////////////////////////
void sorting(char **filenames, int num_files, int r, int S, char *dir_path)
{
    for (int i = 0; i < num_files - 1; i++) // sort alphabetically
    {
        for (int j = i + 1; j < num_files; j++)
        {
            char *name1 = filenames[i]; // file name
            char *name2 = filenames[j]; // file name
            int k = 0;
            int dota = 0, dotb = 0;
            if (name1[k] == '.') // ignore .
            {
                dota = 1; // dota option 1
            }
            if (name2[k] == '.') // ignore .
            {
                dotb = 1; // dotb option 1
            }
            while (toupper(name1[k + dota]) == toupper(name2[k + dotb])) // if not same break
            {
                if (name1[k] == '\0' || name2[k] == '\0') // if end word
                {
                    break; // break
                }
                k++; // count ++
            }

            if (S == 0 && toupper(name1[k + dota]) > toupper(name2[k + dotb])) // if words are not arranged lexicographically
            {
                filenames[i] = name2; // change
                filenames[j] = name1; // change
            }
        }
    }
    if (S == 1) // S option
    {
        DIR *dir;
        struct dirent *dp;
        struct stat sb;
        int filesize_arr[1000] = {0}; // file size

        struct dirent *entry;
        char ap_path[1000];
        realpath(dir_path, ap_path); // Obtaining the absolute path
        for (int i = 0; i < num_files; i++)
        {
            dir = opendir(ap_path); // dir Open
            while ((dp = readdir(dir)) != NULL)
            {
                realpath(dir_path, ap_path); // Obtaining the absolute path

                if (strcmp(filenames[i], dp->d_name) == 0)
                {
                    strcat(ap_path, "/");          // add path
                    strcat(ap_path, filenames[i]); // add path
                    lstat(ap_path, &sb);
                    filesize_arr[i] = sb.st_size; // get file size
                }
            }
        }
        closedir(dir);

        for (int i = 0; i < num_files - 1; i++) // sort alphabetically
        {
            for (int j = i + 1; j < num_files; j++)
            {
                char *name1 = filenames[i]; // file name
                char *name2 = filenames[j]; // file name
                if (filesize_arr[i] < filesize_arr[j])
                {
                    double temp;
                    filenames[i] = name2;              // change name
                    filenames[j] = name1;              // change name
                    temp = filesize_arr[i];            // change size
                    filesize_arr[i] = filesize_arr[j]; // change size
                    filesize_arr[j] = temp;            // change size
                }
            }
        }

        if (r == 1) // if r option 1
        {
            for (int i = 0; i < num_files / 2; i++) // reverse list
            {
                char *temp = filenames[i];
                int temp1;
                filenames[i] = filenames[num_files - i - 1];       // change
                filenames[num_files - i - 1] = temp;               // change
                temp1 = filesize_arr[i];                           // change
                filesize_arr[i] = filesize_arr[num_files - i - 1]; // change
                filesize_arr[num_files - i - 1] = temp1;           // change
            }
            for (int i = 0; i < num_files - 1; i++) // sort alphabetically
            {
                for (int j = i + 1; j < num_files; j++)
                {
                    char *name1 = filenames[i]; // filename
                    char *name2 = filenames[j]; // filename
                    int k = 0;
                    int dota = 0, dotb = 0;
                    if (name1[k] == '.') // ignore .
                    {
                        dota = 1; // dota 1
                    }
                    if (name2[k] == '.') // ignore .
                    {
                        dotb = 1; // dotb 1
                    }

                    if (filesize_arr[i] != filesize_arr[j]) // if file size not same
                    {
                        continue; // continue
                    }
                    else
                    {
                        while (toupper(name1[k + dota]) == toupper(name2[k + dotb])) // if not same break
                        {
                            if (name1[k] == '\0' || name2[k] == '\0') // end word
                            {
                                break; // break
                            }
                            k++; // count ++
                        }
                        if (toupper(name1[k + dota]) < toupper(name2[k + dotb])) // if words are not arranged lexicographically
                        {
                            filenames[i] = name2; // change
                            filenames[j] = name1; // change
                        }
                    }
                }
            }
        }
    }

    if (S == 0 && r == 1) // if S option 0, r option 1
    {
        for (int i = 0; i < num_files / 2; i++) // just reverse list
        {
            char *temp = filenames[i];                   // change
            filenames[i] = filenames[num_files - i - 1]; // change
            filenames[num_files - i - 1] = temp;         // change
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// check_filetype                                                  //
//===============================================================================================//
// Input : mode_t mode -> struct                                         //
// Output : char value -> file type                                              //
// Purpose : Distinguish whether it is a link file or a directory or other                   //
///////////////////////////////////////////////////////////////////////////////////////////////////
char* check_filetype(mode_t mode)
{
    if (S_ISLNK(mode)) return "Green"; // return 1
    else if (S_ISDIR(mode)) return "Blue"; // return 2
    else  return "red"; // return 3
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// print_file_info                                                  //
//===============================================================================================//
// Input : int - total(total file size)                                    //
//      mode_t - mode                                               //
//      struct dirent* -> file structure                                      //
//      char* dir_path -> file path                                         //
//         int lflag -> -l option is ture or false                                //
//        char **filenames -> files list                                      //
//      int num_files -> number of files                                      //
// Output : x(just print)                                               //
// Purpose : print                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////
void print_file_info(int h, int total, mode_t mode, struct dirent *dp, char *dir_path, int lflag, char **filenames, int num_files, int client_fd,char* url,char* IP)
{
    char response_message[BUFSIZE] ={0,}; //response message variable
    sprintf(response_message, "<div>"); //save message
    write(client_fd, response_message, strlen(response_message)); //write
    char ap_path[1000];
    if (lflag) // l option print
    {
        DIR *dir; //DIR variable
        struct dirent *entry; //dirent variable
        struct stat sb; //stat variable
        if (total != 0) // if total != 0
        {
            realpath(dir_path, ap_path);                          // get path
            sprintf(response_message, "<h3>Directory path: %s <br> total : %d <h3> <table border='1'> <tr><th>Name</th> <th>Permission</th> <th>Link</th> <th>Owner</th> <th>Group</th> <th>Size</th> <th>Last Modified</th>", ap_path, total / 2); // save message
            write(client_fd, response_message, strlen(response_message)); //wrtie
        }
        for (int i = 0; i < num_files; i++)
        {
            realpath(dir_path, ap_path);        // get path
            dir = opendir(ap_path);             // open dir
            while ((dp = readdir(dir)) != NULL) // read dir
            {
                realpath(dir_path, ap_path); // call path

                if (strcmp(filenames[i], dp->d_name) == 0) //if same file name
                {
                    strcat(ap_path, "/");
                    strcat(ap_path, filenames[i]);
                
                    if (lstat(ap_path, &sb) == -1) // lstat
                    {
                        printf("에러 file info\n"); // print error
                    }
                    char qq[10];
                    qq[0] = (S_ISDIR(sb.st_mode) ? 'd' : '-');  // d or -
                    qq[1] = (sb.st_mode & S_IRUSR ? 'r' : '-'); // r or -
                    qq[2] = (sb.st_mode & S_IWUSR ? 'w' : '-'); // w or -
                    qq[3] = (sb.st_mode & S_IXUSR ? 'x' : '-'); // x or -
                    qq[4] = (sb.st_mode & S_IRGRP ? 'r' : '-'); // r or -
                    qq[5] = (sb.st_mode & S_IWGRP ? 'w' : '-'); // w or -
                    qq[6] = (sb.st_mode & S_IXGRP ? 'x' : '-'); // x or -
                    qq[7] = (sb.st_mode & S_IROTH ? 'r' : '-'); // r or -
                    qq[8] = (sb.st_mode & S_IWOTH ? 'w' : '-'); // w or -
                    qq[9] = (sb.st_mode & S_IXOTH ? 'x' : '-'); // x or -
                    char* check_type = check_filetype(sb.st_mode);//file type check
                    if(strcmp(filenames[i],".")==0) //current path
                    {
                        sprintf(response_message, "<tr align='left' style = 'color:%s'><td><a href='http://%s:40000%s'> %s</a></td><th>", check_type,IP,dp->d_name, dp->d_name);  //save message
                    }
                    else if(strcmp(filenames[i],"..")==0) //parent path
                    {
                        if(root_path_check) //if root path link
                        {
                            sprintf(response_message, "<tr align='left' style = 'color:%s'><td><a href='http://%s:40000'> %s</a></td><th>", check_type, IP,dp->d_name);  //save message
                            root_path_check = 0;  //root_path == 0;
                        }
                        else 
                        {
                            char *befo = strdup(url);
                            char* last_slash = strrchr(befo, '/');
                            *last_slash = '\0'; //cut link
                            sprintf(response_message, "<tr align='left' style = 'color:%s'><td><a href='http://%s:40000%s'> %s</a></td><th>", check_type,IP,befo, dp->d_name);  //save message
                            free(befo);
                        } //write
                    }
                    else if(strcmp(url,"/")==0)
                    {
                        sprintf(response_message, "<tr align='left' style = 'color:%s'><td><a href='http://%s:40000%s%s'> %s</a></td><th>", check_type,IP,url,dp->d_name, dp->d_name);  //save message   
                    }
                    else sprintf(response_message, "<tr align='left' style = 'color:%s'><td><a href='http://%s:40000%s/%s'> %s</a></td><th>", check_type,IP, url,dp->d_name, dp->d_name);  //save message
                    write(client_fd, response_message, strlen(response_message));
                    
                    for(int i = 0; i<10; i++)
                    {
                        sprintf(response_message, "%c", qq[i]); //save message
                        write(client_fd, response_message, strlen(response_message)); //write
                    }
                    sprintf(response_message, "</th><th>%ld</th>", (long)sb.st_nlink);  //save message
                    write(client_fd, response_message, strlen(response_message)); //write
                    struct passwd *pw = getpwuid(sb.st_uid);
                    struct group *grp = getgrgid(sb.st_gid);

                    sprintf(response_message, "<th>%s</th><th>%s</th>", pw->pw_name, grp->gr_name);  //save message
                    write(client_fd, response_message, strlen(response_message)); //write
                    if (h == 1) // if h option 1
                    {
                        long k = sb.st_size;
                        double kkk = (double)k;
                        double check, check2, check3;
                        check2 = k / 1024 * 1024;
                        check3 = k / 1024;

                        if (k > 1024 * 1024 * 1024) // if G
                        {
                            if (k % (1024 * 1024 * 1024) == 0) // If there's no rest
                            {
                                sprintf(response_message, "<th>%.0fG</th>", (double)k); //save message
                            }
                            else
                                sprintf(response_message, "<th>%.1fG</th>", (double)k / (1024 * 1024 * 1024)); // If there's rest
                        }
                        else if (k > 1024 * 1024) // if M
                        {
                            if (k % (1024 * 1024) == 0) // If there's no rest
                            {
                                sprintf(response_message, "<th>%.0fM</th>", (double)k);  //save message
                            }
                            else
                                sprintf(response_message, "<th>%.1fM</th>", (double)k / (1024 * 1024)); // If there's rest
                        }
                        else if (k > 1024) // if K
                        {
                            if (k % 1024 == 0) // If there's no rest
                            {
                                sprintf(response_message, "<th>%.0fK</th>", (double)sb.st_size / 1024);  //save message
                            }
                            else
                                sprintf(response_message, "<th>%.1fK</th>", (double)k / 1024); // If there's rest
                        }
                        else
                        {
                            sprintf(response_message, "<th>%ld</th>", sb.st_size);  //save message
                        }
                    }
                    else
                    {
                        sprintf(response_message, "<th>%ld</th>", (long)sb.st_size);  //save message
                    }
                    write(client_fd, response_message, strlen(response_message)); //write
                    char *access_time = ctime(&sb.st_mtime);
                    access_time[strlen(access_time) - 1] = '\0';
                    char timebuf[80]; //char variable
                    struct tm *tm_info = localtime(&sb.st_mtime);
                    strftime(timebuf, 80, "%b %d %H:%M", tm_info);
                    sprintf(response_message, "<th>%s</th>", timebuf);  //save message
                    write(client_fd, response_message, strlen(response_message)); //write
                }
            }

            closedir(dir); // close dir
        }
        sprintf(response_message, "</table>");//close table
        write(client_fd, response_message, strlen(response_message));
    }
    else // just ls option
    {
        sprintf(response_message, "<table border='1'><tr align='left'><th>Name</th></tr>"); //make table
        write(client_fd, response_message, strlen(response_message));
        struct stat sb;
        for (int i = 0; i < num_files; i++)
        {
            realpath(dir_path, ap_path); // get path
            strcat(ap_path, "/");
            strcat(ap_path, filenames[i]);
            lstat(ap_path, &sb);
            char* check_type = check_filetype(sb.st_mode); //check file type
            sprintf(response_message, "<tr align='left' style = 'color:%s'><th><a href = '%s'>%s</a></th></tr>",check_type,ap_path,filenames[i]); // print
            write(client_fd, response_message, strlen(response_message));
        }
        sprintf(response_message, "</table>"); 
        write(client_fd, response_message, strlen(response_message));
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// doit1                                     //
//===============================================================================================//
// Input :  void* vptr -> anything                                                       //
// Output : x                                                 //
// Purpose : shared memory initialize                                       //
///////////////////////////////////////////////////////////////////////////////////////////////////
void *doit1(void *vptr)
{
    int shm_id, val; //shm_id, value variable
    void *shm_addr; //void variable
    if((shm_id = shmget((key_t)PORTNO, sizeof(struct ClientInfo), IPC_CREAT | 0666)) == -1) //Create shared memory
    {
        printf("shmget 초기화 fail"); //만약 실패한다면 출력
        return NULL;
    }
    pthread_mutex_lock(&counter_mutex); //뮤택스 락 걸기
    shared_memory = (ClientInfo_list*)shmat(shm_id,NULL,0); //공유메모리 연결
    shared_memory->connect_process = 0;//shared_memory의 값 초기화
    shared_memory->IDLE_process = 0;//shared_memory의 값 초기화
    shared_memory->total_accept = 0;//shared_memory의 값 초기화
    shared_memory->process_counter = 5;//shared_memory의 값 초기화
    for(int i = 0; i<10; i++)
    {
        shared_memory->Idle_pid_list[i] = 0;//shared_memory의 값 초기화
    }
    pthread_mutex_unlock(&counter_mutex);//뮤택스 락 풀기
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// doit2                                     //
//===============================================================================================//
// Input :  void* vptr -> anything                                                       //
// Output : x                                                 //
// Purpose : fork print                                       //
///////////////////////////////////////////////////////////////////////////////////////////////////
void *doit2(void *vptr)
{
    int shm_id; //shm_id, value variable
    void *shm_addr; //void variable
    if((shm_id = shmget((key_t)PORTNO, sizeof(struct ClientInfo), IPC_CREAT | 0666)) == -1) //Create shared memory
    {
        printf("shmget 초기화 fail"); //만약 실패한다면 출력
        return NULL;
    }
    pthread_mutex_lock(&counter_mutex); //뮤택스 락 걸기
    shared_memory = (ClientInfo_list*)shmat(shm_id,NULL,0); //공유메모리 연결
    shared_memory->IDLE_process++; //idle process 증가
    for(int i = 0; i<10; i++)
    {
        if(shared_memory->Idle_pid_list[i] == 0)
        {
            shared_memory->Idle_pid_list[i] = getpid();//shared_memory idle list에 값 넣기
            break;
        }

    }
    
    time_t curtime = time(NULL);
    char* curtime_str = ctime(&curtime);
    curtime_str[strcspn(curtime_str, "\n")] = '\0'; 
    printf("[%s] %d Process is forked.\n[%s] IdleProcessCount : %d\n", curtime_str, getpid(),curtime_str, shared_memory->IDLE_process);
    pthread_mutex_unlock(&counter_mutex);//뮤택스 락 풀기
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// doit3                                     //
//===============================================================================================//
// Input :  void* vptr -> anything                                                       //
// Output : x                                                 //
// Purpose : Idle discount                                       //
///////////////////////////////////////////////////////////////////////////////////////////////////
void *doit3(void *vptr)
{
    int shm_id, val; //shm_id, value variable
    void *shm_addr; //void variable
    if((shm_id = shmget((key_t)PORTNO, sizeof(struct ClientInfo), IPC_CREAT | 0666)) == -1) //Create shared memory
    {
        printf("shmget 초기화 fail"); //만약 실패한다면 출력
        return NULL;
    }
    pthread_mutex_lock(&counter_mutex); //뮤택스 락 걸기
    shared_memory = (ClientInfo_list*)shmat(shm_id,NULL,0); //공유메모리 연결
    shared_memory->IDLE_process--; //idle process 증가
    for(int i = 0; i<10; i++)
    {
        if(shared_memory->Idle_pid_list[i] == getpid())
        {
            shared_memory->Idle_pid_list[i] = 0;//shared_memory idle list에서 빼주기
            break;
        }

    }
    
    time_t curtime = time(NULL);
    char* curtime_str = ctime(&curtime);
    curtime_str[strcspn(curtime_str, "\n")] = '\0'; 
    printf("[%s] IdleProcessCount : %d\n", curtime_str, shared_memory->IDLE_process);
    pthread_mutex_unlock(&counter_mutex);//뮤택스 락 풀기
    return NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// doit4                                     //
//===============================================================================================//
// Input :  void* vptr -> anything                                                       //
// Output : x                                                 //
// Purpose : delete process                                       //
///////////////////////////////////////////////////////////////////////////////////////////////////
void *doit4(void *vptr)
{
    int shm_id, val; //shm_id, value variable
    void *shm_addr; //void variable
    if((shm_id = shmget((key_t)PORTNO, sizeof(struct ClientInfo), IPC_CREAT | 0666)) == -1) //Create shared memory
    {
        printf("shmget 초기화 fail"); //만약 실패한다면 출력
        return NULL;
    }
    pthread_mutex_lock(&counter_mutex); //뮤택스 락 걸기
    shared_memory = (ClientInfo_list*)shmat(shm_id,NULL,0); //공유메모리 연결
    shared_memory->IDLE_process--; //idle process 증가
    for(int i = 0; i<10; i++)
    {
        if(shared_memory->Idle_pid_list[i] == getpid())
        {
            shared_memory->Idle_pid_list[i] = 0;//shared_memory idle list에서 빼주기
            break;
        }

    }
    time_t curtime = time(NULL);
    char* curtime_str = ctime(&curtime);
    curtime_str[strcspn(curtime_str, "\n")] = '\0'; 
    printf("[%s] IdleProcessCount : %d\n", curtime_str, shared_memory->IDLE_process);
    pthread_mutex_unlock(&counter_mutex);//뮤택스 락 풀기
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// doit_dec                                                                   //
//===============================================================================================//
// Input : x                                                                  //
// Output : void index                                                                 //
// Purpose : Reduce idle process                                               //
///////////////////////////////////////////////////////////////////////////////////////////////////
void *doit_dec(void *vptr)
{
    if((shm_id = shmget((key_t)PORTNO, sizeof(struct ClientInfo), IPC_CREAT | 0666)) == -1) //공유 메모리 생성
    {
        printf("shmget dec fail");//공유 메모리 실패하면 출력
        return NULL;
    }
    shared_memory = (ClientInfo_list*)shmat(shm_id,NULL,0); //공유 메모리 연결
    pthread_mutex_lock(&counter_mutex); //뮤택스 잠금
    int i = shared_memory->connect_process; //i에 연결된 프로세스 값 넣고
    if(shared_memory->connect_process < 9)
    {
        shared_memory->connect_process++; //9개 이하라면 증가
    }
    shared_memory->total_accept++; //총 accept한 값 체크
    int* ii = malloc(sizeof(int));  //반환할 값 동적할당
    for(int j = 0; j<MaxHistory; j++) //max history만큼 돌리기
    {
        if(shared_memory->Idle_pid_list[i] == getpid()) // 현재 pid랑 같다면
        {
            shared_memory->Idle_pid_list[i] = 0; // idle process list에서 빼주고
            break;
        }
    }
    if(shared_memory->total_accept > MaxHistory) //만약 배열에 다 저장된 상태라면
    {
        for(int i = 0; i<MaxHistory-1; i++) //옆으로 한칸씩 밀어주기
        {
        strcpy(shared_memory->client_info_list[i].ip, shared_memory->client_info_list[i + 1].ip);
        shared_memory->client_info_list[i].number = shared_memory->client_info_list[i + 1].number;
        shared_memory->client_info_list[i].port = shared_memory->client_info_list[i + 1].port;
        shared_memory->client_info_list[i].time = shared_memory->client_info_list[i + 1].time;
        shared_memory->client_info_list[i].pid = shared_memory->client_info_list[i + 1].pid;
        }
    }
    shared_memory->client_info_list[i].number = shared_memory->total_accept; //accept 번호 바꾸기
    *ii = i;
    pthread_mutex_unlock(&counter_mutex); //뮤택스 잠금 풀기
    return (void*)ii; //리턴
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// doit_disconnect                                                                   //
//===============================================================================================//
// Input : x                                                                   //
// Output : x                                                                    //
// Purpose : Reduce idle process                                               //
///////////////////////////////////////////////////////////////////////////////////////////////////
void *doit_disconnect(void *vptr)
{
    if((shm_id = shmget((key_t)PORTNO, sizeof(struct ClientInfo), IPC_CREAT | 0666)) == -1) //공유 메모리 생성
    {
        printf("shmget count fail");//실패시 오류 출력
        return NULL;
    }
    time_t curtime = time(NULL); // 시간 체크

    char* curtime_str = ctime(&curtime);//시간 변환
    curtime_str[strcspn(curtime_str, "\n")] = '\0'; // \n값 없애주기 
    shared_memory = (ClientInfo_list*)shmat(shm_id,NULL,0);//공유 메모리 연결
    pthread_mutex_lock(&counter_mutex); //뮤택스 잠금
    shared_memory->IDLE_process++; //공유 메모리 값 변경
    for(int i = 0; i<MaxChilds; i++)
    {
        if(shared_memory->Idle_pid_list[i] == 0) //빈공간이면
        {
            shared_memory->Idle_pid_list[i] = getpid(); //해당 공간에 넣기
            break;
        }
    }
    printf("\n[%s] IdleProcessCount : %d\n", curtime_str, shared_memory->IDLE_process); //출력
    pthread_mutex_unlock(&counter_mutex); //뮤택스 잠금 풀기
    return NULL;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// doit5                                     //
//===============================================================================================//
// Input :  void* vptr -> anything                                                       //
// Output : x                                                 //
// Purpose : shared memory terminated                                       //
///////////////////////////////////////////////////////////////////////////////////////////////////
void *doit5(void *vptr)
{
    int shm_id, val; //shm_id, value variable
    void *shm_addr; //void variable
    if((shm_id = shmget((key_t)PORTNO, sizeof(struct ClientInfo), IPC_CREAT | 0666)) == -1) //Create shared memory
    {
        printf("shmget 초기화 fail"); //만약 실패한다면 출력
        return NULL;
    }
    pthread_mutex_lock(&counter_mutex); //뮤택스 락 걸기
    shared_memory = (ClientInfo_list*)shmat(shm_id,NULL,0); //공유메모리 연결
    for (int i = 0; i < MaxChilds; i++)
    {
        int check = 0;
        if (pids[i] != 0) // If the child process pid number exists
        {
            time_t curtime = time(NULL);                    // get time
            char *curtime_str = ctime(&curtime);            // get time
            curtime_str[strcspn(curtime_str, "\n")] = '\0'; // Replace open characters with null characters
            for(int j = 0; j<MaxChilds; j++)
            {
                if(shared_memory->Idle_pid_list[j] == pids[i])
                {
                    shared_memory->IDLE_process--;  //idle process 수 줄이기
                    printf("\n[%s] %d Process is terminated.\n[%s] IdleProcessCount : %d", curtime_str, pids[i], curtime_str, shared_memory->IDLE_process); //종료 표시
                    check = 0;
                    break;
                }
                check = 1;
            }
            if(check) printf("\n[%s] %d Process is terminated.", curtime_str, pids[i]); //만약 idle이 아닌 process 인 경우 표시

            kill(pids[i], SIGTERM); // kill signal
        }
    }

    pthread_mutex_unlock(&counter_mutex); // 뮤택스 락 풀기
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// doit6                                                             //
//===============================================================================================//
// Input :                                                                             //
// Output : print value                                  //
// Purpose : kill Idle process function                                       //
///////////////////////////////////////////////////////////////////////////////////////////////////
void *doit6(void *vptr) 
{
    int shm_id, val;
    void *shm_addr;
    if((shm_id = shmget((key_t)PORTNO, sizeof(struct ClientInfo), IPC_CREAT | 0666)) == -1)//공유 메모리 생성
    {
        printf("shmget 초기화 fail");//에러시 오류 출력
        return NULL;
    }
    pid_t a = 0;
    shared_memory = (ClientInfo_list*)shmat(shm_id,NULL,0); //공유 메모리 연결
    pthread_mutex_lock(&counter_mutex); //뮤택스 락 걸기
    
    for(int i=0;i<10; i++)
    {
        if(getpid() == shared_memory->Idle_pid_list[i])
        {
            shared_memory->Idle_pid_list[i] = 0;
        }
    }
    pthread_mutex_unlock(&counter_mutex); //뮤택스 락 풀어주기
    return NULL;

}
///////////////////////////////////////////////////////////////////////////////////////////////////
// doit7                                                             //
//===============================================================================================//
// Input :                                                                             //
// Output : print value                                  //
// Purpose : discount idle process                                       //
///////////////////////////////////////////////////////////////////////////////////////////////////
void *doit7(void *vptr) 
{
    int shm_id, val;
    void *shm_addr;
    if((shm_id = shmget((key_t)PORTNO, sizeof(struct ClientInfo), IPC_CREAT | 0666)) == -1)//공유 메모리 생성
    {
        printf("shmget 초기화 fail");//에러시 오류 출력
        return NULL;
    }
    shared_memory = (ClientInfo_list*)shmat(shm_id,NULL,0); //공유 메모리 연결
    pthread_mutex_lock(&counter_mutex); //뮤택스 락 걸기
    shared_memory->IDLE_process--; //수 줄이기

    pthread_mutex_unlock(&counter_mutex); //뮤택스 락 풀어주기
    return NULL;

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// doit8                                                             //
//===============================================================================================//
// Input :                                                                             //
// Output : print value                                  //
// Purpose : discount idle process                                       //
///////////////////////////////////////////////////////////////////////////////////////////////////
void *doit8(void *vptr) 
{
    int shm_id, val;
    void *shm_addr;
    if((shm_id = shmget((key_t)PORTNO, sizeof(struct ClientInfo), IPC_CREAT | 0666)) == -1)//공유 메모리 생성
    {
        printf("shmget 초기화 fail");//에러시 오류 출력
        return NULL;
    }
    shared_memory = (ClientInfo_list*)shmat(shm_id,NULL,0); //공유 메모리 연결
    pthread_mutex_lock(&counter_mutex); //뮤택스 락 걸기
    for(int i = 0; i< MaxChilds; i++)
        {
            if(pids[i] != 0)
            {
                for(int j = 0; j< MaxChilds; j++)
                {
                    if(pids[i] == shared_memory->Idle_pid_list[j])
                    {
                        shared_memory->Idle_pid_list[j] = 0; //pids와 같은 pid번호를 가진 공유 메모리 idle list를 0으로 바꿈
                        break;
                    }
                }
                break;
            }
        }
    pthread_mutex_unlock(&counter_mutex); //뮤택스 락 풀어주기
    return NULL;

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// doit_nothing                                                             //
//===============================================================================================//
// Input : x                                                                        //
// Output : x                                                               //
// Purpose : nothing                                       //
///////////////////////////////////////////////////////////////////////////////////////////////////
void *doit_nothing(void *vptr) 
{
    int shm_id, val;
    void *shm_addr;
    if((shm_id = shmget((key_t)PORTNO, sizeof(struct ClientInfo), IPC_CREAT | 0666)) == -1)//공유 메모리 생성
    {
        printf("shmget 초기화 fail");//에러시 오류 출력
        return NULL;
    }
    shared_memory = (ClientInfo_list*)shmat(shm_id,NULL,0); //공유 메모리 연결
    pthread_mutex_lock(&counter_mutex); //뮤택스 락 걸기
    
    pthread_mutex_unlock(&counter_mutex); //뮤택스 락 풀어주기
    return NULL;

}
//새로 코드 갈은거
