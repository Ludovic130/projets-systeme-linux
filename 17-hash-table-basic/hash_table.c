#include "lib.h"

#define NAME_SIZE 256
#define HASH_SIZE 10

typedef struct hash_t
{
    char name[NAME_SIZE];
    struct hash_t *next;
}person;

person *hash_table[HASH_SIZE];

unsigned int hash(char *name)
{
    int lenght = strnlen(name, NAME_SIZE);
    int hash_val = 0;
    for(int i = 0; i < lenght; i++)
    {
        hash_val = (hash_val * 31 + name[i]) % HASH_SIZE;
    }
    return hash_val;
}

void init_table()
{
    for (int i = 0; i < HASH_SIZE; i++)
    {
        hash_table[i] = NULL; // Set them to NULL when empty to avoid segfault
    }  
}

void insert_table(person *p)
{
    int index = hash(p->name); // Find its index by computing its index
    if(hash_table[index] == NULL)
    {
        hash_table[index] = p; // store the value at the position of the index
        p->next = NULL;
    } else
    {
        p->next = hash_table[index]; /* 
                                     * |  head -> name
                                     * |       -> *next -> new head -> old head name
                                     * |                            -> old head *next -> next 
                                     */ 
        hash_table[index] = p; // Set the new structure at the head
    }
}

void search_table(person *p)
{
    int index = hash(p->name);
    person *current = hash_table[index];

    while(current != NULL)
    {
        if(strcmp(current->name, p->name) == 0) // Compare both elements, if equal enter the condition
        {
            printf("the name %s was found at position %d\n",p->name, index);
            return;
        }
        current = current->next; // Otherwise move to the next
    }
    printf("The name %s was not found", p->name);
}

void print_table()
{
    for(int i = 0; i < HASH_SIZE; i++)
    {
        if(hash_table[i] == NULL)
        {
            fprintf(stdout, "\t%d\t----\n", i);
        } else {
            if(hash_table[i]->next == NULL)
            {
                fprintf(stdout,"\t%d\t%s\n", i, hash_table[i]->name);
                continue;
            } else {
                person *curr = hash_table[i];
                printf("\t%d", i);
                while (curr != NULL)
                {
                    fprintf(stdout,"\t%s\t", curr->name); 
                    if(curr->next != NULL)
                    {
                        printf(" -> ");
                    } else {
                        printf("\n");
                    }
                    curr = curr->next;
                    
                }
            }
            
        }
    }
}

int main(void)
{
    person ludovic = {.name = "ludovic"};
    person marcelin = {.name = "marcelin"};
    person sara = {.name = "sara"};
    person albert = {.name = "albert"};
    person celestin = {.name = "celestin"};
    person max = {.name = "max"};
    person alex = {.name = "alex"};
    person gille = {.name = "gille"};
    person diana = {.name = "diana"};

    init_table();

    insert_table(&ludovic);
    insert_table(&marcelin);
    insert_table(&sara);
    insert_table(&albert);
    insert_table(&celestin);
    insert_table(&max);
    insert_table(&alex);
    insert_table(&gille);
    insert_table(&diana);


    search_table(&marcelin);

    print_table();

    return 0;
}