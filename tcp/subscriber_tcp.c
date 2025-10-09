/* subscriber_tcp.c
 * Compilar: gcc -o subscriber_tcp subscriber_tcp.c
 *
 * Uso: ./subscriber_tcp <broker_ip> <sub_port> <topic>
 * Ejemplo: ./subscriber_tcp 127.0.0.1 5001 MatchA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define LINE_BUF 2048

static int send_all(int sock, const char *buf, size_t len) {
    size_t total = 0;
    while (total < len) {
        ssize_t s = send(sock, buf + total, len - total, 0);
        if (s <= 0) return -1;
        total += s;
    }
    return 0;
}

static int read_line(int sock, char *buf, size_t maxlen) {
    size_t idx = 0;
    while (idx < maxlen - 1) {
        char c;
        ssize_t r = recv(sock, &c, 1, 0);
        if (r == 1) {
            buf[idx++] = c;
            if (c == '\n') break;
        } else if (r == 0) {
            if (idx == 0) return 0;
            break;
        } else {
            return -1;
        }
    }
    buf[idx] = '\0';
    return (int)idx;
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Uso: %s <broker_ip> <sub_port> <topic>\n", argv[0]);
        return 1;
    }
    char *broker = argv[1];
    int port = atoi(argv[2]);
    char *topic = argv[3];

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    inet_pton(AF_INET, broker, &sa.sin_addr);

    if (connect(sock, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
        perror("connect");
        return 1;
    }

    char subscribe_line[LINE_BUF];
    snprintf(subscribe_line, sizeof(subscribe_line), "SUBSCRIBE %s\n", topic);
    if (send_all(sock, subscribe_line, strlen(subscribe_line)) < 0) {
        perror("send subscribe");
        close(sock);
        return 1;
    }
    printf("[SUBSCRIBER] Subscribed to %s\n", topic);

    char line[LINE_BUF];
    while (1) {
        int r = read_line(sock, line, sizeof(line));
        if (r <= 0) break;
        printf("[SUBSCRIBER] Received: %s", line);
    }

    printf("[SUBSCRIBER] Disconnected from broker\n");
    close(sock);
    return 0;
}
