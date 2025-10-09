/* publisher_tcp.c
 * Compilar: gcc -o publisher_tcp publisher_tcp.c
 *
 * Uso: ./publisher_tcp <broker_ip> <pub_port> <topic> <num_msgs> [interval_ms]
 * Ejemplo: ./publisher_tcp 127.0.0.1 5000 MatchA 10 500
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

int main(int argc, char *argv[]) {
    if (argc < 5) {
        fprintf(stderr, "Uso: %s <broker_ip> <pub_port> <topic> <num_msgs> [interval_ms]\n", argv[0]);
        return 1;
    }
    char *broker = argv[1];
    int port = atoi(argv[2]);
    char *topic = argv[3];
    int num = atoi(argv[4]);
    int interval = 500;
    if (argc >= 6) interval = atoi(argv[5]);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    inet_pton(AF_INET, broker, &sa.sin_addr);

    if (connect(sock, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
        perror("connect");
        return 1;
    }

    printf("[PUBLISHER] Connected to %s:%d topic=%s sending=%d\n", broker, port, topic, num);

    for (int i = 1; i <= num; ++i) {
        char msg[LINE_BUF];
        int n = snprintf(msg, sizeof(msg), "PUBLISH %s Mensaje_%02d_del_pub_pid%d\n", topic, i, getpid());
        if (n > 0) {
            if (send_all(sock, msg, (size_t)n) < 0) {
                perror("send");
                break;
            }
            printf("[PUBLISHER] Sent: %s", msg);
        }
        usleep(interval * 1000); // ms -> us
    }

    close(sock);
    return 0;
}
