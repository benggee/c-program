#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <strings.h>
#include <errno.h>


int main(int argc, char *argv[]) {
    int sock_fd;
    struct sockaddr_in serv_addr;
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(3000);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    int ret = connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (ret < 0) {
        perror("connect error");
        exit(1);
    }

    char buf[1024];

    while(1) {
        memset(buf, 0, sizeof(buf));
        fgets(buf, sizeof(buf), stdin);
        write(sock_fd, buf, sizeof(buf));
    }
}