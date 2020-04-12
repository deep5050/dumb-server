#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <time.h>

#define max_mssg_len 1024

int server_fd;
char *timestamp()
{
    struct tm *local;
    time_t t = time(NULL);
    char *str;

    // Get the localtime
    local = localtime(&t);
    str = asctime(local);

    int i = 0;
    int len = strlen(str) + 1;

    for (i = 0; i < len; i++)
    {
        if (str[i] == '\n')
        {
            // Move all the char following the char "\n" by one to the left.
            strncpy(&str[i], &str[i + 1], len - i);
        }
    }

    return str;
}

void sigint_handler(int sig)
{

    send(server_fd, "!q", 2, 0);
    close(server_fd);
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    char srvaddr[INET_ADDRSTRLEN];

    int myport;
    if (argc != 3)
    {
        puts("USAGE: <serveraddress> <portno>");
        exit(EXIT_FAILURE);
    }
    if (argv[1] == NULL || argv[1] == " ")
    {
        puts("Invalid server address");
        exit(EXIT_FAILURE);
    }
    if (strncmp(argv[1], "0", 1) == 0 || strncmp(argv[1], "localhost", 9) == 0)
    {
        bzero(srvaddr, sizeof(srvaddr));
        strcpy(srvaddr, "0.0.0.0");
    }
    else
    {
        strcpy(srvaddr, argv[1]);
    }

    if (argv[2] == "" || argv[2] == NULL)
    {
        puts("ERROR: Enter a valid port number");
        exit(EXIT_FAILURE);
    }

    char *strptr;
    myport = strtol(argv[2], &strptr, 10);
    struct sockaddr_in server_addr;
    int connect_status, n, sent_bytes, recv_bytes;
    char client_buffer[1], greetings_from_server[100];

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(myport);                       // asighn the same port where the server is running
    int s = inet_pton(AF_INET, srvaddr, &server_addr.sin_addr); // set the server address to the localhost
    if (s <= 0)
    {
        if (s == 0)
            fprintf(stderr, "server Ip is Not in presentation format\n");
        else
            perror("inet_pton");
        exit(EXIT_FAILURE);
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int synRetries = 2; // Send a total of 3 SYN packets => Timeout ~7s
    setsockopt(server_fd, IPPROTO_TCP, TCP_SYNCNT, &synRetries, sizeof(synRetries));

    if (server_fd < 0)
    {
        perror("ERROR socket()");
        exit(EXIT_FAILURE);
    }

    printf("[%s] Connection request sent, waiting for server to response\n", timestamp());
    connect_status = connect(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    if (connect_status < 0)
    {
        perror("ERROR connect()");
        exit(EXIT_FAILURE);
    }

    recv_bytes = recv(server_fd, &greetings_from_server, sizeof(greetings_from_server), 0);
    if (recv_bytes < 0)
    {
        perror("ERROR recv()");
    }
    else
    {
        printf("[%s] %s", timestamp(), greetings_from_server);
    }
    printf("[%s] Remote Terminal is Ready\n", timestamp());
    printf("[%s] Type your message (Type '!q' or hit ^C to quit):\n", timestamp());

    while (1)
    {

        int n = 0;
        char buff[max_mssg_len];
        bzero(buff, max_mssg_len);
        signal(SIGINT, sigint_handler);
        printf("you@server:~$");
        while ((buff[n++] = getchar()) != '\n')
            ;

        sent_bytes = send(server_fd, &buff, sizeof(buff), 0);
        if (sent_bytes == -1)
        {
            perror("ERROR");
        }
        if (strncmp(buff, "!q", 2) == 0)
        {
            close(server_fd);
            printf("\n[%s] TERMINATED\n", timestamp());
            exit(EXIT_SUCCESS);
        }
    }

    return 0;
}
