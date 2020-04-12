#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>


#define max_mssg_len 1024

int server_fd;

char *timestamp()
{
    struct tm *local;
    time_t t = time(NULL);
    char *str;

    /* Get the localtime */
    local = localtime(&t);
    str = asctime(local);

    int i = 0;
    int len = strlen(str) + 1;

    for (i = 0; i < len; i++)
    {
        if (str[i] == '\n')
        {
            /* Move all the char following the char "\n" by one to the left. */
            strncpy(&str[i], &str[i + 1], len - i);
        }
    }

    return str;
}

void sigchld_handler(int s)
{
    /* waitpid() might overwrite errno, so we save and restore it: */
    int saved_errno = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
    errno = saved_errno;
}

void INTR_handler(int sig)
{
    close(server_fd);
    printf("\n[%s] SERVER CLOSED\n", timestamp());
    exit(0);
}

int main(int argc, char *argv[])
{
    int backlog;

    if (argc != 3)
    {
        puts("USAGE: <portno> <backlog>");
        exit(EXIT_FAILURE);
    }

    if (argv[1] == "" || argv[1] == NULL)
    {
        puts("ERROR: Enter a valid port number");
        exit(EXIT_FAILURE);
    }

    if (argv[2] == "" || argv[2] == NULL)
    {
        puts("ERROR: Enter a valid backlog value");
        exit(EXIT_FAILURE);
    }
    char *strptr;
    backlog = strtol(argv[2], &strptr, 10);

    struct addrinfo server_addr, *results, *p;
    struct sockaddr_in client_addr;
    socklen_t client_addr_size;

    int client_fd, server_status, bind_status, listen_status, accept_status, addr_status;
    char buff[max_mssg_len];
    int n = 0, yes = 1;

    int bytes_sent;
    char addr[INET_ADDRSTRLEN];

    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    /*
            struct addrinfo {
                int              ai_flags;
                int              ai_family;
                int              ai_socktype;
                int              ai_protocol;
                socklen_t        ai_addrlen;
                struct sockaddr *ai_addr;
                char            *ai_canonname;
                struct addrinfo *ai_next;
            };
    */

    server_addr.ai_family = PF_INET;       /* IPv4 */
    server_addr.ai_socktype = SOCK_STREAM; /* tcp Stream */
    server_addr.ai_flags = AI_PASSIVE;     /* fill ip automatically */

    /* get all the available address of this machine */

    if ((addr_status = getaddrinfo(NULL, argv[1], &server_addr, &results)) != 0)
    {
        printf("[%s] SERVER: ERROR getaddrinfo(): %s\n", timestamp(), gai_strerror(addr_status));
        exit(EXIT_FAILURE);
    }

    /* bind a socket at the first available address */
    for (p = results; p != NULL; p = p->ai_next)
    {

        inet_ntop(p->ai_family, p->ai_addr, addr, sizeof(addr));
        printf("[%s] SERVER: Trying to build on: %s", timestamp(), addr);

        if ((server_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("ERROR socket()");
            continue; /* if socket can not be created on this address try another */
        }

        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
        {
            perror("ERROR setsockopt()");
            exit(EXIT_FAILURE);
        }

        if (((bind_status = bind(server_fd, p->ai_addr, p->ai_addrlen)) == -1))
        {
            perror("ERROR bind()");
            continue;
        }

        /**
         * control reaches here if all the above conditions satifie
         * we have succesfully bound a socket and can exit from this loop
         */

        break;
    }

    freeaddrinfo(results);

    /**
     * if we get p== NULL that means we couldn't bind to any of the addresses
     * should return from the program
     */

    if (p == NULL)
    {
        puts("\nERROR: Couldn't build a server");
        exit(EXIT_FAILURE);
    }

    if (listen_status = listen(server_fd, backlog) == -1)
    {
        perror("ERROR listen()");
        exit(EXIT_FAILURE);
    }
    /* handle sigaction */

    printf("\n[%s] SERVER: UP and LISTENING on %s:%s with BACKLOG %d\n", timestamp(), addr, argv[1], backlog);

    struct sigaction sa;
    sa.sa_handler = sigchld_handler; /* reap all dead processes */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    char greetings[] = "SERVER: Hi client, you are now connected\n";

    while (1)
    {
        signal(SIGINT, INTR_handler);

        inet_ntop(client_addr.sin_family, &client_addr.sin_addr.s_addr, addr, sizeof(addr));

        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_size);

        printf("[%s] SERVER: NEW CLIENT ID(%d): %s:%d\n", timestamp(), client_fd - 3, addr, ntohs(client_addr.sin_port));

        if (fork() == 0)
        {
            bytes_sent = send(client_fd, &greetings, sizeof(greetings), 0);
            if (bytes_sent == 0)
            {
                printf("[%s] SERVER: Connection lost with client(%d)\n", timestamp(), client_fd - 3);
                exit(EXIT_FAILURE);
            }
            else if (bytes_sent == -1)
            {
                perror("SERVER: send()");
                exit(EXIT_FAILURE);
            }

            close(server_fd);
            bzero(buff, max_mssg_len);

            while (1)
            {
                int bytes_rcvd;

                bytes_rcvd = recv(client_fd, &buff, sizeof(buff), 0);
                if (bytes_rcvd == 0)
                {
                    printf("[%s] SERVER: Connection lost with client(%d)\n", timestamp(), client_fd - 3);
                    exit(EXIT_FAILURE);
                }
                else if (bytes_rcvd == -1)
                {
                    perror("SERVER: recv()");
                    exit(EXIT_SUCCESS); // dont think its a failure
                }
                if (strncmp(buff, "!q", 2) == 0)
                {
                    close(client_fd);
                    printf("[%s] SERVER: DISCONNECTED: CLIENT(%d)\n", timestamp(), client_fd - 3);
                    exit(EXIT_SUCCESS);
                }

                fflush(stdout);
                printf("[%s] CLIENT_%d@host:~$%s", timestamp(), client_fd - 3, buff);
            }
        }
    }
    return 0;
}