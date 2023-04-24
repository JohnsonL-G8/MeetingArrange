#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <unordered_set>
#include <vector>
#include <sstream>

#define TCP_PORT 24443 // TCP port number
#define BUFFER_SIZE 1024 // buffer size for receiving data
#define UDP_PORT 23443 //UDP port number
#define UDP_PORT_A 21443
#define UDP_PORT_B 22443

struct TimeInterval {
    int start;
    int end;
};

std::vector<TimeInterval> mergeInterval(std::vector<TimeInterval> intervalsA, std::vector<TimeInterval> intervalsB){
    // std::cout << "Start merging...";
    std::vector<TimeInterval> ret;
    if(intervalsA.size() == 0 && intervalsB.size() == 0){
        return ret;
    }
    if(intervalsA.size() == 0){
        return intervalsB;
    }
    if(intervalsB.size() == 0){
        return intervalsA;
    }
    int m = intervalsA.size(), n = intervalsB.size();
    // std::sort(intervals.begin(), intervals.end(), compareInterval);
    int i = 0, j = 0;
    while(i < m && j < n){
        if(intervalsA[i].end <= intervalsB[j].start){
            i++;
        }else if(intervalsB[j].end <= intervalsA[i].start){
            j++;
        }else{
            int new_start = std::max(intervalsA[i].start, intervalsB[j].start);
            int new_end = std::min(intervalsA[i].end, intervalsB[j].end);
            TimeInterval cur = {new_start, new_end};
            ret.push_back(cur);
            if(intervalsA[i].end < intervalsB[j].end){
                i++;
            }else{
                j++;
            }
        }
    }
    return ret;
}

std::string parseInput(std::string input_str){
    // iterate through each character in the input string
    for (int i = 0; i < input_str.length(); i++) {
        // if the current character is a space
        if (input_str[i] == ' ') {
            // skip all consecutive spaces until a non-space character is found
            while (input_str[i + 1] == ' ') {
                input_str.erase(i + 1, 1);  // erase the extra space
            }
        }
    }
    return input_str;
}

