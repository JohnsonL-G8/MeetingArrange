#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>

#define PORT 8080

int main() {
    int sockfd;
    char buffer[1024];
    struct sockaddr_in servaddr;

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // ServerM address and port
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Names to be sent to ServerM
    std::vector<std::string> names;
    names.push_back("Jas");
    names.push_back("Leo");
    names.push_back("Jay");

    // Send names to ServerM
    for (const auto& name : names) {
        memset(buffer, 0, sizeof(buffer));
        strncpy(buffer, name.c_str(), sizeof(buffer));
        sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr*)&servaddr, sizeof(servaddr));
    }

    std::cout << "Names sent to ServerM" << std::endl;

    close(sockfd);
    return 0;
}
