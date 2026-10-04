#include "lib.h"

#define PORT 8080
#define BUFFER_SIZE 4096

void build_http_response(char *response, size_t *resp_len)
{
    const char *body = 
        "<!DOCTYPE html>"
        "<html><head><title>LUDOX</title></head>"
        "<body><h1>Minimal server</h1>"
        "<p>This is the first web page served by my C code!</p>"
        "</body></html>";
    
    // HTTP header
    *resp_len = snprintf(response, BUFFER_SIZE,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s", 
        strlen(body),
        body);
}

int main(void)
{
    int server_dt, client_dt;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    char response[BUFFER_SIZE] = {0};
    size_t resp_len;

    // Create the connection socket
    server_dt = socket(AF_INET, SOCK_STREAM, 0);

    if(server_dt <= 0)
    {
        perror("server error");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = INADDR_ANY;

    if(bind(server_dt, (struct sockaddr *)&address, sizeof(address)) < 0) // Give an identity to the socket
    {
        perror("bind error");
        exit(EXIT_FAILURE);
    }

    if(listen(server_dt, 5) < 0) // Listening connection socket
    {
        perror("listen error");
        exit(EXIT_FAILURE);
    }

    printf("HTTP server started on http://localhost:%d\n", PORT);
    printf("Waiting for connections...\n");

    while (1)
    {
        client_dt = accept(server_dt, (struct sockaddr *)&address, &addrlen); // Accept the connection

        if(client_dt < 0)
        {
            perror("client error");
            exit(EXIT_FAILURE);
        }

        recv(client_dt, buffer, BUFFER_SIZE - 1, 0); // Receive the messages from the client socket into the buffer

        build_http_response(response, &resp_len);

        send(client_dt, response, resp_len, 0); // Send the packet to the client socket

        // Close the client connection
        close(client_dt);
        printf("Response sent to %s\n", inet_ntoa(address.sin_addr));
    }
    
    close(server_dt); // Close the connection socket

    return 0;
}