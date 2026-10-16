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
    
    return 0;
}