#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdint>
#include <cstring>
using namespace std;

//vn info
struct NodeInfo {
    int client_fd;
    in_addr ip;
    uint16_t udp_port; //since udp 2 bytes
};

//to make sure server gets all the required bytes
bool recv_all(int fd, char* buffer, size_t length) {
    size_t received = 0;
    while (received<length) {
        ssize_t bytes = recv(fd,buffer+received,length-received,0);
        if (bytes<=0) {
            return false;
        }
        received+=bytes;
    }
    return true;
}

//to make sure server sends all the required bytes
bool send_all(int fd, const char* buffer, size_t length) {
    size_t sent = 0;
    while (sent<length) {
        ssize_t bytes=send(fd,buffer+sent,length-sent,0);
        if (bytes<=0) {
            return false;
        }
        sent += bytes;
    }
    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr<<"Usage:./oracle <config-file>\n";
        return 1;
    }
    ifstream file(argv[1]);
    if (!file.is_open()) {
        cerr<<"Could not open config file\n";
        return 1;
    }
    string line;
    vector<vector<int>> rows;
    while (getline(file,line)) {
        size_t comment=line.find('#');
        if (comment!=string::npos) {
            line.erase(comment);
        }
        istringstream input(line);
        vector<int> row;
        int cost;
        while (input>>cost) {
            row.push_back(cost);
        }
        if (!row.empty()) {
            rows.push_back(row);
        }
    }
    if (rows.empty()) {
        cerr<<"Configuration file is empty: no network defined\n";
        return 1;
    }
    int n=rows.size()+1;
    if (n>26) {
        cerr<<"Too many nodes\n";
        return 1;
    }
    for (int i=0; i<n-1;i++) {
        int expected=n-i-1;
        if ((rows[i].size())!=expected) {
            cerr<<"Incorrect number of costs in row "<<i+1<<'\n';
            return 1;
        }
        for (int cost:rows[i]) {
            if (cost!=-1 && cost<=0) {
                cerr<<"Invalid link cost\n";
                return 1;
            }
        }
    }
    //this is our adjacency matrix
    vector<vector<int>> graph(n,vector<int>(n,-1));
    for (int i=0; i<n; i++) {graph[i][i] = 0;}
    for (int i=0; i<n-1; i++) {
        for (int j=0; j<rows[i].size(); j++) {
            int u=i;
            int v=i+j+1;
            int cost=rows[i][j];
            graph[u][v]=cost;
            graph[v][u]=cost;
        }
    }


    //construction of socket
    int server_fd=socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        cerr<<"Failed to create socket\n";
        return 1;
    }
    struct sockaddr_in server_addr{};
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(5000);
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    //assuming we need extensive checking for error
    if (bind(server_fd,(struct sockaddr*)&server_addr,sizeof(server_addr))==-1) {
        cerr<<"Failed to bind to port 5000\n";
        close(server_fd);
        return 1;
    }
    //now supposedly bound to port 5000
    cout<<"bound to port 5000"<<endl;

    if (listen(server_fd,SOMAXCONN)<0) {
        cerr<<"Failed to listen\n";
        return 1;
    }
    cout<<"Oracle Node is listening on port 5000\n";

    //test with more nodes
    vector<NodeInfo> nodes;
    while (nodes.size()<n) {
        struct sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd,(struct sockaddr*)&client_addr,&client_len);
        if (client_fd < 0) {
            cerr << "Failed to accept connection\n";
            continue;
        }
        char buffer[6];
        if (!recv_all(client_fd, buffer, sizeof(buffer))) {
            cerr << "Failed to receive CONNECT message\n";
            close(client_fd);
            continue;
        }
        NodeInfo node{};
        node.client_fd = client_fd;
        memcpy(&node.ip.s_addr, buffer, 4);
        uint16_t port_network;
        memcpy(&port_network, buffer + 4, 2);
        node.udp_port = ntohs(port_network);
        char node_id = 'A' + nodes.size();
        char ip_text[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &node.ip, ip_text, sizeof(ip_text));
        cout << "Received CONNECT from node " << node_id<< ": IP = " << ip_text << ", UDP port = " << node.udp_port << '\n';
        nodes.push_back(node);
    }
    cout << "Received CONNECT messages from all configured nodes\n";

    for (int i = 0; i < n; i++) {
        vector<char> message(2, 0);
        uint16_t tuple_count = 0;
        for (int j = 0; j < n; j++) {
            if (i != j && graph[i][j] <= 0) {
                continue;
            }
            message.push_back(static_cast<char>('A' + j));
            const char* ip_bytes =reinterpret_cast<const char*>(&nodes[j].ip.s_addr);
            message.insert(message.end(), ip_bytes, ip_bytes + 4);
            uint16_t port_network = htons(nodes[j].udp_port);
            const char* port_bytes =reinterpret_cast<const char*>(&port_network);
            message.insert(message.end(), port_bytes, port_bytes + 2);
            uint32_t cost = (i == j)? 0: static_cast<uint32_t>(graph[i][j]);
            uint32_t cost_network = htonl(cost);
            const char* cost_bytes =reinterpret_cast<const char*>(&cost_network);
            message.insert(message.end(), cost_bytes, cost_bytes + 4);
            tuple_count++;
        }
        uint16_t count_network = htons(tuple_count);
        memcpy(message.data(), &count_network, sizeof(count_network));
        if (!send_all(nodes[i].client_fd,message.data(),message.size())) {
            cerr << "Failed to send LINK-STATE to node "<< static_cast<char>('A' + i) << '\n';
            close(nodes[i].client_fd);
            nodes[i].client_fd = -1;
            continue;
        }
        cout << "Sent LINK-STATE to node "<< static_cast<char>('A' + i)<< " with " << tuple_count << " tuples\n";
    }

    cout << "Finished sending LINK-STATE messages\n";

    return 0;
}