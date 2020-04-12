#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>

#include "tools/headers/utils.h"

int main(int argc, char *argv[])
{
    int backlog, port;
    char *strptr;

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

    backlog = strtol(argv[2], &strptr, 10);
    if (backlog == 0)
    {
        printf("ERROR: Enter a valid backlogvalue\n");
        exit(EXIT_FAILURE);
    }

    /* socket related initializations */

    struct sockaddr_in client_addr; // client address

    socklen_t client_addr_size;
    char addr[INET_ADDRSTRLEN];

    fd_set temp_fds; // master file descriptor list
    fd_set read_fds;   // temp file descriptor list for select()
    int max_fd;        // maximum file descriptor number

    int listener_fd; // listening socket descriptor
    int new_fd;      // newly accept()ed socket descriptor

    FD_ZERO(&temp_fds); // clear the master and temp sets
    FD_ZERO(&read_fds);

    char buff[max_mssg_len];

    int bytes_sent, bytes_recvd;
    char greetings[] = "SERVER: Hi client, you are now connected\n";

    char server_IP[INET_ADDRSTRLEN];
    /* ***********************/

    /* SIGINT and SIGKILL */
    struct sigaction sa;
    sa.sa_handler = interrupt_handler;

    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("ERROR: sigaction()");
        exit(EXIT_FAILURE);
    }

    int server_stat = establish_server(argv[1], backlog, server_IP);
    if (server_fd < 0)
    {
        if (server_stat == -1)
        {
            puts("\nERROR: Couldn't build a server\n");
            exit(EXIT_FAILURE);
        }
    }

    else if (server_stat == 0)
    {
        printf("\n[%s] SERVER: UP and LISTENING on %s:%s with BACKLOG %d\n", timestamp(), server_IP, argv[1], backlog);

        // add the listener to the master set
        FD_SET(server_fd, &temp_fds);
        // keep track of the biggest file descriptor
        max_fd = server_fd; // so far, it's this one
        printf("server fd: %d", server_fd);

        while (1)
        {
            read_fds = temp_fds; // copy it
            
            // wait forever for a connection
            if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1)
            {
                perror("ERROR");
                exit(EXIT_FAILURE);
            }
           
            // search for the connection
            for (int i = 0; i <= max_fd; i++)
            {
                if (FD_ISSET(i, &read_fds))
                {
                    if (i == server_fd)
                    {
                        printf("under server fd\n");

                        // there is a new connection
                        new_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_size);
                        if (new_fd == -1)
                        {
                            perror("ERROR accept()");
                            continue;
                        }
                        else
                        {
                            FD_SET(new_fd, &temp_fds);
                            if (new_fd > max_fd)
                                max_fd = new_fd;
                            inet_ntop(client_addr.sin_family, &client_addr.sin_addr.s_addr, addr, sizeof(addr));
                            printf("[%s] SERVER: NEW CLIENT ID(%d): %s:%d\n", timestamp(), new_fd - 3, addr, ntohs(client_addr.sin_port));
                            // send greetings

                            bytes_sent = send(new_fd, &greetings, sizeof(greetings), 0);
                            if (bytes_sent == 0)
                            {
                                printf("[%s] SERVER: Connection lost with client(%d)\n", timestamp(), new_fd - 3);
                                FD_CLR(new_fd, &read_fds);

                                close(new_fd);
                                break;
                            }
                            else if (bytes_sent == -1)
                            {
                                perror("SERVER: send()");
                                break;
                            }
                        } // valid new_fd
                    }     // if server fd ends

                    else
                    {
                        // handle data from a client
                        if ((bytes_recvd = recv(i, buff, sizeof(buff), 0)) <= 0)
                        {
                            // got error or connection closed by client
                            if (bytes_recvd == 0)
                            {
                                // connection closed
                                printf("[%s] SERVER: Connection lost with client(%d)\n", timestamp(), i - 3);
                                close(i);               // bye!
                                FD_CLR(i, &temp_fds); // remove from master set
                            }
                        }
                        else
                        {
                            if (strncmp(buff, "!q", 2) == 0)
                            {
                                close(i);
                                FD_CLR(i, &temp_fds); // remove from master set
                                printf("[%s] SERVER: DISCONNECTED: CLIENT(%d)\n", timestamp(), i - 3);
                                break;
                            }

                            fflush(stdout);
                            printf("[%s] CLIENT_%d@host:~$%s", timestamp(), i - 3, buff);
                        }
                    } // othe than server_fd ends
                }
            }
        }
    }
}
