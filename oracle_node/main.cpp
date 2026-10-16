#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
using namespace std;

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

    //test with one node
    struct sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(
        server_fd,
        (struct sockaddr*)&client_addr,
        &client_len
    );
    if (client_fd < 0) {
        cerr << "Failed to accept connection\n";
        return 1;
    }
    cout << "A Virtual Node connected\n";
    return 0;
}