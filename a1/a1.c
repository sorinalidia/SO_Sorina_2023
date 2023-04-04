#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_PATH 4096
#define MAX_FILTER 4096

void listDir(int *valid, const char *path, int recursive, int size_limit, char *name_pref)
{
    DIR *dir = NULL;
    dir = opendir(path);
    struct dirent *entry;
    char fullPath[MAX_PATH];
    struct stat file_stat;
    // printf("%s\n", path );
    if (dir == NULL)
    {
        printf("Error\ninvalid directory path\n");
        return;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            // char fullPath[MAX_PATH];
            snprintf(fullPath, MAX_PATH, "%s/%s", path, entry->d_name);
            if (lstat(fullPath, &file_stat) == 0)
            {
                if (name_pref == NULL || strncmp(entry->d_name, name_pref, strlen(name_pref)) == 0)
                {

                    if (S_ISREG(file_stat.st_mode))
                    {

                        if (size_limit == 0 || file_stat.st_size < size_limit)
                        {
                            printf("%s\n", fullPath);
                        }
                    }

                    else if (S_ISDIR(file_stat.st_mode))
                    {

                        if (size_limit == 0)
                        {
                            printf("%s\n", fullPath);
                        }
                        if (recursive)
                        {
                            listDir(valid, fullPath, recursive, size_limit, name_pref);
                        }
                    }
                }
            }
        }
    }
    closedir(dir);
}

int main(int argc, char **argv)
{
    char path[MAX_PATH] = "";
    int recursive = 0;
    int size_limit = 0;
    int *valid = 0;
    char name_pref[MAX_FILTER] = "";
    if (argc >= 2)
    {
        if (strcmp(argv[1], "variant") == 0)
        {
            printf("70127\n");
        }
        else if (strcmp(argv[1], "list") == 0)
        {
            printf("%s\n", "SUCCESS");
            for (int i = 2; i < argc; i++)
            {
                if (strncmp(argv[i], "path=", 5) == 0)
                {
                    strcpy(path, argv[i] + 5);
                }
                else if (strcmp(argv[i], "recursive") == 0)
                {
                    recursive = 1;
                }
                else if (strncmp(argv[i], "size_smaller=", 13) == 0)
                {
                    size_limit = atoi(argv[i] + 13);
                }
                else if (strncmp(argv[i], "name_starts_with=", 17) == 0)
                {
                    strcpy(name_pref, argv[i] + 17);
                }
                else
                {
                    printf("ERROR\ninvalid option '%s'\n", argv[i]);
                    return 1;
                }
            }

            listDir(valid, path, recursive, size_limit, name_pref);

            return 0;
        }
        else
        {
            printf("ERROR\ninvalid command '%s'\n", argv[1]);
            return 1;
        }
    }

    return 0;
}