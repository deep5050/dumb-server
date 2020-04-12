#ifndef UTILS_H
#define max_mssg_len 1024

int server_fd;

char *timestamp(void);
void interrupt_handler(int);
int establish_server( char *, int, char *);

#endif