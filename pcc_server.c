#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

void sigusr1_handler();

//global vars
int cfd = -1;
int flag_for_sig = 0;
uint32_t total[95] = {0};

// we start by defining some functions that will help us with the error handling
void check_args(int argsnum)
{
    if (argsnum != 2) {
        fprintf(stderr, "Wrong number of arguments because you didn't pass 2 args, Error: %s\n", strerror(EINVAL));
        exit(1);
    }
}

void check_sigact(int num)
{
    if (num != 0) {
        fprintf(stderr, "Problem with Signal handling. Error: %s\n", strerror(errno));
        exit(1);
    }
}

void check_lfd(int lfd)
{
    if (lfd < 0){
        fprintf(stderr, "Problem while creating socket, Error: %s\n", strerror(errno));
        exit(1);
    }
}

void check_sock_opt(int num)
{
    if(num < 0){
        fprintf(stderr, "Problem with setsockopt Error: %s\n", strerror(errno));
        exit(1);
    }
}

void check_bind(int num)
{
    if (num != 0){
        fprintf(stderr, "Problem with bind, Error: %s \n", strerror(errno));
        exit(1);
    }
}

void check_listen(int num)
{
    if (num != 0){
        fprintf(stderr, "Problem with listen. %s \n", strerror(errno));
        exit(1);
    }
}

void check_accept(int num)
{
    if(num < 0){
        fprintf(stderr, "Accept Failed, Error: %s\n", strerror(errno));
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    // check that the number of arguments is right
    check_args(argc);
    int temp; // a temporary integer that stores some of the results that we will need
    char *buffer1;
    char *buffer2;
    uint32_t uint1, uint2, uint3;
    uint32_t buf[95] = {0};
    int x = 1;
    int lfd = -1;
    struct sockaddr_in s_a;
    // initiate SIGUSR1 handler
    struct sigaction sigusr1;
    sigusr1.sa_handler = &sigusr1_handler;
    sigemptyset(&sigusr1.sa_mask);
    sigusr1.sa_flags = SA_RESTART;
    temp = sigaction(SIGUSR1, &sigusr1, 0);
    check_sigact(temp);
    lfd = socket(AF_INET, SOCK_STREAM, 0);
    check_lfd(lfd);
    temp = setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &x, sizeof(int));
    check_sock_opt(temp);
    memset(&s_a, 0, sizeof(s_a));
    s_a.sin_family = AF_INET;
    s_a.sin_addr.s_addr = htonl(INADDR_ANY);
    s_a.sin_port = htons(atoi(argv[1]));
    temp = bind(lfd, (struct sockaddr *)&s_a, sizeof(s_a));
    check_bind(temp);
    temp = listen(lfd, 10);
    check_listen(temp);
    // infinite loop for accepting TCP connections
    while(1){
        //checking for a SIGUSR1
        if(flag_for_sig){
            for(int i = 0; i < 95; i++){
                printf("char '%c' : %u times\n", (i+32), total[i]);
            }
            exit(0);
        }
        cfd = accept(lfd, NULL, NULL);
        check_accept(cfd);
        buffer1 = (char*)&uint2;
        int bts_transfered = 0;
        int bts = 1;
        while (bts > 0){
            bts = read(cfd, buffer1 + bts_transfered, 4 - bts_transfered);
            bts_transfered += bts;
        }
        if(bts == 0){
            if(bts_transfered != 4){
                fprintf(stderr, "Problem with reading bts the server will keep accepting new connections. Error: %s\n", strerror(errno));
                close(cfd);
                cfd = -1;
                continue;
            }
        }else{
            if (errno == ETIMEDOUT || errno == EPIPE || errno == ECONNRESET){
                fprintf(stderr, "TCP problem while reading: %s\n", strerror(errno));
                close(cfd);
                cfd = -1;
                continue;
            }else{
                fprintf(stderr, "Problem with uint1 reading, Error: %s\n", strerror(errno));
                exit(1);
            }
        }
        // convert uint1 to host memory allocation
        uint1 = ntohl(uint2);
        buffer2 = malloc(uint1);
        bts_transfered = 0;
        bts = 1;
        while(bts > 0){
            bts = read(cfd, buffer2 + bts_transfered, uint1 - bts_transfered);
            bts_transfered += bts;
        }
        if(bts == 0){
            if(bts_transfered != uint1){
                fprintf(stderr, "Problem with reading bts the server will keep accepting new connections. Error: %s\n", strerror(errno));
                free(buffer2);
                close(cfd);
                cfd = -1;
                continue;
            }
        }else{
            if (errno == ETIMEDOUT || errno == ECONNRESET || errno == EPIPE){
                fprintf(stderr, "TCP problem while reading: %s\n", strerror(errno));
                free(buffer2);
                close(cfd);
                cfd = -1;
                continue;
            }else{
                fprintf(stderr, "Problem with reading, Error: %s\n", strerror(errno));
                exit(1);
            }
        }
        for(int i = 0; i < 95; i++){
            buf[i] = 0;
        }
        uint3 = 0;
        for(int i = 0; i < uint1; i++){
            if(buffer2[i] >= 32){
                if(buffer2[i] <= 126){
                    uint3++;
                    buf[(int)(buffer2[i] - 32)]++;
                }
            }
        }
        free(buffer2);
        uint2 = htonl(uint3);
        bts_transfered = 0;
        bts = 1;
        while(bts > 0){
            bts = write(cfd, buffer1 + bts_transfered, 4 - bts_transfered);
            bts_transfered += bts;
        }
        if(bts == 0){
            if(bts_transfered != 4){
                fprintf(stderr, "Problem with writing bts %s\n", strerror(errno));
                close(cfd);
                cfd = -1;
                continue;
            }
        }else{
            if (errno == ETIMEDOUT || errno == ECONNRESET || errno == EPIPE){
                fprintf(stderr, "TCP problem while writing: %s\n", strerror(errno));
                close(cfd);
                cfd = -1;
                continue;
            }else{
                fprintf(stderr, "Problem with uint3 write, Error: %s\n", strerror(errno));
                exit(1);
            }
        }
        for(int i = 0; i < 95; i++){
            total[i] += buf[i];
        }
        close(cfd);
        cfd = -1;
    }
}
void sigusr1_handler(){
    if(cfd >= 0){
        flag_for_sig = 1; // we will catch that in the server's while loop
    }else{
        for(int i = 0; i < 95; i++){
            printf("char '%c' : %u times\n", (i+32), total[i]);
        }
        exit(0);
    }
}