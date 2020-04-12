#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <error.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>

#include "headers/utils.h"

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

/* sighandler */
void interrupt_handler(int s)
{
    printf("[%s] SERVER: QUITING \n", timestamp());
    close(server_fd);
    exit(EXIT_SUCCESS);
}

/* Get listener fd */

int establish_server( char *port, int backlog, char *server_IP)
{
    int server_status, bind_status, listen_status, accept_status, addr_status;
    int n = 0, yes = 1;
    struct addrinfo server_addr, *results, *p;
    char addr[INET_ADDRSTRLEN];

    memset(&server_addr, 0, sizeof(server_addr));

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

    if ((addr_status = getaddrinfo(NULL, port, &server_addr, &results)) != 0)
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
        return -1;
    }

    if (listen_status = listen(server_fd, backlog) == -1)
    {
        // perror("ERROR listen()");
        return -2;
    }
    strncpy(server_IP, addr, INET_ADDRSTRLEN);

    return 0;
}
