#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define UDP_PORT_B 22443 
#define UDP_PORT_M 23443

struct TimeInterval {
    int start;
    int end;
};

bool compareInterval(TimeInterval i1, TimeInterval i2)
{
    return (i1.start < i2.start);
}  

std::vector<TimeInterval> mergeInterval(std::vector<TimeInterval> intervalsA, std::vector<TimeInterval> intervalsB){
    // std::cout << "Start merging...";
    std::vector<TimeInterval> ret;
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

int main() {
    std::string filename = "b.txt";
    std::ifstream input_file(filename);

    std::unordered_map<std::string, std::vector<TimeInterval> > intervals_map;
    std::unordered_set<std::string> names;

    std::string line;
    /* 1.Preprosessing: Read the name lists and store the intervals.*/
    while (std::getline(input_file, line)) {
        std::stringstream line_ss(line);
        std::string username;
        std::getline(line_ss, username, ';');

        std::vector<TimeInterval> intervals;

        // extract intervals from the line
        std::string interval_str;
        std::getline(line_ss, interval_str, ';');

        // remove brackets
        interval_str.erase(0, 2);
        interval_str.erase(interval_str.size() - 2);

        // extract individual intervals
        std::string delimiter = "],[";
        size_t pos = 0;
        while ((pos = interval_str.find(delimiter)) != std::string::npos) {
            std::string interval_token = interval_str.substr(0, pos);
            interval_str.erase(0, pos + delimiter.length());

            TimeInterval interval;
            std::stringstream interval_ss(interval_token);
            std::string start_str, end_str;
            std::getline(interval_ss, start_str, ',');
            std::getline(interval_ss, end_str, ',');
            interval.start = std::stoi(start_str);
            interval.end = std::stoi(end_str);

            intervals.push_back(interval);
        }

        // extract last interval
        TimeInterval interval;
        std::stringstream interval_ss(interval_str);
        std::string start_str, end_str;
        std::getline(interval_ss, start_str, ',');
        std::getline(interval_ss, end_str, ',');
        interval.start = std::stoi(start_str);
        interval.end = std::stoi(end_str);
        intervals.push_back(interval);

        intervals_map[username] = intervals;
        names.insert(username);
    }

    std::cout << "Server B is up and running using UDP on port 22443" << std::endl;
    // std::cout << "printing the results:\n";
    // // print out the intervals for each user
    // for (auto const & [username, intervals] : intervals_map) {
    //     std::cout << "User " << username << " has the following intervals:\n";
    //     for (auto const& interval : intervals) {
    //         std::cout << "[" << interval.start << "," << interval.end << "]\n";
    //     }
    // }

    /***
     * Send Name List
    */
    /* Send data to ServerM through UDP */
    int sockfd;
    char buffer[8192];
    struct sockaddr_in udp_cli_addr;

    // Creating socket file descriptor
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }

    memset(&udp_cli_addr, 0, sizeof(udp_cli_addr));

    // Filling server information
    udp_cli_addr.sin_family = AF_INET; // IPv4
    udp_cli_addr.sin_port = htons(UDP_PORT_M);
    udp_cli_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    /*Send number of names*/
    int size = names.size();
    sendto(sockfd, &size, sizeof(size), 0, (const struct sockaddr*)&udp_cli_addr, sizeof(udp_cli_addr));
    // std::cout << size << std::endl;
//    sssendto(sock, &num, sizeof(num), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));

    for (const auto& name : names) {
        memset(buffer, 0, sizeof(buffer));
        strncpy(buffer, name.c_str(), sizeof(buffer));
        sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr*)&udp_cli_addr, sizeof(udp_cli_addr));
    }
    
    std::cout << "Server B finished sending a list of usernames to Main Server" << std::endl;

    /***
     * Receive Query Names
    */
   struct sockaddr_in udp_serv_addr;

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
    udp_serv_addr.sin_port = htons(UDP_PORT_B);

    // Bind the socket with the server address
    if (bind(udp_sockfd, (const struct sockaddr *)&udp_serv_addr, sizeof(udp_serv_addr)) < 0) {
        std::cerr << "UDP Bind failed" << std::endl;
        return 1;
    }

    int len, n;
    /***
     * Listening for queries
    */
    while(true){
        int queryLen = 0;
        std::vector<std::string> query_names;
        /* Read query size */
        while(true){
            // std::cout << "Receiving query length from serverM" << std::endl;
            int bytesReceived = recvfrom(udp_sockfd, buffer, sizeof(buffer), 0, (sockaddr*)&udp_cli_addr, 
            reinterpret_cast<socklen_t *>(&len));
            if (bytesReceived < 0) {
                std::cerr << "Failed to receive data." << std::endl;
                return 1;
            }
            memcpy(&queryLen, buffer, sizeof(queryLen));
            if(queryLen != 0){
                break;
            }
        }

        // If the query has 0 names
        if(queryLen == 0){
            continue;
        }

        /* Receiving query list from serverM */
        // std::cout << "listening for the name lists" << std::endl;
        while(true){
            /* Forming UDP connection */
            len = sizeof(udp_cli_addr); // len is value/resuslt
            n = recvfrom(udp_sockfd, (char *)buffer, 1024,
                        MSG_WAITALL, (struct sockaddr *)&udp_cli_addr,
                        reinterpret_cast<socklen_t *>(&len));
            buffer[n] = '\0';
            std::string name(buffer);
            query_names.push_back(name);

            if(query_names.size() == queryLen){
                break;
            }
        }

        std::cout << "Server B received the usernames from Main Server using UDP over port 22443." << std::endl;

        // for (const auto& t : query_names) {
        //     std::cout << t + " ";
        // }
        // std::cout << "\n";

        /* Merge the intervals */
        std::vector<TimeInterval> combined;
        if(query_names.size() > 0){
            combined = intervals_map[query_names[0]];
        }
        for(std::string name : query_names){
            std::vector<TimeInterval> cur = intervals_map[name];
            combined = mergeInterval(combined, cur);
        }

        std::string intervalStr = "[";
        for (auto const& interval : combined) {
            intervalStr = intervalStr + "[" + std::to_string(interval.start) + ", " + std::to_string(interval.end) + "], ";
        }
        if(intervalStr.size() > 1){
            intervalStr.pop_back();
            intervalStr.pop_back();
        }
        intervalStr = intervalStr + "]";

        std::string names = "";
        for(std::string name : query_names){
            names = names + name + ", ";
        }
        names.pop_back();
        names.pop_back();

        std::cout << "Found the intersection result: " + intervalStr + " for " + names + "." << std::endl;
        // std::cout << "The intervals available are: \n";
        // for (auto const& interval : combined) {
        //     std::cout << "[" << interval.start << "," << interval.end << "]\n";
        // }

        if(combined.size() == 0){
            TimeInterval no_result = {-1, -1};
            combined.push_back(no_result);
        }
        /* Send the result back to serverM */
        char* interval_buffer = new char[combined.size() * sizeof(TimeInterval)];
        memcpy(interval_buffer, combined.data(), combined.size() * sizeof(TimeInterval));
        ssize_t bytes_sent = sendto(sockfd, interval_buffer, combined.size() * sizeof(TimeInterval), 0, (struct sockaddr*)&udp_cli_addr, sizeof(udp_cli_addr));
        // std::cout << "Sent " << bytes_sent << " bytes to client: " << interval_buffer << "\n";
        std::cout << "Server B finished sending the response to Main Server." << std::endl;
    }
}


