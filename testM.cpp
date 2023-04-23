#include <iostream>
#include <unordered_set>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 8080
#define MAXLINE 1024

int main() {
    int sockfd;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr, cliaddr;

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // ServerM address and port
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // Bind socket to ServerM address and port
    if (bind(sockfd, (const struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    std::unordered_set<std::string> names;

    // Receive names from ServerA and store in names unordered_set
    while (true) {
        socklen_t len = sizeof(cliaddr);
        memset(buffer, 0, sizeof(buffer));
        int n = recvfrom(sockfd, buffer, MAXLINE, MSG_WAITALL, (struct sockaddr*)&cliaddr, &len);
        if (n <= 0) {
            perror("recvfrom failed");
            exit(EXIT_FAILURE);
        }

        std::string name(buffer, n);
        names.insert(name);
        std::cout << "Received name: " << name << std::endl;
    }

    close(sockfd);
    return 0;
}
