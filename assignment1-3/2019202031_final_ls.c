///////////////////////////////////////////////////////////////////////////////////////////////////
// File Name     : 2019202031_final_ls.c                        				 //
// Date          : 2023/04/11                            					 //
// OS            : Ubuntu 16.04 LTS 64bits                      				 //
// Author    : Jang hyung beom                         						 //
// Student ID     : 2019202031                            					 //
//-----------------------------------------------------------------------------------------------//
// Title : System Programming Assignment #1-3                      				 //
// Description : Add wild card  option in ls command                   				 //
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// sorting                                  							 //
//===============================================================================================//
// Input : files list, the number of files                      				 //
// Output : x (just sorting)                            					 //
// Purpose : sort alphabetical order                         					 //
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// print_file_info                               						 //
//===============================================================================================//
// Input : int - total(total file size)            					         //
//      mode_t - mode                            						 //	
//      struct dirent* -> file structure                      					 //
//      char* dir_path -> file path                         					 //
//         int lflag -> -l option is ture or false                   				 //
//        char **filenames -> files list                      					 //
//      int num_files -> number of files                      					 //
// Output : x(just print)                            						 //
// Purpose : print                                						 //
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// print_filetype                               						 //
//===============================================================================================//
// Input : mode_t mode -> struct                         					 //
//       int l -> l option                            						 //
// Output : x (just print)                            						 //
// Purpose : Distinguish whether it is a file or a directory                			 //
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// ls                                     							 //
//===============================================================================================//
// Input : char* dirpath -> directory path                      				 //
//       int a -> a option                            						 //
//       int l -> l option                            						 //
// Output : x                                  							 //
// Purpose : process by file option                         					 //
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// ls2                                     							 //
//===============================================================================================//
// Input : char* dirpath -> directory path                     					 //
// Output : x                                 							 //
// Purpose : error code print                           					 //
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// no_dir                                 							 //
//===============================================================================================//
// Input : struct stat *sb -> struct                         					 //
//       char* filename -> files list                        					 //
// Output : x                                  							 //
// Purpose : -a optional File output                        					 //
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// wild_card											 //
//===============================================================================================//
// Input : int argc -> path count								 //
//	   char** argv -> path									 //
// Output : x											 //
// Purpose : wild card check and print								 //
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include<ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/stat.h>
#include <time.h>
#include <grp.h>
#include <glob.h>

