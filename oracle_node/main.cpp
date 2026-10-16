#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
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
    return 0;
}