#include "lib.h"

void affiche_status(struct stat *status, const char *repertoire)
{
    DIR *dir; // Declare a pointer to a directory
    struct dirent *entry; // Declare a pointer to a dirent structure

    dir = opendir(repertoire); // Open the directory.

    if(dir == NULL)
    {
        return;
    }

    while((entry = readdir(dir)) != NULL) // read the directory contents until the end
    {
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", repertoire, entry->d_name);
        if(stat(path, status) == -1)
        {
            perror("stat");
            return;
        } else {
            // Display information about the file type or directory
            if(S_ISBLK(status->st_mode))
                fprintf(stderr, "Type : block\n");
            else if(S_ISCHR(status->st_mode))
                fprintf(stderr, "Type : character device\n");
            else if(S_ISDIR(status->st_mode))
                fprintf(stderr, "Type : directory\n");
            else if(S_ISFIFO(status->st_mode))
                fprintf(stderr, "Type : FIFO/pipe\n");
            else if(S_ISLNK(status->st_mode))
                fprintf(stderr, "Type : symlink\n");
            else if(S_ISREG(status->st_mode))
                fprintf(stderr, "Type : regular file\n");
            else if(S_ISSOCK(status->st_mode))
                fprintf(stderr, "Type : socket\n");


            // Display the permissions of the file or directory
            fprintf(stderr, "Permissions : ");
            fprintf(stderr, "u:");
            fprintf(stderr, status->st_mode & S_IRUSR ? "r" : "-");
            fprintf(stderr, status->st_mode & S_IWUSR ? "w" : "-");
            fprintf(stderr, status->st_mode & S_IXUSR ? "x" : "-");
            fprintf(stderr, " g:");
            fprintf(stderr, status->st_mode & S_IRGRP ? "r" : "-");
            fprintf(stderr, status->st_mode & S_IWGRP ? "w" : "-");
            fprintf(stderr, status->st_mode & S_IXGRP ? "x" : "-");
            fprintf(stderr, " o:");
            fprintf(stderr, status->st_mode & S_IROTH ? "r" : "-");
            fprintf(stderr, status->st_mode & S_IWOTH ? "w" : "-");
            fprintf(stderr, status->st_mode & S_IXOTH ? "x" : "-");
            fprintf(stderr, "\n");

            // Display the size of the file or directory
            fprintf(stderr, "Taille : ");
            fprintf(stderr, "%ld octets\n", (long)status->st_size);
        } 

        // Display the name of the file or directory
        fprintf(stderr, "Nom : ");
        fprintf(stdout, "%s\n",entry->d_name); // Display the inode number and the entry name
    }
    fprintf(stdout, "\n");

    closedir(dir); // Close the directory

}

int main(int argc, char const *argv[])
{
    struct stat status; // Declare a stat structure to store information about the file or directory

    if(argc == 1)
    {
        affiche_status(&status, ".");
    } else {
        for(int i = 1; i < argc; i++)
        {
            fprintf(stderr, "%s : ", argv[i]);
            affiche_status(&status, argv[i]);
        }
    }
    return EXIT_SUCCESS;
}