void sorting(char** filenames, int num_files, int r, int S,char *dirpath); 
void print_file_info(int h, int total, mode_t mode, struct dirent* dp, char* dir_path, int lflag, char** filenames, int num_files);
void print_filetype(mode_t mode, int l);
void ls(char* dirpath, int a, int l, int r, int h, int S);
void ls2(char* dirpath);
void no_dir(struct stat* sb, char* filename);
void wild_card(int argc, char** argv);
int main(int argc, char** argv)
{
    char* dirpath;
    dirpath = malloc(1000);
    int a = 0, l = 0, h = 0, r = 0, S = 0; //Parsing Flag
    struct stat stat_buf; //stat variable
    int wild = 0; //check wild factor
    char * ap_path;
    for (int i = 1; i < argc; i++)
    {
        for (int j = 0; j < strlen(argv[i]); j++)
        {
            if (argv[i][j] == '*' | argv[i][j] == '?' | argv[i][j] == '[') //if wildcard existence
            {
                wild = 1; //wild flag true
            }
        }
    }
    if(wild) //if wild true
    {
        wild_card(argc, argv); //wild_card function call
    }
    else if (argc == 1) // there are no options and no paths
    {
        dirpath = "."; //set current path
        ls(dirpath, a, l, r, h, S); //ls function call
    }
    else if (argc == 2) //option only or path only
    {
        if (argv[1][0] == '-') //if have an option
        {
            int c = 0;
            while ((c = getopt(argc, argv, "alhrS")) != -1) //parsing start
            {
                switch (c)
                {
                case 'a': //if option a existence
                    a = 1;  //a flag set 1
                    break;
                case 'l': //if option l existence
                    l = 1;  //l flag set 1
                    break;
                case 'h': //if option h existence
                    h = 1;  //h flag set 1
                    break; 
                case 'r': //if option r existence
                    r = 1;  //r flag set 1
                    break;
                case 'S': //if option S existence
                    S = 1;  //S flag set 1
                    break;
                case '?': //Options that do not exist
                    printf("that is no option\n");
                    break;
                }
            }
            getcwd(dirpath, 1000); //get current path
            ls(dirpath, a, l, r, h, S); // ls function call
        }
        else //not exist option
        {
            ls(argv[1], a, l, r, h, S); //ls function call
        }
    }
    else //if you have options and paths
    {
        if (argv[1][0] == '-')
        {
            int c = 0;
            while ((c = getopt(argc, argv, "alhrS")) != -1) //parsing start
            {
                switch (c)
                {
                case 'a': //if option a existence
                    a = 1;  //a flag set 1
                    break;
                case 'l':  //if option l existence
                    l = 1;   //l flag set 1
                    break;
                case 'h': //if option h existence
                    h = 1;  //h flag set 1
                    break;
                case 'r':  //if option r existence
                    r = 1;  //r flag set 1
                    break;
                case 'S':  //if option S existence
                    S = 1;  //S flag set 1
                    break;
                case '?':  //Options that do not exist
                    printf("that is no option \n");
                    break;
                }
            }
            for (int i = 2; i < argc; i++) //output error code first
            {
                dirpath = argv[i];  
                ls2(dirpath);  //ls2 function call
            }
            for (int i = 2; i < argc; i++)
            {
                dirpath = argv[i];
                ls(dirpath, a, l, r, h, S); //ls function call
            }
        }
        else
        {

            for (int i = 1; i < argc; i++) //output error code first
            {
                ls2(argv[i]);   //ls2 function call
            }
            for (int i = 1; i < argc; i++)
            {
                ls(argv[i], a, l, r, h, S); //ls function call
            }
        }
    }
    return 0; //Exit the program
}

