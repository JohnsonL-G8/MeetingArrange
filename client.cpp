#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_set>

#define TCP_PORT_M 24443 // TCP port number
#define BUFFER_SIZE 1024 // buffer size for receiving data

struct TimeInterval {
    int start;
    int end;
};

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
    // create socket
    std::cout << "Client is up and running." << std::endl;

    while(true){
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
        std::cout << "Please enter the usernames to check schedule availability: " << std::endl;
        std::string names;
        std::getline(std::cin, names);
        names = parseInput(names);

        // save names into a unordered_set
        std::vector<std::string> query_names;
        std::unordered_set<std::string> invalid_names;
        std::stringstream ss(names);
        std::string name;
        
        while (getline(ss, name, ' ')) {
            query_names.push_back(name);
        }

        // send list of names to server
        if (send(sock, names.c_str(), names.length(), 0) < 0) {
            std::cerr << "Error sending data to server\n";
            close(sock);
            return -1;
        }

        std::cout << "Client finished sending the usernames to Main Server." << std::endl;

        /* Receive the invalid names from serverM */
        char buffer[8192];
        int numBytes;
        memset(buffer, 0, 8192);
        numBytes = read(sock, buffer, 8192);
        std::string invalid = "";
        // std::cout << "before copy names\n";
        invalid = buffer;
        // std::cout << "After read intervals" + std::to_string(numBytes) + "\n";
        // std::cout << "Invalid: " + invalid + "\n";

        if(invalid != "127"){
            std::cout << "Client received the reply from Main Server using TCP over port 24443: " + invalid + " do not exist" << std::endl;
            std::string delimiter = ", ";
            size_t pos = 0;
            int count = 0;
            while ((pos = invalid.find(delimiter)) != std::string::npos) {
                std::string token = invalid.substr(0, pos);
                invalid_names.insert(token);
                invalid.erase(0, pos + delimiter.length());
                count++;
            }
            invalid_names.insert(invalid);
            std::cout << "invalid name size: " + std::to_string(invalid_names.size()) + "\n";
            if(invalid_names.size() == query_names.size()){
                std::cout << "All input names are invalid" << std::endl;
                std::cout << "-----Start a new request-----" << std::endl;
                continue;
            }
        }


        /* Receive data from serverM */
        // Wait for response from server
        // std::vector<TimeInterval> intervals(numBytes / sizeof(TimeInterval));
        // for (auto const& interval : intervals) {
        //     std::cout << "[" << interval.start << "," << interval.end << "]\n";
        // }
        memset(buffer, 0, 8192);
        std::string intervals;
        numBytes = read(sock, buffer, 8192);
        intervals = buffer;
        // std::cout << "Intervals: " + intervals + "\n";
        std::string valid_names;
        for(std::string t : query_names){
            if(invalid_names.find(t) != invalid_names.end()){
                continue;
            }else{
                valid_names = valid_names + t + ", ";
            }
        }
        valid_names.pop_back();
        valid_names.pop_back();
        std::cout << "Client received the reply from Main Server using TCP over port 24443: Time intervals " + intervals + 
        " works for " + valid_names + "." << std::endl;
        // std::cout << buffer + "\n";
        // memcpy(intervals.data(), buffer, numBytes);
        

        std::cout << "-----Start a new request-----" << std::endl;

        close(sock);
    }
}
