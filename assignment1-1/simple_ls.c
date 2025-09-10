////////////////////////////////////////////////////////////////////////////////
// File Name     : 2019202031_simple_ls.c                                     //
// Date		 : 2023/03/25						      //
// OS		 : Ubuntu 16.04 LTS 64bits				      //
// Author	 : Jang hyung beom				 	      //
// Student ID	 : 2019202031						      //
// -------------------------------------------------------------------------- //
// Title : System Programming Assignment #1-1				      //
// Description : Find and print files in the directory 		    	      //
////////////////////////////////////////////////////////////////////////////////



#include<stdio.h>
#include<string.h>
#include<dirent.h>
#include<stdlib.h>
#include<ctype.h>

void sorting(char** filenames, num_files);

int main(int argc, char *argv[])
{
	DIR *dirp;
	struct dirent *direntp;
	
	char *dir_path;//input dir path

	if(argc ==1) 
	{
		dir_path = "."; //if no factors were entered
	}
	else if(argc == 2) 
	{
		dir_path = argv[1]; //if factor is entered
	}
	else
	{
		printf("simple_ls: only one directory path can be processed\n");
		return 0;
	}
	dirp = opendir(dir_path); //open directory and save filename in array
	if(dirp == NULL)
	{
		printf("simple_ls: cannot access: '");
		printf("%s",argv[1]);
		printf("' : No such directory\n");
		return 0;
	}
	
	char **filenames = malloc(sizeof(char *) * 100);//dynamic memory allocate

	int num_files = 0;
	while((direntp = readdir(dirp)) != NULL)
	{
		if(direntp->d_name[0] != '.')
		{
			filenames[num_files] = strdup(direntp->d_name);
			num_files++;
		}
	}
	closedir(dirp);

	
	for(int i = 0; i<num_files; i++)//memory free
	{
		free(filenames[i]);
	}
	return 0;
	
}

void sorting(char** filenames, int num_files) {
	for (int i = 0; i < num_files - 1; i++) //sort alphabetically
	{
		for (int j = i + 1; j < num_files; j++)
		{
			char* name1 = filenames[i];
			char* name2 = filenames[j];
			int k = 0;
			while (toupper(name1[k]) == toupper(name2[k]))
			{
				if (name1[k] == '\0' || name2[k] == '\0')
				{
					break;
				}
				k++;
			}
			if (toupper(name1[k]) > toupper(name2[k])) //if words are not arranged lexicographically
			{
				filenames[i] = name2;
				filenames[j] = name1;

			}
		}
	}
	for (int i = 0; i < num_files; i++)//print filenames
	{
		printf("%s\n", filenames[i]);
	}
}