void wild_card(int argc,char** argv) //wild_card function
{
    glob_t result;  //glob variable
    struct stat sb; //struct stat
    struct stat sbb; //struct stat
    char **file_name = NULL;  //file name array
    char **dir_name = NULL; //dir name array
    int filek = 0, dirk = 0; //file , dir count
    int wild_flag =0; //wild flag check
    for (int i = 1; i < argc; i++)
    {
        glob(argv[i], 0, NULL, &result);          // use glob function
        for (int j = 0; j < result.gl_pathc; j++) // Obtain the number of files and directories
        {
            if (stat(result.gl_pathv[j], &sb) == 0 && 0 == S_ISDIR(sb.st_mode)) // if file
            {
                filek++; //file count ++
            }
            if (stat(result.gl_pathv[j], &sbb) == 0 && S_ISDIR(sbb.st_mode)) // if dir
            {
                dirk++; //dir count ++
            }
        }
        globfree(&result); //glob memory free
    }
    // Dynamically allocate memory to an array to store file names and directory names
    if (filek > 0)
    {
        file_name = malloc(sizeof(char *) * (filek+300));  //Dynamic memory allocation
    }

    if (dirk > 0)
    {
        dir_name = malloc(sizeof(char *) * (dirk+300));  //Dynamic memory allocation
    }

    // Save files and directories
    int file_idx = 0, dir_idx = 0; //file, dir index count
    for (int km = 1; km < argc; km++)
    {
        glob(argv[km], 0, NULL, &result); //instance glob
        for (int i = 0; i < result.gl_pathc; i++)
        {
            if (stat(result.gl_pathv[i], &sb) == 0 && 0 == S_ISDIR(sb.st_mode)) // if file
            {
                file_name[file_idx] = result.gl_pathv[i]; // save file name
                file_idx++;
                //printf("%s\n\n\n",result.gl_pathv[i]);
            }
            if (stat(result.gl_pathv[i], &sbb) == 0 && S_ISDIR(sbb.st_mode)) // if dir
            {
                dir_name[dir_idx] = result.gl_pathv[i]; // save dir name
                dir_idx++;
            }
        }

    }
    struct stat sst; //stat struct
   
    for(int i = 1; i<argc; i++)//Receive non-wildcard paths
    {
        for(int j = 0; j< strlen(argv[i]); j++)
        {
            if(argv[i][j] == '?' || argv[i][j] == '*' || argv[i][j] == '[') //if the factor is a wildcard
            {
                wild_flag = 1; //wild flag true
            }
        }
        if (!wild_flag) //if not wild factor just path
        {
            if (stat(argv[i], &sst) == 0) //if existence
            {
                if (S_ISDIR(sst.st_mode) == 0) //if file
                {
                    file_name[file_idx] = argv[i]; //push data
                    file_idx++; //index ++
                    filek++; //index ++
                }
                else if (S_ISDIR(sst.st_mode)) //if dir
                {
                    dir_name[dir_idx] = argv[i];//push data
                    dir_idx++; //index ++
                    dirk++; //index ++
                }
            }
        }
    }
    
    if (filek > 0) // print file list
    {
        sorting(file_name, filek, 0, 0, NULL); // sort file list
        for (int i = 0; i < filek; i++)
        {
            printf("%s  \n", file_name[i]); //print
        }
        printf("\n\n"); //print
    }
    if (dirk > 0) //print dir list
    {
        sorting(dir_name, dirk, 0, 0, NULL); //sort dir list
        for (int i = 0; i < dirk; i++)
        {
            DIR* dir; //DIR
            char ap_path[1000]; //path
            struct dirent *dp; //dirent struct
            printf("Directory path: %s\n", dir_name[i]); //print file path
            if(dir_name[i][0] =='/') //if path
            {
                if((dir = opendir(dir_name[i]))==NULL) //if not open
                {
                    continue; //continue
                }
            }
            else //if relative path
            {
                getcwd(ap_path,1000); //add path
                strcat(ap_path,"/"); //add /
                strcat(ap_path,dir_name[i]);//add path
                if((dir = opendir(ap_path)) == NULL)//if not open
                {
                    continue;//continue
                }
            }
            while ((dp = readdir(dir)) != NULL) //read file
            {
                if(dp->d_name[0] == '.') //if hidden file
                {
                    continue;//coninue
                }
                printf("%s  ",dp->d_name);//print
            }
            printf("\n\n");//print
            closedir(dir);// close dir
        }
    }    
}
void no_dir(struct stat* sb, char* filename)
{
    printf("%c", S_ISDIR(sb->st_mode) ? 'd' : '-'); //File Option Output
    printf("%c", (sb->st_mode & S_IRUSR) ? 'r' : '-'); //print r or -
    printf("%c", (sb->st_mode & S_IWUSR) ? 'w' : '-'); //print w or -
    printf("%c", (sb->st_mode & S_IXUSR) ? 'x' : '-'); //print x or -
    printf("%c", (sb->st_mode & S_IRGRP) ? 'r' : '-'); //print r or -
    printf("%c", (sb->st_mode & S_IWGRP) ? 'w' : '-'); //print w or -
    printf("%c", (sb->st_mode & S_IXGRP) ? 'x' : '-'); //print x or -
    printf("%c", (sb->st_mode & S_IROTH) ? 'r' : '-'); //print r or -
    printf("%c", (sb->st_mode & S_IWOTH) ? 'w' : '-'); //print w or -
    printf("%c  ", (sb->st_mode & S_IXOTH) ? 'x' : '-'); //print x or -
    printf("%ld ", (long)sb->st_nlink); //print
    struct passwd* pw = getpwuid(sb->st_uid);   // owner name
    if (pw == NULL) //if pw NULL
    {
        printf("\t%d ", sb->st_uid); //st_uid print
    }
    else
    {
        printf("\t%s ", pw->pw_name); //pw name print
    }
    struct group* gr = getgrgid(sb->st_gid);    // group name
    if (gr == NULL) //if gr NULL
    {
        printf("\t%d ", sb->st_gid); //print sb->stgid
    }
    else
    {
        printf("\t%s ", gr->gr_name); //print group name
    }
    printf("%5ld ", (long)sb->st_size); // file size
    char* access_time = ctime(&sb->st_mtime);
    access_time[strlen(access_time) - 1] = '\0';
    printf("\t%s\t%s\n", access_time, filename);
}

