#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>

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

// typedef struct header
// {
//     char magic[3];
//     short int eader_size, version;
//     char num_sections;
//     section_header_t *section_headers;
// } sf_header_t;

void listDir(const char *path, int recursive, int size_limit, char *name_pref)
{
    DIR *dir = NULL;
    dir = opendir(path);
    struct dirent *entry;
    char fullPath[MAX_PATH];
    struct stat file_stat;
    if (dir == NULL)
    {
        printf("Error\ninvalid directory path\n");
        return;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
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
                            listDir(fullPath, recursive, size_limit, name_pref);
                        }
                    }
                }
            }
        }
    }
    closedir(dir);
}

int checkTypes(int fd, char num_sections, section_header_t *sh)
{

    int check = 0;
    for (int i = 0; i < num_sections; i++)
    {
        if (read(fd, &sh[i].name, 9) < 0)
        {
            free(sh);
            close(fd);
            return 0;
        }
        sh[i].name[9] = '\0';

        if (read(fd, &sh[i].type, 1) < 0)
        {
            free(sh);
            close(fd);
            return -5;
        }
        if (read(fd, &sh[i].offset, 4) < 0)
        {
            free(sh);
            close(fd);
            return 0;
        }
        if (read(fd, &sh[i].size, 4) < 0)
        {
            free(sh);
            close(fd);
            return 0;
        }
        if (strnlen(sh[i].name, 9) > 9)
        {
            return 0;
        }
        if (sh[i].type != 16 && sh[i].type != 65 && sh[i].type != 46 && sh[i].type != 34 && sh[i].type != 46 && sh[i].type != 44)
        {
            check = 1;
        }
    }
    return check;
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
    if (read(fd, &num_sections, 1) < 0)
    {
        close(fd);
        return 0;
    }

    if (num_sections < 6 || num_sections > 18)
    {
        close(fd);
        printf("ERROR\nwrong sect_nr\n");
        return 0;
    }

    section_header_t *section_headers = (section_header_t *)malloc(num_sections * sizeof(section_header_t));
    if (section_headers == NULL)
    {
        close(fd);
        return 0;
    }

    int valid = 1;
    int check = checkTypes(fd, num_sections, section_headers);
    for (int i = 0; i < num_sections; i++)
    {
        if (section_headers[i].type != 16 && section_headers[i].type != 65 && section_headers[i].type != 44 && section_headers[i].type != 34 && section_headers[i].type != 46 && section_headers[i].type != 44)
        {
            printf("ERROR\nwrong sect_types");
            free(section_headers);
            close(fd);
            return 0;
        }

        if (valid == 1 && check == 0)
        {
            printf("SUCCESS\n");
            printf("version=%d\n", version);
            printf("nr_sections=%d\n", num_sections);
            valid = 0;
        }

        if (check == 0)
        {
            printf("section%d: %s %d %d\n", i + 1, section_headers[i].name, section_headers[i].type, section_headers[i].size);
        }
    }

    free(section_headers);
    return 1;
}

int isSectionFile(const char *path)
{
    int fd = -1;
    char magic[3];
    short int version, header_size;
    char num_sections;

    fd = open(path, O_RDONLY);
    if (fd == -1)
    {
        close(fd);
        return -1;
    }

    if (read(fd, magic, 2) < 0)
    {
        return -1;
    }

    magic[2] = '\0';
    if (strcmp(magic, "nQ") != 0)
    {
        close(fd);
        return -1;
    }

    if (read(fd, &header_size, 2) < 0)
    {
        close(fd);
        return -1;
    }

    if (read(fd, &version, 2) < 0)
    {
        close(fd);
        return -1;
    }
    if (version < 122 || version > 143)
    {
        close(fd);
        return -1;
    }
    if (read(fd, &num_sections, 1) < 0)
    {
        close(fd);
        return -1;
    }

    if (num_sections < 6 || num_sections > 18)
    {
        close(fd);
        return -1;
    }

    section_header_t *section_headers = (section_header_t *)malloc(num_sections * sizeof(section_header_t));
    if (section_headers == NULL)
    {
        close(fd);
        return -1;
    }

    int k = 0;
    int check = checkTypes(fd, num_sections, section_headers);
    for (int i = 0; i < num_sections; i++)
    {
        if (section_headers[i].type == 44)
        {
            k++;
        }
        if (section_headers[i].type != 16 && section_headers[i].type != 65 && section_headers[i].type != 46 && section_headers[i].type != 34 && section_headers[i].type != 46 && section_headers[i].type != 44)
        {
            free(section_headers);
            return -1;
        }
    }
    if (k < 2 || check == 1)
    {
        free(section_headers);
        return 0;
    }

    free(section_headers);
    return 1;
}

