#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <config-file>\n";
        return 1;
    }
    ifstream file(argv[1]);
    if (!file) {
        cerr << "Error: cannot open config file\n";
        return 1;
    }
    vector<vector<int>> rows;
    string line;
    int lineNumber = 0;
    while (getline(file, line)) {
        ++lineNumber;
        size_t comment = line.find('#');
        if (comment != string::npos) {
            line.erase(comment);
        }
        istringstream input(line);
        vector<int> row;
        int cost;
        while (input >> cost) {
            row.push_back(cost);
        }

        if (!input.eof()) {
            cerr << "Invalid value on line " << lineNumber << '\n';
            return 1;
        }
        if (!row.empty()) {
            rows.push_back(row);
        }
    }
    if (rows.empty()) {
        cerr << "Error: config must contain at least one row\n";
        return 1;
    }
    const size_t n = rows.size() + 1;
    if (n > 26) {
        cerr<<"Error: maximum 26 nodes supported\n";
        return 1;
    }
    for (size_t i = 0; i < rows.size(); ++i) {
        size_t expected = n - i - 1;
        if (rows[i].size() != expected) {
            cerr<<"Error: row " << i + 1
                <<" must contain "<<expected<< " costs\n";
            return 1;
        }
        for (int c : rows[i]) {
            if (c != -1 && c <= 0) {
                cerr<<"Error: costs must be positive or -1\n";
                return 1;
            }
        }
    }
    vector<vector<int>> graph(n, vector<int>(n, -1));
    for (size_t i = 0; i < n; ++i) {
        graph[i][i] = 0;
    }
    for (size_t i = 0; i < rows.size(); ++i) {
        for (size_t j = i + 1; j < n; ++j) {
            int c = rows[i][j - i - 1];
            graph[i][j] = c;
            graph[j][i] = c;
        }
    }
    cout<<"Parsed "<<n<<" nodes.\n";
    for (size_t i=0;i<n; ++i) {
        cout<<char('A' + i) << ": ";
        bool first = true;
        for (size_t j=0; j<n;++j) {
            if (i != j && graph[i][j] != -1) {
                if (!first) cout << ", ";
                cout << char('A' + j)
                     << '(' << graph[i][j] << ')';
                first = false;
            }
        }
        if (first) cout<<"(no neighbours)";
        cout<<'\n';
    }
    return 0;
}