void ls2(char* dirpath)
{
    DIR* dirp; //DIR
    char** filenames = malloc(sizeof(char*) * 1000); //file name array

    int num_files = 0; //file count
    struct stat s; //stat
    char* ss = dirpath; //dirpath
    if (stat(ss, &s) == 0) //if not directory
    {
        if (s.st_mode & S_IFREG) //if file
        {
            return; //return
        }
    }
    else if ((dirp = opendir(dirpath)) == NULL) //error print
    {
        printf("cannot access %s: No such file or directory\n", dirpath); //print error code
    }
}

void ls(char *dirpath, int a, int l, int r, int h, int S)
{
    struct dirent *direntp;//dirent struct
    DIR *dirp; //DIR
    char **filenames = malloc(sizeof(char *) * 1000); //memory allocate
    struct stat stats[1000]; //stat struct
    int total = 0;//total
    struct stat statbuf; //stat struct
    int num_files = 0; //file count
    struct stat s; //stat struct
    char *ss = dirpath; //path

    if ((dirp = opendir(dirpath)) != NULL) // opendir function
    {
        while ((direntp = readdir(dirp)) != NULL) // readdir function
        {
            struct stat k;
            stat(direntp->d_name, &k); // stat function
            if (a == 0)
            {
                if (direntp->d_name[0] == '.') // if not a option
                {
                    continue;
                }
            }
            total += k.st_blocks; // total size check
            filenames[num_files] = strdup(direntp->d_name); //push data
            num_files++; //num count ++
        }
        sorting(filenames, num_files, r, S,dirpath); //sorting filename list
        if (stat(dirpath, &statbuf) == -1) //if error
        {
            printf("cannot access %s : No such file or directory\n", direntp->d_name); //print error
        }
        else
        {
            print_file_info(h, total, statbuf.st_mode, direntp, dirpath, l, filenames, num_files); //instance print_file info function
            closedir(dirp); //close dir
        }
    }
    else if (stat(ss, &s) == 0) // if not directory
    {
        if (s.st_mode & S_IFREG) //if file
        {
            if (l == 1) //if l option
            {
                no_dir(&s, dirpath); //call no_dir function
            }
            else
                printf("%s\n", ss);//print
        }
    }
}