int main() {
    /* UDP connection */
    struct sockaddr_in udp_serv_addr, udp_cli_addr;

    // Creating UDP socket file descriptor
    int udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sockfd < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }

    memset(&udp_serv_addr, 0, sizeof(udp_serv_addr));
    memset(&udp_cli_addr, 0, sizeof(udp_cli_addr));

    // Filling server information
    udp_serv_addr.sin_family = AF_INET; // IPv4
    udp_serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    udp_serv_addr.sin_port = htons(UDP_PORT);

    // Bind the socket with the server address
    if (bind(udp_sockfd, (const struct sockaddr *)&udp_serv_addr, sizeof(udp_serv_addr)) < 0) {
        std::cerr << "UDP Bind failed" << std::endl;
        return 1;
    }

    int len, n;
    char buffer[1024];
    std::unordered_set<std::string> namesA;
    std::unordered_set<std::string> namesB;


    /***
     *  TCP connection 
     ***/

    // create socket
    int tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_sockfd < 0) {
        std::cerr << "Error creating socket\n";
        return -1;
    }

    // set server address and port
    struct sockaddr_in tcp_serv_addr, tcp_cli_addr;
    memset(&tcp_serv_addr, 0, sizeof(tcp_serv_addr));
    tcp_serv_addr.sin_family = AF_INET;
    tcp_serv_addr.sin_addr.s_addr = INADDR_ANY;
    tcp_serv_addr.sin_port = htons(TCP_PORT);

    // bind socket to address and port
    if (bind(tcp_sockfd, (struct sockaddr*)&tcp_serv_addr, sizeof(tcp_serv_addr)) < 0) {
        std::cerr << "Error binding TCP socket\n";
        close(tcp_sockfd);
        return -1;
    }

    // listen for incoming connections
    if (listen(tcp_sockfd, 5) < 0) {
        std::cerr << "Error listening for connections\n";
        close(tcp_sockfd);
        return -1;
    }

    std::cout << "Main Server is up and running" << std::endl;

    /***
     * 
    */
    /* Receiving the length of the name lists */
    int nameLenA = 0;
    int nameLenB = 0;
    /* Get name list from A*/
    while(true){
        //std::cout << "Receiving Lists length from serverA" << std::endl;
        int bytesReceived = recvfrom(udp_sockfd, buffer, sizeof(buffer), 0, (sockaddr*)&udp_cli_addr, 
        reinterpret_cast<socklen_t *>(&len));
        if (bytesReceived < 0) {
            std::cerr << "Failed to receive data." << std::endl;
            return 1;
        }
        memcpy(&nameLenA, buffer, sizeof(nameLenA));
        // std::cout << "Receiving list size: " << nameLenA << std::endl;
        if(nameLenA != 0){
            break;
        }
    }

    /* Receiving name lists from serverA*/
    while(true){
        // std::cout << "listening for the name lists" << std::endl;
        /* Forming UDP connection */
        len = sizeof(udp_cli_addr); // len is value/resuslt
        n = recvfrom(udp_sockfd, (char *)buffer, 1024,
                     MSG_WAITALL, (struct sockaddr *)&udp_cli_addr,
                     reinterpret_cast<socklen_t *>(&len));
        buffer[n] = '\0';
        std::string name(buffer);
        if (namesA.find(name) == namesA.end()) {
            namesA.insert(name);
            // std::cout << "Added " << name << " to the set." << std::endl;
        }

        if(namesA.size() == nameLenA){
            break;
        }
        // std::copy(namesA.begin(), namesA.end(), 
        // std::ostream_iterator<std::string>(std::cout, " "));
    }
    std::cout << "Main Server received the username list from server A using UDP over port 23443." << std::endl;

    // for (auto it = namesA.begin(); it != namesA.end(); ++it) {
    //    std::cout << *it << " ";
    // }
    // std::cout << "ausxbnoinasdpo" << std::endl;

    /* Get name list from B*/
    while(true){
        // std::cout << "Receiving Lists length from serverB" << std::endl;
        int bytesReceived = recvfrom(udp_sockfd, buffer, sizeof(buffer), 0, (sockaddr*)&udp_cli_addr, 
        reinterpret_cast<socklen_t *>(&len));
        if (bytesReceived < 0) {
            std::cerr << "Failed to receive data." << std::endl;
            return 1;
        }
        memcpy(&nameLenB, buffer, sizeof(nameLenB));
        if(nameLenB != 0){
            break;
        }
    }

    /* Receiving name lists from serverB */
    while(true){
        // std::cout << "listening for the name lists" << std::endl;
        /* Forming UDP connection */
        len = sizeof(udp_cli_addr); // len is value/resuslt
        n = recvfrom(udp_sockfd, (char *)buffer, 1024,
                     MSG_WAITALL, (struct sockaddr *)&udp_cli_addr,
                     reinterpret_cast<socklen_t *>(&len));
        buffer[n] = '\0';
        std::string name(buffer);
        if (namesB.find(name) == namesB.end()) {
            namesB.insert(name);
            // std::cout << "Added " << name << " to the set." << std::endl;
        }

        if(namesB.size() == nameLenB){
            break;
        }
    }

    std::cout << "Main Server received the username list from server B using UDP over port 23443." << std::endl;


    /***
     * (1) Receiving names from client(TCP) ✅
     * (2) Check if the names exist ✅
     * (3) Sending names to serverA and serverB(UDP) ✅
     * (4) Receiving the time intervals from serverA & B(UDP) 
     * (5) Merge the time intervals
     * (6) Send the available intervals to client(TCP)
    */
    char tcp_buffer[BUFFER_SIZE];
    std::vector<std::string> client_names;
    while (true) {
        /* Forming TCP connection */
        // std::cout << "Start listening to the client..." << std::endl;
        // std::cout << "Server listening for connections...\n";
        socklen_t client_len = sizeof(tcp_cli_addr);
        int client_sock = accept(tcp_sockfd, (struct sockaddr*)&tcp_cli_addr, &client_len);
        if (client_sock < 0) {
            std::cerr << "Error accepting connection\n";
            continue;
        }

        // std::cout << "Accepting names....\n";
        // (1) receive list of names from client
        memset(&tcp_buffer, 0, sizeof(tcp_buffer));
        int bytes_received = recv(client_sock, tcp_buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received < 0) {
            std::cout << "No Valid Input\n";
            continue;
        }

        std::string client_input = tcp_buffer;
        client_input = parseInput(client_input);
        std::vector<std::string> query_names, queryA, queryB, notExist;
        std::stringstream ss(client_input);
        std::string name;
        
        while (getline(ss, name, ' ')) {
            query_names.push_back(name);
        }

        std::cout << "Main Server received the request from client using TCP over port 24443." << std::endl;
        
        // (2) Check if input name exists
        for (std::string t : query_names) {
            // std::cout << t << std::endl;
            if(namesA.count(t)){
                queryA.push_back(t);
                continue;
            }
            if(namesB.count(t)){
                queryB.push_back(t);
                continue;
            }
            notExist.push_back(t);
        }

        /* Send name that does not exist to the client */
        std::string name_return = "";
        for(std::string t : notExist){
            name_return = name_return + t + ", ";
        }
        if(name_return != ""){
            name_return.pop_back();
            name_return.pop_back();
            std::cout << name_return + " do not exist. Send a reply to the client." << std::endl;

            // Send invalid names to client
            char* invalid = new char[name_return.size()];
            memcpy(invalid, name_return.data(), name_return.size());
            ssize_t bytes_sent = send(client_sock, invalid, name_return.size(), 0);
            if (bytes_sent == -1) {
                std::cerr << "Failed to send response to client\n";
                return 1;
            }
        }else{
            char* invalid = new char[3];
            std::string mark = "127";
            memcpy(invalid, mark.data(), mark.size());
            ssize_t bytes_sent = send(client_sock, invalid, mark.size(), 0);
            if (bytes_sent == -1) {
                std::cerr << "Failed to send response to client\n";
                return 1;
            }
        }

        if(queryA.size() == 0 && queryB.size() == 0){
            continue;
        }

        // (3) Sending names to serverA and serverB(UDP)
        /* Send data to ServerA through UDP */
        int udp_sockfd_A, udp_sockfd_B;
        char udp_buffer[1024];
        struct sockaddr_in udp_serv_addrA, udp_serv_addrB;

        // Creating socket file descriptor
        udp_sockfd_A = socket(AF_INET, SOCK_DGRAM, 0);
        if (udp_sockfd_A < 0) {
            std::cerr << "Error creating socket" << std::endl;
            return 1;
        }

        memset(&udp_serv_addrA, 0, sizeof(udp_serv_addrA));

        // Filling server information
        udp_serv_addr.sin_family = AF_INET; // IPv4
        udp_serv_addr.sin_port = htons(UDP_PORT_A);
        udp_serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

        /*Send number of names*/
        int size = queryA.size();
        if(size > 0){
            sendto(udp_sockfd_A, &size, sizeof(size), 0, (const struct sockaddr*)&udp_serv_addr, sizeof(udp_serv_addr));
            std::string namesToA = "";
            for(std::string name : queryA){
                namesToA = namesToA + name + ", ";
            }
            namesToA.pop_back();
            namesToA.pop_back();
            std::cout << "Found " + namesToA + " located at Server A. Send to Server A" << std::endl;
        }
        // std::cout << size << std::endl;
    //    sssendto(sock, &num, sizeof(num), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));

        for (const auto& name : queryA) {
            // std::cout << "Sending name to A" << std::endl;
            memset(buffer, 0, sizeof(buffer));
            strncpy(buffer, name.c_str(), sizeof(buffer));
            sendto(udp_sockfd_A, buffer, strlen(buffer), 0, (const struct sockaddr*)&udp_serv_addr, sizeof(udp_serv_addr));
        }

        // Creating socket file descriptor
        udp_sockfd_B = socket(AF_INET, SOCK_DGRAM, 0);
        if (udp_sockfd_B < 0) {
            std::cerr << "Error creating socket" << std::endl;
            return 1;
        }

        memset(&udp_serv_addrB, 0, sizeof(udp_serv_addrB));

        // Filling server information
        udp_serv_addr.sin_family = AF_INET; // IPv4
        udp_serv_addr.sin_port = htons(UDP_PORT_B);
        udp_serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

        /*Send number of names*/
        size = queryB.size();
        if(size > 0){
            sendto(udp_sockfd_B, &size, sizeof(size), 0, (const struct sockaddr*)&udp_serv_addr, sizeof(udp_serv_addr));
            std::string namesToB = "";
            for(std::string name : queryB){
                namesToB = namesToB + name + ", ";
            }
            namesToB.pop_back();
            namesToB.pop_back();
            std::cout << "Found " + namesToB + " located at Server B. Send to Server B" << std::endl;
        }

        for (const auto& name : queryB) {
            memset(buffer, 0, sizeof(buffer));
            strncpy(buffer, name.c_str(), sizeof(buffer));
            sendto(udp_sockfd_B, buffer, strlen(buffer), 0, (const struct sockaddr*)&udp_serv_addr, sizeof(udp_serv_addr));
        }

        /* (4) Receiving the time intervals from serverA(UDP) */
        char* intervals_buffer = new char[8192];
        std::vector<TimeInterval> intervalsA;
        while(true){
            if(queryA.size() == 0){
                break;
            }
            // std::cout << "listening for result from A" << std::endl;
            /* Forming UDP connection */
            int numBytes = recvfrom(udp_sockfd_A, intervals_buffer, 8192, 0, NULL, NULL);
            if (numBytes < 0) {
                std::cerr << "Failed to receive interval data." << std::endl;
                 return 1;
            }

            std::vector<TimeInterval> bufferA(numBytes / sizeof(TimeInterval));
            // std::string buffer_str = buffer;
            // std::cout << "Check A data: " + buffer_str;
            memcpy(bufferA.data(), intervals_buffer, numBytes);
            intervalsA = bufferA;
            if(numBytes > 0){
                break;
            }
        }
        std::string intervalAStr = "[";
        if(intervalsA.size() == 1 && intervalsA[0].start == -1 && intervalsA[0].end == -1){
            int a = 1;
        }else{
            for (auto const& interval : intervalsA) {
                intervalAStr = intervalAStr + "[" + std::to_string(interval.start) + ", " + std::to_string(interval.end) + "], ";
            }
            if(intervalAStr.size() > 1){
                intervalAStr.pop_back();
                intervalAStr.pop_back();
            }
        }
        intervalAStr = intervalAStr + "]";
        std::cout << "Main Server received from server A the intersection result using UDP over port 23443: " + intervalAStr << std::endl;



        /* (4) Receiving the time intervals from serverB(UDP) */
        intervals_buffer = new char[8192];
        std::vector<TimeInterval> intervalsB;
        while(true){
            if(queryB.size() == 0){
                break;
            }
            std::cout << "listening for result from B" << std::endl;
            /* Forming UDP connection */
            int numBytes = recvfrom(udp_sockfd_B, intervals_buffer, 8192, 0, NULL, NULL);
            if (numBytes < 0) {
                std::cerr << "Failed to receive interval data." << std::endl;
                return 1;
            }

            std::vector<TimeInterval> bufferB(numBytes / sizeof(TimeInterval));
            memcpy(bufferB.data(), intervals_buffer, numBytes);
            intervalsB = bufferB;
            if(numBytes > 0){
                break;
            }
        }
        std::string intervalBStr = "[";
        for (auto const& interval : intervalsB) {
            intervalBStr = intervalBStr + "[" + std::to_string(interval.start) + ", " + std::to_string(interval.end) + "], ";
        }
        if(intervalBStr.size() > 1){
            intervalBStr.pop_back();
            intervalBStr.pop_back();
        }
        intervalBStr = intervalBStr + "]";
        std::cout << "Main Server received from server B the intersection result using UDP over port 23443: " + intervalBStr << std::endl;


        /* (5) Merge the time intervals */
        std::vector<TimeInterval> intervals = mergeInterval(intervalsA, intervalsB);
        std::string intervalStr = "[";
        for (auto const& interval : intervals) {
            intervalStr = intervalStr + "[" + std::to_string(interval.start) + ", " + std::to_string(interval.end) + "], ";
        }
        if(intervalStr.size() > 1){
            intervalStr.pop_back();
            intervalStr.pop_back();
        }
        intervalStr = intervalStr + "]";
        std::cout << "Found the intersection between the results from server A and B: " + intervalStr << std::endl;
        

        /* (6) Send the available intervals to client(TCP) */
        // char* response = new char[intervals.size() * sizeof(TimeInterval)];
        // memcpy(response, intervals.data(), intervals.size() * sizeof(TimeInterval));
        // ssize_t bytes_sent = send(client_sock, response, intervals.size() * sizeof(TimeInterval), 0);
        // if (bytes_sent == -1) {
        //     std::cerr << "Failed to send response to client\n";
        //     return 1;
        // }
        char* response = new char[intervalStr.size()];
        memcpy(response, intervalStr.data(), intervalStr.size());
        ssize_t bytes_sent = send(client_sock, response, intervalStr.size(), 0);
        if (bytes_sent == -1) {
            std::cerr << "Failed to send response to client\n";
            return 1;
        }

        std::cout << "Main Server sent the result to the client." << std::endl;

        // Close sockets
        close(client_sock);
    }
    close(tcp_sockfd);
}
