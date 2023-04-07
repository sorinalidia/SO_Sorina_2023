#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_PATH 4096
#define MAX_FILTER 4096
#define MAX_LENGTH 4096


typedef struct
{
    char name[10];
    char type;
    int offset;
    int size;
} section_header_t;

typedef struct header
{
    char magic[3];
    short int eader_size,version;
    char num_sections;
    section_header_t *section_headers;
} sf_header_t;

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


int sectionFile(const char *path)
{
    int fd = -1;
    char magic[3];
    short int version, header_size;
    char num_sections;
   
    fd = open(path, O_RDONLY);
    if (fd == -1)
    {
        printf("Could not open file\n");
        close(fd);
        return -1;
    }

    if (read(fd, magic, 2) < 0)
    {
        printf("Could not read magic field");
        return -1;
    }

    magic[2] = '\0';
    if (strcmp(magic, "nQ") != 0)
    {
        close(fd);
        printf("ERROR\nwrong magic");
        return -1;
    }

    if (read(fd, &header_size, 2) < 0)
    {
        close(fd);
        return 0;
    }

    if (read(fd, &version, 2) < 0)
    {
        close(fd);
        return 0;
    }
    if (version < 122 || version > 143)
    {
        close(fd);
        printf("ERROR\nwrong version");
        return -3;
    }
    if(read(fd,&num_sections,1)<0){
        close(fd);
        return 0;
    }

    if (num_sections < 6 || num_sections > 18)
    {
        close(fd);
        printf("ERROR\nwrong sect_nr\n");
        return 0;
    }

    printf("SUCCESS\n");
    printf("version=%d\n", version);
    printf("nr_sections=%d\n", num_sections);

    section_header_t *section_headers =(section_header_t*)malloc(num_sections * sizeof(section_header_t));
    if (section_headers == NULL)
    {
        close(fd);
        return 0;
    }

    for (int i = 0; i < num_sections; i++)
    {
        if (read(fd, &section_headers[i].name, 9) < 0)
        {
            free(section_headers);
            close(fd);
            return 0;
        }
        section_headers[i].name[9] = '\0';

        if (read(fd, &section_headers[i].type, 1) < 0)
        {
            free(section_headers);
            close(fd);
            return -5;
        }
        if (read(fd, &section_headers[i].offset, 4) < 0)
        {
            free(section_headers);
            close(fd);
            return 0;
        }
        if (read(fd, &section_headers[i].size, 4) < 0)
        {
            free(section_headers);
            close(fd);
            return 0;
        }
        if (strnlen(section_headers[i].name, 9) > 9)
        {
            printf("Invalid section name %ld", strnlen(section_headers[i].name, 9));
        }
        if (section_headers[i].type != 16 && section_headers[i].type !=65 &&  section_headers[i].type !=44 &&  section_headers[i].type !=34 && section_headers[i].type !=46 && section_headers[i].type !=44)
        {
            printf("\nERROR\nwrong sect_types");
            free(section_headers);
            close(fd);
            return 0;
        }
        
        printf("section%d: %s %d %d\n", i + 1, section_headers[i].name, section_headers[i].type, section_headers[i].size);
    }

    free(section_headers);
    return 1;

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
        else if (strcmp(argv[1], "parse") == 0)
        {
            if (strncmp(argv[2], "path=", 5) == 0)
            {
                strcpy(path, argv[2] + 5);
                sectionFile(path);
            }
        }
        else
        {
            printf("ERROR\ninvalid command '%s'\n", argv[1]);
            return 1;
        }
    }

    return 0;
}