void sorting(char **filenames, int num_files, int r, int S, char *dir_path)
{
    for (int i = 0; i < num_files - 1; i++) // sort alphabetically
    {
        for (int j = i + 1; j < num_files; j++)
        {
            char *name1 = filenames[i]; //file name
            char *name2 = filenames[j]; //file name
            int k = 0;
            int dota = 0, dotb = 0;
            if (name1[k] == '.') //ignore .
            {
                dota = 1; //dota option 1
            }
            if (name2[k] == '.') //ignore .
            {
                dotb = 1; //dotb option 1
            }
            while (toupper(name1[k + dota]) == toupper(name2[k + dotb])) //if not same break
            {
                if (name1[k] == '\0' || name2[k] == '\0') //if end word
                {
                    break; //break
                }
                k++; //count ++
            }

            if (S == 0 && toupper(name1[k + dota]) > toupper(name2[k + dotb])) // if words are not arranged lexicographically
            {
                filenames[i] = name2; //change
                filenames[j] = name1; //change
            }
        }
    }
    if (S == 1) //S option
    {
        DIR *dir;
        struct dirent *dp;
        struct stat sb;
        int filesize_arr[1000] = {0}; //file size

        struct dirent *entry;
        char ap_path[1000];
        realpath(dir_path, ap_path); //Obtaining the absolute path
        for (int i = 0; i < num_files; i++)
        {
            dir = opendir(ap_path); //dir Open
            while ((dp = readdir(dir)) != NULL)
            {
                realpath(dir_path, ap_path); //Obtaining the absolute path

                if (strcmp(filenames[i], dp->d_name) == 0)
                {
                    strcat(ap_path, "/"); //add path
                    strcat(ap_path, filenames[i]);//add path
                    stat(ap_path, &sb);
                    filesize_arr[i] = sb.st_size; //get file size
                }
            }
        }
        closedir(dir);

        for (int i = 0; i < num_files - 1; i++) // sort alphabetically
        {
            for (int j = i + 1; j < num_files; j++)
            {
                char *name1 = filenames[i]; //file name
                char *name2 = filenames[j];//file name
                if(filesize_arr[i] < filesize_arr[j])
                {
                    double temp;
                    filenames[i] = name2; //change name
                    filenames[j] = name1; //change name
                    temp = filesize_arr[i]; //change size
                    filesize_arr[i] = filesize_arr[j]; //change size
                    filesize_arr[j] = temp; //change size
                }
            }
        }

        if (r == 1) //if r option 1
        {
            for (int i = 0; i < num_files / 2; i++) //reverse list
            {
                char *temp = filenames[i]; 
                int temp1;
                filenames[i] = filenames[num_files - i - 1]; //change
                filenames[num_files - i - 1] = temp; //change
                temp1 = filesize_arr[i]; //change
                filesize_arr[i] = filesize_arr[num_files - i - 1]; //change
                filesize_arr[num_files - i - 1] = temp1; //change
            }
            for (int i = 0; i < num_files - 1; i++) // sort alphabetically
            {
                for (int j = i + 1; j < num_files; j++)
                {
                    char *name1 = filenames[i]; //filename
                    char *name2 = filenames[j]; //filename
                    int k = 0;
                    int dota = 0, dotb = 0;
                    if (name1[k] == '.') //ignore .
                    {
                        dota = 1; //dota 1
                    }
                    if (name2[k] == '.') //ignore .
                    {
                        dotb = 1; //dotb 1
                    }

                    if(filesize_arr[i] != filesize_arr[j]) // if file size not same
                    {
                        continue; //continue
                    }
                    else
                    {
                        while (toupper(name1[k + dota]) == toupper(name2[k + dotb])) //if not same break
                        {
                            if (name1[k] == '\0' || name2[k] == '\0') //end word
                            {
                                break; //break
                            }
                            k++; //count ++
                        }
                        if (toupper(name1[k + dota]) < toupper(name2[k + dotb])) // if words are not arranged lexicographically
                        {
                            filenames[i] = name2; //change
                            filenames[j] = name1; //change
                        }
                    }
                }
            }
        }
    }

    if (S == 0 && r == 1) //if S option 0, r option 1
    {
        for (int i = 0; i < num_files / 2; i++) //just reverse list
        {
            char *temp = filenames[i]; //change
            filenames[i] = filenames[num_files - i - 1];//change
            filenames[num_files - i - 1] = temp;//change
        }
        
    }
}

void print_filetype(mode_t mode, int l)
{
    if (l == 1) //if l option 1 print filetype
    {
        if (S_ISREG(mode)) printf("-"); //print -
        else if (S_ISDIR(mode)) printf("d");//print d
        else if (S_ISCHR(mode)) printf("c");//print c
        else if (S_ISBLK(mode)) printf("b");//print b
        else if (S_ISLNK(mode)) printf("l");//print l
        else if (S_ISSOCK(mode)) printf("s");//print s
        else if (S_ISFIFO(mode)) printf("p");//print p
    }
}

