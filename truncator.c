#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>

/* Read all sbao files and truncate them to ogg */
int main(int argc, char **argv)
{
    DIR* FD;
    DIR* OUT;
    struct dirent* in_file;
    FILE    *output_file;
    FILE    *entry_file;
    char    buffer[5];

    buffer[4] = 0;

    char* in_dir = argv[1];
    char* out_dir = "./output/";

    /* Scanning the in directory */
    if (NULL == (FD = opendir (in_dir))) 
    {
        fprintf(stderr, "Error : Failed to open input directory - %s\n", strerror(errno));

        return 1;
    }

    if (NULL == (OUT = opendir (out_dir))) 
    {
        if (ENOENT == errno) 
            mkdir(out_dir);
        else {
            fprintf(stderr, "Error : Failed to open input directory - %s\n", strerror(errno));

            return 1;
        }
    }

    while ((in_file = readdir(FD))) 
    {
        /* On linux/Unix we don't want current and parent directories
         * On windows machine too, thanks Greg Hewgill
         */
        if (!strcmp (in_file->d_name, "."))
            continue;
        if (!strcmp (in_file->d_name, ".."))    
            continue;

        char* dot = in_file->d_name;
        char* path = calloc(255, sizeof(char));
        
        strcpy(path, in_dir);
        strcpy(path + strlen(in_dir), in_file->d_name);

        while (strchr(dot + 1, '.') != NULL)
            dot = strchr(dot + 1, '.');

        if (!dot || strcmp(dot, ".sbao") != 0)
            continue;

        /* Open directory entry file for common operation */
        /* TODO : change permissions to meet your need! */
        entry_file = fopen(path, "rb");
        if (entry_file == NULL)
        {
            fprintf(stderr, "Error : Failed to open entry file %s - %s\n", path, strerror(errno));

            return 1;
        }

        printf("Begin converting file %s\n", path);

        char * newName;
        
        newName = calloc(255, sizeof(char));
        strcpy(newName, "./output/");
        memcpy(newName + 9, in_file->d_name, dot - in_file->d_name);
        strcpy(newName + (dot - in_file->d_name) + 9, ".ogg");


        output_file = fopen(newName, "wb");
        if (output_file == NULL)
        {
            fprintf(stderr, "Error : Failed to open output_file - %s\n", strerror(errno));
            fclose(entry_file);

            return 1;
        }
        /* Doing some struf with entry_file : */
        /* For example use fgets */

        unsigned int header = 0;
        char c;
        while (!feof(entry_file))
        {
            c = fgetc(entry_file);
            if (header) fputc(c, output_file);
            else {
                for(int i = 3; i > 0 ; i--) 
                    buffer[i] = buffer[i - 1];

                buffer[0] = c;
                if (strcmp(buffer, "SggO") == 0) {
                    header = 1;
                    fputs("OggS", output_file);
                    for(int i = 0; i < 5; i++) buffer[i] = 0;
                }
            }
        }

        /* When you finish with the file, close it */
        free(path);
        free(newName);
        fclose(output_file);
        fclose(entry_file);
    }

    return 0;
}