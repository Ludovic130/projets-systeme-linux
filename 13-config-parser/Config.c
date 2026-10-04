#include "lib.h"

#define SIZE 50

typedef struct conf
{
    char key[SIZE];
    char val[SIZE];
    struct conf *value;
   
}config_t;

config_t *head = NULL;
config_t *last = NULL;

void load_conf(void)
{
    FILE *fd;
    char c[SIZE];
    // config_t *head;
    char *e;

    fd = fopen("config.txt", "r"); // Normal file creation.

    // 1. Check that the file opened successfully
    if (fd == NULL)
    {
        perror("fopen");
        return;
    }

    // 2. Use sizeof(c) to read up to 100 bytes
    while (fgets(c, sizeof(c), fd) != NULL)
    {

        c[strcspn(c, "\r\n")] = '\0'; // Remove special characters
        if((e = strchr(c, '=')) != NULL)
        {
            *e = '\0';

            char *key = c;
            char *val = e + 1;

            config_t *co = malloc(sizeof(config_t)); // Allocate memory for the new structure

            if(co == NULL) // Check the structure to see if it was allocated correctly
            {
                perror("malloc");
                fclose(fd);
                return;
            }

            // Copy *key and *val into co->key, co->val
            strncpy(co->key, key, sizeof(co->key) - 1); // copy the key into the co structure
            strncpy(co->val, val, sizeof(co->val) - 1); // copy the value into the co structure
            co->value = NULL;

            if (head == NULL)
            {
                head = co; // First node
            } 
            else 
            {
                last->value = co; // Attach it to the PREVIOUS one (the address)
            }

            last = co; // Move last to the address of co
        }
    }
    // Close the file
    fclose(fd);
}

void set_struct(char *key, char *val)
{
    config_t *current = head;
    config_t *last = NULL;

    while (current != NULL)
    {
        if(strcmp(key, current->key) == 0) // Compare the two keys
        {
            strcpy(current->val, val); // If true, copy val into the current structure
            return;
        }
        last = current; // Store the address of current in order to reach the last structure
        current = current->value; // Store the address of current->value to compare and see if the added key and the old key are the same thing
    }

    config_t *new = malloc(sizeof(config_t)); // Dynamically allocate memory for the structure

    if(new == NULL) // Check if the structure was allocated correctly
    {
        perror("new-malloc");
        return;
    }

    strcpy(new->key, key); // copy
    strcpy(new->val, val); // copy
    new->value = NULL;

    if(head == NULL)
    {
        head = new;
    } else {
        last->value = new;
    }
}

void set_conffile()
{
    FILE *f;
    config_t *current = head;
    // char chaine[128];

    f = fopen("config.txt", "w");

    while(current != NULL)
    {
        // Send the key=value pairs to the file.
        fprintf(f, "%s=%s\n", current->key, current->val);
        current = current->value; // Move to the next address
    }

    fclose(f); // Release the file descriptor
}

void load()
{
    // Use 'current' to move around without modifying 'head'
    config_t *current = head;

    // Traverse the list until the end (NULL pointer)
    while (current != NULL)
    {
        printf("%s = %s\n", current->key, current->val);
        // Move to the next node
        current = current->value; // (or current->value depending on your field name)
    }
}
void fr()
{
    config_t *current = head;
    config_t *next_node;
    while (current != NULL)
    {
        next_node = current->value; // Store the next structure
        free(current); // Free the head structure
        current = next_node; // Store the address of next_node which points to current_value
    }  
    head = NULL;
    last = NULL;
}

int main(void)
{
    load_conf();
    set_struct("name", "pawpaw");
    set_struct("firstname", "marc");
    set_struct("game", "mario");
    set_struct("fun", "crazy");
    set_struct("test", "Ok");
    set_struct("price", "200");
    set_conffile();
    load();
    fr();
    
    return 0;
}