///////////////////////////////////////////////////////////////////////////////////////////////////
// File Name     : spls_advanced.c								 //
// Date          : 2023/04/02									 //
// OS            : Ubuntu 16.04 LTS 64bits							 //
// Author	 : Jang hyung beom								 //
// Student ID 	 : 2019202031									 //
//-----------------------------------------------------------------------------------------------//
// Title : System Programming Assignment #1-2							 //
// Description : Add -a, -l option in ls command						 //
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// sorting											 //
//===============================================================================================//
// Input : files list, the number of files							 //
// Output : x (just sorting)									 //
// Purpose : sort alphabetical order								 //
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// print_file_info										 //
//===============================================================================================//
// Input : int - total(total file size)								 //
//	   mode_t - mode									 //
//	   struct dirent* -> file structure							 //
//	   char* dir_path -> file path								 //
//         int lflag -> -l option is ture or false						 //
//  	   char **filenames -> files list							 //
//	   int num_files -> number of files							 //
// Output : x(just print)									 //
// Purpose : print 										 //
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// print_filetype										 //
//===============================================================================================//
// Input : mode_t mode -> struct								 //
// 	   int l -> l option									 //
// Output : x (just print)									 //
// Purpose : Distinguish whether it is a file or a directory					 //
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// ls												 //
//===============================================================================================//
// Input : char* dirpath -> directory path							 //
// 	   int a -> a option									 //
// 	   int l -> l option									 //
// Output : x											 //
// Purpose : process by file option								 //
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// ls2												 //
//===============================================================================================//
// Input : char* dirpath -> directory path							 //
// Output : x											 //
// Purpose : error code print									 //
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// no_dir											 //
//===============================================================================================//
// Input : struct stat *sb -> struct								 //
// 	   char* filename -> files list								 //
// Output : x											 //
// Purpose : -a optional File output								 //
///////////////////////////////////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
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

void sorting(char** filenames, int num_files);
void print_file_info(int total, mode_t mode, struct dirent* dp, char* dir_path, int lflag, char** filenames, int num_files);
void print_filetype(mode_t mode, int l);
void ls(char* dirpath, int a, int l);
void ls2(char* dirpath);
void no_dir(struct stat* sb, char* filename);
int main(int argc, char** argv)
{
    char* dirpath;
    dirpath = malloc(1000);
    int a = 0, l = 0;
    struct stat stat_buf;
    if (argc == 1) // there are no options and no paths
    {
        dirpath = ".";
        ls(dirpath, a, l);
    }
    else if (argc == 2) //option only or path only
    {
        if (argv[1][0] == '-')
        {
            int c = 0;
            while ((c = getopt(argc, argv, "al")) != -1)
            {
                switch (c)
                {
                case 'a':
                    a = 1;
                    break;
                case 'l':
                    l = 1;
                    break;
                case '?':
                    printf("that is no option\n");
                    break;
                }
            }
            if (getcwd(dirpath, 1000) == NULL)
            {
                printf("cannot access %s : No such file or directory\n", dirpath);
                exit(1);
            }
            ls(dirpath, a, l);
        }
        else
        {
            ls(argv[1], a, l);
        }
    }
    else //if you have options and paths
    {
        if (argv[1][0] == '-')
        {
            int c = 0;
            while ((c = getopt(argc, argv, "al")) != -1)
            {
                switch (c)
                {
                case 'a':
                    a = 1;
                    break;
                case 'l':
                    l = 1;
                    break;
                case '?':
                    printf("that is no option \n");
                    break;
                }
            }
            for (int i = 2; i < argc; i++) //output error code first
            {
                dirpath = argv[i];
                ls2(dirpath);
            }
            for (int i = 2; i < argc; i++)
            {
                dirpath = argv[i];
                ls(dirpath, a, l);
            }
        }
        else
        {
            for (int i = 1; i < argc; i++) //output error code first
            {
                ls2(argv[i]);
            }
            for (int i = 1; i < argc; i++)
            {
                ls(argv[i], a, l);
            }
        }
    }
    return 0;
}

void no_dir(struct stat* sb, char* filename)
{
    printf("%c", S_ISDIR(sb->st_mode) ? 'd' : '-'); //File Option Output
    printf("%c", (sb->st_mode & S_IRUSR) ? 'r' : '-');
    printf("%c", (sb->st_mode & S_IWUSR) ? 'w' : '-');
    printf("%c", (sb->st_mode & S_IXUSR) ? 'x' : '-');
    printf("%c", (sb->st_mode & S_IRGRP) ? 'r' : '-');
    printf("%c", (sb->st_mode & S_IWGRP) ? 'w' : '-');
    printf("%c", (sb->st_mode & S_IXGRP) ? 'x' : '-');
    printf("%c", (sb->st_mode & S_IROTH) ? 'r' : '-');
    printf("%c", (sb->st_mode & S_IWOTH) ? 'w' : '-');
    printf("%c  ", (sb->st_mode & S_IXOTH) ? 'x' : '-');
    printf("%ld ", (long)sb->st_nlink);
    struct passwd* pw = getpwuid(sb->st_uid);   // owner name
    if (pw == NULL)
    {
        printf("\t%d ", sb->st_uid);
    }
    else
    {
        printf("\t%s ", pw->pw_name);
    }
    struct group* gr = getgrgid(sb->st_gid);    // group name
    if (gr == NULL)
    {
        printf("\t%d ", sb->st_gid);
    }
    else
    {
        printf("\t%s ", gr->gr_name);
    }
    printf("%5ld ", (long)sb->st_size); // file size
    char* access_time = ctime(&sb->st_mtime);
    access_time[strlen(access_time) - 1] = '\0';
    printf("\t%s\t%s\n", access_time, filename);
}

