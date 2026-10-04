#include "lib.h"

int main(void)
{
    int fd;
    struct flock lock;
    char t[25];

    fd = open("verrou.txt", O_RDWR | O_CREAT, 0644); // Create a file with open()

    if(fd < 0)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    write(fd, "Ludovic est verrouiller.\n", 25); // Fill the file with data

    lock.l_type = F_WRLCK; // Exclusive write lock
    lock.l_whence = SEEK_SET;
    lock.l_start = 25 / 2;
    lock.l_len = 4;

    if((fcntl(fd, F_SETLKW, &lock)) == -1) // Set the lock on data 12, 13, 14, 15
    {
        perror("fcntl");
        exit(EXIT_FAILURE);
    } else {
        printf("The lock is on file descriptor: %d\n", fd);
    }

    lseek(fd, 0, SEEK_SET); // Reposition the cursor at the beginning of the file

    read(fd, t, 24); // Read the first 24 characters of the file

    printf("The lock is on file descriptor: %d which contains %s\n", fd, t);

    lock.l_type = F_UNLCK; // Unlock the lock on data 12, 13, 14, 15

    fcntl(fd, F_SETLK, &lock); // Activate the unlocking of the lock on data 12, 13, 14, 15

    close(fd); // Close the file descriptor

    return 0;
}