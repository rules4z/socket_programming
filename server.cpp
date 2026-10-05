#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main() {
    int s = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(s, (sockaddr*)&addr, sizeof(addr));
    listen(s, 1);
    int c = accept(s, NULL, NULL);
    char msg[] = "Hello Client";
    send(c, msg, sizeof(msg), 0);

    close(c);
    close(s);
}