void ls2(char* dirpath)
{
    DIR* dirp;
    char** filenames = malloc(sizeof(char*) * 1000);

    int num_files = 0;
    struct stat s;
    char* ss = dirpath;
    if (stat(ss, &s) == 0) //if not directory
    {
        if (s.st_mode & S_IFREG)
        {
            return;
        }
    }
    else if ((dirp = opendir(dirpath)) == NULL) //error print
    {
        printf("cannot access %s: No such file or directory\n", dirpath);
    }
}

void ls(char* dirpath, int a, int l)
{
    struct dirent* direntp;
    DIR* dirp;
    char** filenames = malloc(sizeof(char*) * 1000);
    struct stat stats[1000];
    int total = 0;
    struct stat statbuf;
    int num_files = 0;
    struct stat s;
    char* ss = dirpath;

    if ((dirp = opendir(dirpath)) != NULL) //opendir function
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
            filenames[num_files] = strdup(direntp->d_name);
            num_files++;
        }
        sorting(filenames, num_files);
        if (stat(dirpath, &statbuf) == -1)
        {
            printf("cannot access %s : No such file or directory\n", direntp->d_name);
        }
        else
        {
            print_file_info(total, statbuf.st_mode, direntp, dirpath, l, filenames, num_files);
            closedir(dirp);
        }
    }
    else if (stat(ss, &s) == 0)// if not directory
    {
        if (s.st_mode & S_IFREG)
        {
            if (l == 1)
            {
                no_dir(&s, dirpath);
            }
            else printf("%s\n", ss);
        }
    }
}

void sorting(char** filenames, int num_files) {
    for (int i = 0; i < num_files - 1; i++) //sort alphabetically
    {
        for (int j = i + 1; j < num_files; j++) {
            char* name1 = filenames[i];
            char* name2 = filenames[j];
            int k = 0;
            int dota = 0, dotb = 0;
            if(name1[k] == '.')
            {
                dota = 1;
            }
            if(name2[k] == '.')
            {
                dotb = 1;
            }
            while (tolower(name1[k+dota]) == tolower(name2[k+dotb])) {
                if (name1[k] == '\0' || name2[k] == '\0') {
                    break;
                }
                k++;
            }
            if (tolower(name1[k+dota]) > tolower(name2[k+dotb])) //if words are not arranged lexicographically
            {
                filenames[i] = name2;
                filenames[j] = name1;
            }
        }
    }

}


void print_filetype(mode_t mode, int l)
{
    if (l == 1) //if l option 1 print filetype
    {
        if (S_ISREG(mode)) printf("-");
        else if (S_ISDIR(mode)) printf("d");
        else if (S_ISCHR(mode)) printf("c");
        else if (S_ISBLK(mode)) printf("b");
        else if (S_ISLNK(mode)) printf("l");
        else if (S_ISSOCK(mode)) printf("s");
        else if (S_ISFIFO(mode)) printf("p");
    }
}

void print_file_info(int total, mode_t mode, struct dirent* dp, char* dir_path, int lflag, char** filenames, int num_files)
{
    if (lflag) // l option print
    {
        DIR* dir;
        struct dirent* entry;
        char ap_path[1000];
        struct stat sb;
        if (total != 0)
        {   
            realpath(dir_path,ap_path);
            printf("Directory path: %s\n", ap_path);
            printf("total : %d\n", total / 2);
        }
        for (int i = 0; i < num_files; i++)
        {
            dir = opendir(dir_path);
            while ((dp = readdir(dir)) != NULL)
            { 
                if (strcmp(filenames[i], dp->d_name) == 0)
                {
                    if (stat(dir_path, &sb) == -1)
                    {
                        printf("에러 file info\n");
                    }
                    print_filetype(mode, lflag);
                    printf((sb.st_mode & S_IRUSR) ? "r" : "-");
                    printf((sb.st_mode & S_IWUSR) ? "w" : "-");
                    printf((sb.st_mode & S_IXUSR) ? "x" : "-");
                    printf((sb.st_mode & S_IRGRP) ? "r" : "-");
                    printf((sb.st_mode & S_IWGRP) ? "w" : "-");
                    printf((sb.st_mode & S_IXGRP) ? "x" : "-");
                    printf((sb.st_mode & S_IROTH) ? "r" : "-");
                    printf((sb.st_mode & S_IWOTH) ? "w" : "-");
                    printf((sb.st_mode & S_IXOTH) ? "x" : "-");
                    printf("\t%ld", (long)sb.st_nlink);
                    struct passwd* pw = getpwuid(sb.st_uid);
                    struct group* grp = getgrgid(sb.st_gid);

                    printf("\t%s\t%s\t%ld ", pw->pw_name, grp->gr_name, (long)sb.st_size);
                    char* access_time = ctime(&sb.st_mtime);
                    access_time[strlen(access_time) - 1] = '\0';
                    char timebuf[80];
                    struct tm* tm_info = localtime(&sb.st_mtime);
                    strftime(timebuf,80, "%b %d %H:%M", tm_info);
                    printf("\t%s\t%s\n", timebuf, dp->d_name);
                }
            }
            closedir(dir);
        }
    }
    else //just ls option
    {
        for (int i = 0; i < num_files; i++)
        {
            printf("%s\n", filenames[i]);
        }
    }
}
