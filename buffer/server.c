#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <errno.h>


int create_sock();

int main(int argc, char *argv[]) {

    int sock_fd = create_sock();
    char buf[1024];

    struct sockaddr_in client_addr;

    while(1) {
        socklen_t cli_len = sizeof(client_addr);
        memset(&client_addr, 0, sizeof(client_addr));

        int fd = accept(sock_fd, (struct sockaddr *)&client_addr, &cli_len);
        if (fd < 0) {
            perror("accept error");
            exit(1);
        }

        bzero(buf, sizeof(buf));

        int ret = read(fd, buf, sizeof(buf));
       // int ret = recv(fd, buf, sizeof(buf), 0);
        if (ret < 0) {
            perror("recv error");
            exit(1);
        }
        if (ret == 0) {
            printf("client closed \n");
            close(fd);
            continue;
        }

        printf("recv: %s\n", buf);
    }

}

int create_sock() {
    int sock_fd;
    struct sockaddr_in serv_addr;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0 , sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(3000);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int on = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    bind(sock_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

    listen(sock_fd, 1024);

    return sock_fd;
}