void findAllSF(const char *path, int *valid)
{
    DIR *dir = NULL;
    dir = opendir(path);
    struct dirent *entry;
    char fullPath[MAX_PATH];
    struct stat file_stat;

    if (dir == NULL)
    {
        printf("Error\ninvalid directory path\n");
        return;
    }
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            snprintf(fullPath, MAX_PATH, "%s/%s", path, entry->d_name);
            if (lstat(fullPath, &file_stat) == 0)
            {
                if (S_ISREG(file_stat.st_mode))
                {
                    if (isSectionFile(fullPath) == 1)
                    {
                        printf("%s\n", fullPath);
                    }
                }

                else if (S_ISDIR(file_stat.st_mode))
                {
                    findAllSF(fullPath, valid);
                }
            }
        }
    }
    closedir(dir);
}

void extractLine(const char *path, int section, int line)
{
    int fd = -1;
    char magic[3];
    short int version, header_size;
    char num_sections;

    fd = open(path, O_RDONLY);
    if (fd == -1)
    {
        close(fd);
        printf("Could not open file\n");

        return;
    }

    if (read(fd, magic, 2) < 0)
    {
        close(fd);
        return;
    }

    magic[2] = '\0';
    if (strcmp(magic, "nQ") != 0)
    {
        close(fd);
        return;
    }

    if (read(fd, &header_size, 2) < 0)
    {
        close(fd);
        return;
    }

    if (read(fd, &version, 2) < 0)
    {
        close(fd);
        return;
    }
    if (version < 122 || version > 143)
    {
        close(fd);
        return;
    }
    if (read(fd, &num_sections, 1) < 0)
    {
        close(fd);
        return;
    }

    if (num_sections < 6 || num_sections > 18)
    {
        close(fd);
        return;
    }

    section_header_t *section_headers = (section_header_t *)malloc(num_sections * sizeof(section_header_t));

    if (section_headers == NULL)
    {
        close(fd);
        return;
    }
    int check = checkTypes(fd, num_sections, section_headers);
    if (check != 0)
    {
        printf("ERROR\ninvalid file\n");
        free(section_headers);
        close(fd);
        return;
    }

    if (section < 1 || section > num_sections)
    {
        printf("ERROR\ninvalid section");
        free(section_headers);
        close(fd);
        return;
    }

    char buffer[2048];

    int offset = section_headers[section - 1].offset;
    int size = section_headers[section - 1].size;

    int bytes_read = 0;
    int line_count = 1;
    while (bytes_read < size)
    {
        int bytes_to_read = sizeof(buffer);
        if (bytes_read + bytes_to_read > size)
        {
            bytes_to_read = size - bytes_read;
        }
        if (lseek(fd, offset + size - bytes_read - bytes_to_read, SEEK_SET) < 0)
        {
            printf("ERROR\n");
            free(section_headers);
            close(fd);
            return;
        }
        int bytes = read(fd, buffer, bytes_to_read);
        if (bytes < 0)
        {
            printf("ERROR\n");
            free(section_headers);
            close(fd);
            return;
        }
        bytes_read += bytes;

        for (int i = bytes - 1; i >= 0; i--)
        {
            if (buffer[i] == '\x0A')
            {
                line_count++;
            }
            if (line_count - 1 == line)
            {
                char *fullLine = strtok(&buffer[i + 1], "\x0A");
                if (fullLine != NULL)
                {
                    printf("SUCCESS\n%s\n", fullLine);
                    free(section_headers);
                    close(fd);
                    return;
                }
                else
                {
                    printf("ERROR\n");
                    free(section_headers);
                    close(fd);
                    return;
                }
            }
        }
    }
}

int main(int argc, char **argv)
{
    char path[MAX_PATH] = "";
    int recursive = 0;
    int size_limit = 0;
    int *valid = 0;
    char section = 0;
    int line = 0;
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

            listDir(path, recursive, size_limit, name_pref);

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
        else if (strcmp(argv[1], "findall") == 0)
        {
            if (strncmp(argv[2], "path=5", 5) == 0)
            {
                strcpy(path, argv[2] + 5);
                printf("SUCCESS\n");
                findAllSF(path, valid);
            }
        }
        else if (strcmp(argv[1], "extract") == 0)
        {
            if (strncmp(argv[2], "path=", 5) == 0)
            {
                strcpy(path, argv[2] + 5);
                if (strncmp(argv[3], "section=", 8) == 0)
                {
                    section = atoi(argv[3] + 8);
                    if (strncmp(argv[4], "line=", 5) == 0)
                    {
                        line = atoi(argv[4] + 5);
                        extractLine(path, section, line);
                    }
                }
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
