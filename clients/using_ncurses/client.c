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
#include <ncurses.h>

#define max_mssg_len 1024

int server_fd;
void ncurse_in()
{

    initscr();
    cbreak();
    keypad(stdscr, TRUE);
}

void ncurse_exit()
{
    endwin(); /*  restore the default terminal settings */
}

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
    ncurse_exit();
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    int myport;
    if (argc != 2)
    {
        puts("USAGE: <portno>");
        exit(EXIT_FAILURE);
    }

    if (argv[1] == "" || argv[1] == NULL)
    {
        puts("ERROR: Enter a valid port number");
        exit(EXIT_FAILURE);
    }

    myport = atoi(argv[1]);
    struct sockaddr_in server_addr;
    int connect_status, n, sent_bytes, recv_bytes;
    char client_buffer[1], greetings_from_server[100];

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(myport);                   // asighn the same port where the server is running
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr); // set the server address to the localhost

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
    printf("[%s] Type your message (hit ^C to quit):\n", timestamp());
    ncurse_in();
    printf("you@server:~$");
    while (1)
    {

        int n = 0;
        char buff[1];
        bzero(buff, 1);
        signal(SIGINT, sigint_handler);

        /*  get char by char */
        char ch;
        nodelay(stdscr, TRUE);
        for (;;)
        {
            if ((ch = getch()) == ERR)
            {
                /* user hasn't responded
               ...
              */
            }
            else
            {
                /* user has pressed a key ch
               ...
              */
                if (ch == 10)
                {
                    // ncurse_exit();
                    fflush(stdout);
                    printf("\nyou@server:~$");
                    ncurse_in();
                }
                sent_bytes = send(server_fd, &ch, sizeof(ch), 0);
                if (sent_bytes == -1)
                {
                    perror("ERROR");
                }
            }
        }
    }

    return 0;
}
