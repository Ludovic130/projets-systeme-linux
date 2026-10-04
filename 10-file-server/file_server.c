#include "lib.h"

#define PORT 8081
#define BUFFER_SIZE 1024

// Structure to keep track of what a client has opened
typedef struct {
    int client_fd;          // The client's socket
    int file_fd;            // The descriptor of the opened file (-1 if none)
    char filename[256];     // The name of the opened file
} client_context_t;

// Function to lock a part (or all) of the file
int verrouiller_fichier(int fd, short type) 
{
    struct flock lock;
    
    lock.l_type = type;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0; // (to lock the whole file)
    
    // Call F_SETLKW (blocking)
    if (fcntl(fd, F_SETLKW, &lock) == -1) 
    { 
        perror("fcntl lock"); 
        return -1; 
    }
    return 0;
}

// Function to unlock the file
int deverrouiller_fichier(int fd) 
{
    struct flock lock;
    lock.l_type = F_UNLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0; // 0 means "until the end of the file"

    if(fcntl(fd, F_SETLK, &lock) == -1)
    {
        perror("fcntl unlock");
        return -1;
    }
    return 0;
}

// Function that processes a client's command
void traiter_commande(client_context_t *ctx, char *commande) 
{
    
    if (strncmp(commande, "OPEN", 4) == 0) 
    {
        char *filename = commande + 5; // Skip "OPEN "
        ctx->file_fd = open(filename, O_RDWR | O_CREAT, 0644);

        if(ctx->file_fd < 0)
        {
            perror("error open-file");
            send(ctx->client_fd, "ERROR : Fichier ne peut pas etre ouvert\n", 39, 0);
            return;
        }
        strncpy(ctx->filename, filename, sizeof(ctx->filename) - 1);
        ctx->filename[sizeof(ctx->filename) - 1] = '\0';

        send(ctx->client_fd, "OK Fichier ouvert\n", 17, 0);
    }
    else if (strncmp(commande, "LOCK", 4) == 0) 
    {   
        if(ctx->file_fd == -1)
        {
            send(ctx->client_fd, "ERROR: Open a file first\n", 25, 0);
            return;
        }
        int retv = verrouiller_fichier(ctx->file_fd, F_WRLCK);
        if(retv == -1)
        {
            send(ctx->client_fd, "ERROR : Fichier ne peut pas Verrouiller\n", 39, 0);
            return;
        } else 
        {
            send(ctx->client_fd, "Fichier Verrouiller", 21, 0);
        }
    }
    else if (strncmp(commande, "UNLOCK", 6) == 0) 
    {
        if(ctx->file_fd == -1)
        {
            send(ctx->client_fd, "ERROR: Open a file first\n", 25, 0);
            return;
        }
        int retd = deverrouiller_fichier(ctx->file_fd);
        if(retd == -1)
        {
            send(ctx->client_fd, "ERROR : Fichier ne peut pas etre Deverrouiller\n", 46, 0);
            return;
        } else {
            send(ctx->client_fd, "Fichier Derrouiller", 20, 0);
        }
        
    }
    else if (strncmp(commande, "WRITE", 5) == 0) 
    {
        if(ctx->file_fd == -1)
        {
            send(ctx->client_fd, "ERROR: Open a file first\n", 25, 0);
            return;
        }
        char *data = commande + 6;
        write(ctx->file_fd, data, strlen(data));
    }
    else 
    {
        send(ctx->client_fd, "COMMANDE INCONNUE\n", 18, 0);
    }
}

// The Client Thread function
void *gerer_client(void *arg) 
{
    client_context_t ctx;
    client_context_t *pctx = (client_context_t *)arg; // Actually we pass a pointer
    char buffer[BUFFER_SIZE];
    int bytes_read;

    // Initialize the context (copy the socket)
    ctx.client_fd = pctx->client_fd;
    ctx.file_fd = -1;
    ctx.filename[0] = '\0';

    while ((bytes_read = recv(ctx.client_fd, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_read] = '\0';
        buffer[strcspn(buffer, "\r\n")] = 0; // Remove carriage return
        
        if (strlen(buffer) > 0) {
            traiter_commande(&ctx, buffer);
        }
    }

    // 🔥 Cleanup: If the client disconnects, we release its locks!
    if (ctx.file_fd != -1) {
        deverrouiller_fichier(ctx.file_fd);
        close(ctx.file_fd);
    }
    close(ctx.client_fd);
    return NULL;
}

// --- MAIN (The server) ---
int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    // Socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) { perror("socket"); exit(1); }

    // Bind
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) { perror("bind"); exit(1); }

    // Listen
    if (listen(server_fd, 5) < 0) { perror("listen"); exit(1); }

    printf("Locked file server started on port %d\n", PORT);
    printf("Available commands: OPEN <file>, LOCK, UNLOCK, WRITE <text>\n");

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (client_fd < 0) { perror("accept"); continue; }

        printf("New client connected.\n");

        // Create a thread for this client
        pthread_t thread_id;
        client_context_t *ctx = malloc(sizeof(client_context_t));
        ctx->client_fd = client_fd;
        ctx->file_fd = -1;
        ctx->filename[0] = '\0';
        
        pthread_create(&thread_id, NULL, gerer_client, (void *)ctx);
        pthread_detach(thread_id); // Let the thread clean itself up
    }

    close(server_fd);
    return 0;
}