void print_file_info(int h,int total, mode_t mode, struct dirent* dp, char* dir_path, int lflag, char** filenames, int num_files)
{
    if (lflag) // l option print
    {
        DIR* dir;
        struct dirent* entry;
        char ap_path[1000];
        
        struct stat sb;
        if (total != 0) //if total != 0
        {   
            realpath(dir_path,ap_path); //get path
            printf("Directory path: %s\n", ap_path);//print path
            printf("total : %d\n", total / 2);//print total
        }
        for (int i = 0; i < num_files; i++)
        {
            realpath(dir_path,ap_path);//get path
            dir = opendir(ap_path); //open dir
            while ((dp = readdir(dir)) != NULL) //read dir
            { 
                realpath(dir_path,ap_path);//call path
                    
                if (strcmp(filenames[i], dp->d_name) == 0)
                {
                    strcat(ap_path, "/");
                    strcat(ap_path, filenames[i]);
                    
                    if (stat(ap_path, &sb) == -1) //stat
                    {
                        printf("에러 file info\n"); //print error
                    }
                    print_filetype(mode, lflag);
                    printf("%c", S_ISDIR(sb.st_mode) ?'d':'-'); //print d or -
                    printf("%c", sb.st_mode & S_IRUSR ? 'r' : '-');//print r or -
                    printf("%c", sb.st_mode & S_IWUSR ? 'w' : '-');//print w or -
                    printf("%c", sb.st_mode & S_IXUSR ? 'x' : '-');//print x or -
                    printf("%c", sb.st_mode & S_IRGRP ? 'r' : '-');//print r or -
                    printf("%c", sb.st_mode & S_IWGRP ? 'w' : '-');//print w or -
                    printf("%c", sb.st_mode & S_IXGRP ? 'x' : '-');//print x or -
                    printf("%c", sb.st_mode & S_IROTH ? 'r' : '-');//print r or -
                    printf("%c", sb.st_mode & S_IWOTH ? 'w' : '-');//print w or -
                    printf("%c\t", sb.st_mode & S_IXOTH ? 'x' : '-');//print x or -
                    printf("\t%ld", (long)sb.st_nlink);
                    struct passwd* pw = getpwuid(sb.st_uid);
                    struct group* grp = getgrgid(sb.st_gid);

                    printf("\t%s\t%s\t ", pw->pw_name, grp->gr_name);
                    if(h == 1) //if h option 1
                    {
                        long k = sb.st_size;
                        double kkk = (double)k;
                        double check, check2, check3;
                        check2 = k/1024*1024;
                        check3 = k/1024;
                        
                        if(k > 1024* 1024 * 1024) //if G
                        {
                            if(k % (1024*1024*1024) == 0) //If there's no rest
                            {
                                printf("%.0fG",(double)k);
                            }
                            else printf("%.1fG",(double)k/(1024*1024*1024)); //If there's rest
                        }
                        else if(k > 1024*1024) //if M
                        {
                            if(k % (1024*1024) == 0) //If there's no rest
                            {
                                printf("%.0fM",(double)k);
                            }
                            else printf("%.1fM",(double)k/(1024*1024)); //If there's rest
                        }
                        else if(k > 1024 ) //if K
                        {
                            if(k%1024 == 0) //If there's no rest
                            {
                                printf("%.0fK", (double)sb.st_size/1024);
                            }
                            else printf("%.1fK",(double)k/1024); //If there's rest
                        }
                        else
                        {
                            printf("%ld",sb.st_size);
                        }
                    }
                    else
                    {
                        printf("%ld",(long)sb.st_size);
                    }
                    char* access_time = ctime(&sb.st_mtime);
                    access_time[strlen(access_time) - 1] = '\0';
                    char timebuf[80];
                    struct tm* tm_info = localtime(&sb.st_mtime);
                    strftime(timebuf,80, "%b %d %H:%M", tm_info);
                    printf("\t%s\t%s\n", timebuf, dp->d_name); //print
                }
            }

            closedir(dir);//close dir
        }
    }
    else //just ls option
    {
        for (int i = 0; i < num_files; i++)
        {
            printf("%s\n", filenames[i]); //print
        }
    }
}

