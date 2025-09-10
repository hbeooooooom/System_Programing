///////////////////////////////////////////////////////////////////////////////////////////////////
// File Name     : 2019202031_adv_server.c                                     //
// Date          : 2023/05/13                                            //
// OS            : Ubuntu 16.04 LTS 64bits                                   //
// Author    : Jang hyung beom                                            //
// Student ID     : 2019202031                                            //
//-----------------------------------------------------------------------------------------------//
// Title : System Programming Assignment #2-3                                   //
// Description : multi access server                                //
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
#include<errno.h>
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


struct ClientInfo { //client information structure
    int number;
    char ip[16];
    int port;
    pid_t pid;
    time_t time;
};
struct ClientInfo history[MAX_HISTORY]; // save array

///////////////////////////////////////////////////////////////////////////////////////////////////
// alarmHandler                                     //
//===============================================================================================//
// Input :  int signum -> signal                                                       //
// Output : print value                                  //
// Purpose : handler                                       //
///////////////////////////////////////////////////////////////////////////////////////////////////
void alarmHandler(int signum)
{
    if (signum == SIGINT) // if parent process and signal is SIGINT
    {
        for (int i = 0; i < 5; i++)
        {
            time_t curtime = time(NULL); //get time
            char *curtime_str = ctime(&curtime); //get time
            curtime_str[strcspn(curtime_str, "\n")] = '\0'; // Replace open characters with null characters
            printf("\n[%s] %d Process is terminated.", curtime_str, pids[i]);

            kill(pids[i], SIGTERM);

        }
            time_t curtime = time(NULL);                           // get time
            char *curtime_str = ctime(&curtime);                   // get time
            curtime_str[strcspn(curtime_str, "\n")] = '\0';        // Replace open characters with null characters
            printf("\n[%s] Server is terminated.\n", curtime_str); // print
            exit(0);                                               // parent process termination
    }
    else if (signum == SIGCHLD)
    {
        pid_t child_pid;                                  // child pid variable
        int status;                                       // status
        while ((child_pid = waitpid(-1, &status, 0)) > 0) // retrieve one's child
        {
        }
    }
    else if (signum == SIGALRM) // if parent process alarm
    {
        printf("============ Connection History ===========================================\nNo.\tIP\t\tPID\tPORT\tTIME\n");
        for (int i = 0; i < 5; i++)
        {
            kill(pids[i], SIGUSR1);
        }
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
            exit(0);
    }
    else if(signum == SIGUSR1)
        printHistory(); //print history
}

int main(int argc, char **argv)
{
    for (int i = 0; i < MAX_HISTORY; i++) {
        history[i].number = -1; //initialize
    }
    parent_pid = getpid();
    signal(SIGINT, alarmHandler); //signal setting
    signal(SIGCHLD, alarmHandler);//signal setting
    signal(SIGALRM, alarmHandler); //signal setting
    alarm(10); //set alarm time
    struct sockaddr_in server_addr, client_addr; //struct variable
    int socket_fd, client_fd;                    //int variable
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

    listen(socket_fd, 5); //listen
    pids = (pid_t*)malloc(5*sizeof(pid_t));
    int addrlen = sizeof(client_addr);
    for(int i = 0; i<5; i++)
    {
        pids[i] = child_make(i,socket_fd, addrlen);
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

    time_t curtime = time(NULL);
    char* curtime_str = ctime(&curtime);
    curtime_str[strcspn(curtime_str, "\n")] = '\0'; 
    printf("[%s] %d Process is forked.\n", curtime_str, getpid());

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
    char buf[BUFSIZE];
    socklen_t chilen;
    struct sockaddr_in client_addr;
    signal(SIGINT, SIG_IGN);
    signal(SIGUSR1, child_alarmHandler);
    signal(SIGTERM, child_alarmHandler);
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
        IP1 = inet_ntoa(inet_client_address); // IP 주소 가져오
        puts("=============== New Client ===============");
        time_t curtime = time(NULL);                    // get time
        char *curtime_str = ctime(&curtime);            // get time
        curtime_str[strcspn(curtime_str, "\n")] = '\0'; // Replace open characters with null characters
        printf("[%s] \n", curtime_str);               // print
        printf("IP : %s\n", IP1);                       // print IP
        printf("Port : %d\n", client_addr.sin_port);    // print Port
        puts("==========================================");
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
        printf("========== Disconnected Client ===========\n");
        printf("[%s] \n", curtime_str);  
        printf("IP : %s\n", IP1);                    // print IP number
        printf("Port : %d\n", client_addr.sin_port); // print Port number
        puts("==========================================");
        addHistory(IP1, client_addr.sin_port, getpid()); // add history
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
    if (numRequests < 10) { //process number < 10
        for (int i = 0; i < numRequests; i++) {
            int index = (numRequests - 1 - i) % MAX_HISTORY;
            if (history[index].number != -1) { //if number is not -1
                printf("%d\t%s\t%d\t%d\t%s", history[index].number, history[index].ip, history[index].pid, history[index].port, ctime(&(history[index].time))); //print
            }
        }
    }
    else
    {
        for (int i = 0; i < MAX_HISTORY; i++) 
        {
            int index = (10 - 1 - i) % MAX_HISTORY; //index number setting
            printf("%d\t%s\t%d\t%d\t%s", history[index].number, history[index].ip, history[index].pid, history[index].port, ctime(&(history[index].time))); //print
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
// addHistory                                        //
//===============================================================================================//
// Input : char *ip -> ip address                  //
//         int port -> port number                  //
//         pid_t pid -> pid number                      //
// Output : x                                     //
// Purpose : add process info                            //
///////////////////////////////////////////////////////////////////////////////////////////////////
void addHistory(char *ip, int port, pid_t pid) {
    if (numRequests < MAX_HISTORY) {
        struct ClientInfo *history_entry = &history[numRequests];
        history_entry->number = numRequests + 1;
        strcpy(history_entry->ip, ip);//input ip
        history_entry->port = port; //input port number
        history_entry->pid = pid; //input pid number
        history_entry->time = time(NULL); //input time
     
    } else {
        for (int i = 0; i < MAX_HISTORY - 1; i++) {// Shift the history to make room for the new entry
            history[i] = history[i + 1]; //shift
        }
        struct ClientInfo *last_history = &history[MAX_HISTORY - 1]; 
        last_history->number = numRequests + 1;
        strcpy(last_history->ip, ip); //input ip address
        last_history->port = port; //input port number
        last_history->pid = pid; //input pid number
        last_history->time = time(NULL); //input time
    }
    numRequests++;
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
