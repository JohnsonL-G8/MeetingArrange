#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>

#define TCP_PORT_M 24443 // TCP port number
#define BUFFER_SIZE 1024 // buffer size for receiving data

struct TimeInterval {
    int start;
    int end;
};

int main() {
    // create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Error creating socket\n";
        return -1;
    }

    // set server address and port
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // example IP address
    server_addr.sin_port = htons(TCP_PORT_M);

    // connect to server
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error connecting to server\n";
        close(sock);
        return -1;
    }

    // read input of names and store in a list
    std::cout << "Enter names separated by spaces: ";
    std::string names;
    std::getline(std::cin, names);

    // send list of names to server
    if (send(sock, names.c_str(), names.length(), 0) < 0) {
        std::cerr << "Error sending data to server\n";
        close(sock);
        return -1;
    }

    /* Receive data from serverM */
    // Wait for response from server
    char buffer[8192];
    int numBytes = read(sock, buffer, 8192);
    std::vector<TimeInterval> intervals(numBytes / sizeof(TimeInterval));
    memcpy(intervals.data(), buffer, numBytes);
    for (auto const& interval : intervals) {
        std::cout << "[" << interval.start << "," << interval.end << "]\n";
    }

    // close socket
    close(sock);

    std::cout << "Data sent successfully\n";
    return 0;
}
