#define _DEFAULT_SOURCE
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

void check_args(int argsnum)
{
    if (argsnum != 4) {
        fprintf(stderr, "Wrong number of arguments because you didn't pass 4 args, Error: %s\n", strerror(EINVAL));
        exit(1);
    }
}

void check_sock(int num)
{
    if (num < 0){
        fprintf(stderr, "Problem while creating the socket, Error: %s\n", strerror(errno));
        exit(1);
    }
}

void check_bts(int bts)
{
    if(bts < 0){
        fprintf(stderr, "Problem with writing to the server, Error: %s\n", strerror(errno));
        exit(1);
    }
}

void check_connect(int num)
{
    if (num < 0){
        fprintf(stderr, "Problem with connecting, Error: %s\n", strerror(errno));
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    // first we make sure that we have the right number of arguments
    check_args(argc);
    int temp;
    int con = 4;
    FILE *fl;
    fl = fopen(argv[3],"rb");
    if (fl == NULL) {
        fprintf(stderr, "Problem with opening the file: %s\n Error: %s\n", argv[3], strerror(errno));
        exit(1);
    }
    uint32_t uint2;
    char *buffer1;
    char *buffer2;
    struct sockaddr_in s_a; //the destination
    fseek(fl, 0, SEEK_END);
    uint2 = ftell(fl);
    fseek(fl, 0, SEEK_SET);
    buffer2 = malloc(uint2);
    if (buffer2 == NULL){
        fprintf(stderr, "Got an error while allocating N=%u bytes to the buffer: %s\n", uint2, strerror(errno));
        exit(1);
    }
    temp = fread(buffer2, 1, uint2, fl);
    if (temp != uint2){
        fprintf(stderr, "Got an error while reading from the file: %s\n", strerror(errno));
        exit(1);
    }
    fclose(fl);
    int sock = socket(AF_INET, SOCK_STREAM, 0);// creation of the socket
    check_sock(sock);
    // information for the server, setting the IP and port from the arguments and then we try to connect the socket
    // and the target address
    memset(&s_a, 0, sizeof(s_a));
    s_a.sin_family = AF_INET;
    s_a.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[1], &s_a.sin_addr);
    temp = connect(sock, (struct sockaddr *)&s_a, sizeof(s_a));
    check_connect(temp);
    uint32_t uint1;
    uint1 = (htonl(uint2));
    buffer1 = (char*)&uint1;
    // next we will make 3 while loops in order to transfer data between the client and the server
    // 2 of them will transfer 4 bytes and 1 will transfer uint2 bytes
    int bts_transfered = 0;
    int bts;
    //this loop is responsible for writing 4 bytes
    while(bts_transfered < con){
        bts = write(sock, buffer1 + bts_transfered, 4 - bts_transfered);
        check_bts(bts);
        bts_transfered += bts;
    }
    bts_transfered = 0;
    //this loop is responsible for writing uint2 bytes
    while(bts_transfered < uint2){
        bts = write(sock, buffer2 + bts_transfered, uint2 - bts_transfered);
        check_bts(bts);
        bts_transfered += bts;
    }
    free(buffer2);
    bts_transfered = 0;
    //this loop is responsible for reading 4 bytes
    while(bts_transfered < con){
        bts = read(sock, buffer1 + bts_transfered, 4 - bts_transfered);
        check_bts(bts);
        bts_transfered += bts;
    }
    close(sock);
    printf("# of printable characters: %u\n", ntohl(uint1));
    exit(0);
}