/* broker_tcp.c
 * Compilar: gcc -o broker_tcp broker_tcp.c -lpthread
 *
 * Uso: ./broker_tcp <pub_port> <sub_port>
 * Ejemplo: ./broker_tcp 5000 5001
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define MAX_TOPIC_LEN 128
#define MAX_MSG_LEN 1024
#define LINE_BUF 2048
#define BACKLOG 10

typedef struct Subscriber {
    int sockfd;
    struct Subscriber *next;
} Subscriber;

typedef struct Topic {
    char name[MAX_TOPIC_LEN];
    Subscriber *subs;
    struct Topic *next;
} Topic;

static Topic *topics_head = NULL;
static pthread_mutex_t topics_lock = PTHREAD_MUTEX_INITIALIZER;

static int send_all(int sock, const char *buf, size_t len) {
    size_t total = 0;
    while (total < len) {
        ssize_t sent = send(sock, buf + total, len - total, 0);
        if (sent <= 0) return -1;
        total += sent;
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
            if (idx == 0) return 0; // closed
            break;
        } else {
            if (errno == EINTR) continue;
            return -1;
        }
    }
    buf[idx] = '\0';
    return (int)idx;
}

void add_subscriber(const char *topic_name, int sock) {
    pthread_mutex_lock(&topics_lock);
    Topic *t = topics_head;
    while (t) {
        if (strncmp(t->name, topic_name, MAX_TOPIC_LEN) == 0) break;
        t = t->next;
    }
    if (!t) {
        t = (Topic*)malloc(sizeof(Topic));
        strncpy(t->name, topic_name, MAX_TOPIC_LEN);
        t->subs = NULL;
        t->next = topics_head;
        topics_head = t;
    }
    // add subscriber (if not already present)
    Subscriber *s = (Subscriber*)malloc(sizeof(Subscriber));
    s->sockfd = sock;
    s->next = t->subs;
    t->subs = s;
    pthread_mutex_unlock(&topics_lock);
}

void remove_subscriber_from_all(int sock) {
    pthread_mutex_lock(&topics_lock);
    Topic *t = topics_head;
    while (t) {
        Subscriber **cur = &t->subs;
        while (*cur) {
            if ((*cur)->sockfd == sock) {
                Subscriber *tmp = *cur;
                *cur = tmp->next;
                free(tmp);
            } else {
                cur = &(*cur)->next;
            }
        }
        t = t->next;
    }
    pthread_mutex_unlock(&topics_lock);
}

void broadcast_to_topic(const char *topic_name, const char *message) {
    pthread_mutex_lock(&topics_lock);
    Topic *t = topics_head;
    while (t && strncmp(t->name, topic_name, MAX_TOPIC_LEN) != 0) t = t->next;
    if (!t) {
        pthread_mutex_unlock(&topics_lock);
        return;
    }

    Subscriber **cur = &t->subs;
    while (*cur) {
        Subscriber *s = *cur;
        char out[LINE_BUF];
        int n = snprintf(out, sizeof(out), "EVENT %s %s\n", topic_name, message);
        if (n < 0) { cur = &(*cur)->next; continue; }
        if (send_all(s->sockfd, out, (size_t)n) < 0) {
            // remove this subscriber (failed send)
            Subscriber *tmp = *cur;
            *cur = tmp->next;
            close(tmp->sockfd);
            free(tmp);
        } else {
            cur = &(*cur)->next;
        }
    }
    pthread_mutex_unlock(&topics_lock);
}

typedef struct {
    int listenfd;
    int is_publisher; // 1 => publisher port, 0 => subscriber port
} acceptor_arg_t;

void *publisher_handler(void *arg_ptr) {
    int sock = *((int*)arg_ptr);
    free(arg_ptr);
    char line[LINE_BUF];

    while (1) {
        int r = read_line(sock, line, sizeof(line));
        if (r <= 0) break; // closed or error
        if (line[r-1] == '\n') line[r-1] = '\0';
        if (strncmp(line, "PUBLISH ", 8) == 0) {
            char *p = line + 8;
            char *space = strchr(p, ' ');
            if (!space) continue;
            *space = '\0';
            char *topic = p;
            char *message = space + 1;
            printf("[BROKER] Publish received: topic='%s' msg='%s'\n", topic, message);
            broadcast_to_topic(topic, message);
        } else {
            // unknown command from publisher -> ignore or log
            printf("[BROKER] Unknown publisher command: %s\n", line);
        }
    }

    close(sock);
    return NULL;
}

void *subscriber_handler(void *arg_ptr) {
    int sock = *((int*)arg_ptr);
    free(arg_ptr);
    char line[LINE_BUF];

    while (1) {
        int r = read_line(sock, line, sizeof(line));
        if (r <= 0) break;
        if (line[r-1] == '\n') line[r-1] = '\0';
        if (strncmp(line, "SUBSCRIBE ", 10) == 0) {
            char *topic = line + 10;
            add_subscriber(topic, sock);
            printf("[BROKER] Subscriber %d subscribed to '%s'\n", sock, topic);
        } else if (strncmp(line, "UNSUBSCRIBE ", 12) == 0) {
            // Optionally implement unsubscribe (not required)
        } else {
            printf("[BROKER] Unknown subscriber command: %s\n", line);
        }
    }

    printf("[BROKER] Subscriber disconnected: %d\n", sock);
    remove_subscriber_from_all(sock);
    close(sock);
    return NULL;
}

void *acceptor_loop(void *arg) {
    acceptor_arg_t a = *((acceptor_arg_t*)arg);
    free(arg);
    while (1) {
        struct sockaddr_in cli;
        socklen_t clen = sizeof(cli);
        int client = accept(a.listenfd, (struct sockaddr*)&cli, &clen);
        if (client < 0) {
            perror("accept");
            continue;
        }
        int *pclient = malloc(sizeof(int));
        *pclient = client;
        pthread_t tid;
        if (a.is_publisher) {
            pthread_create(&tid, NULL, publisher_handler, pclient);
            pthread_detach(tid);
        } else {
            pthread_create(&tid, NULL, subscriber_handler, pclient);
            pthread_detach(tid);
        }
    }
    return NULL;
}

int create_and_bind(int port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { perror("socket"); exit(1); }
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port = htons(port);
    if (bind(fd, (struct sockaddr*)&sa, sizeof(sa)) < 0) { perror("bind"); close(fd); exit(1); }
    if (listen(fd, BACKLOG) < 0) { perror("listen"); close(fd); exit(1); }
    return fd;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <pub_port> <sub_port>\n", argv[0]);
        exit(1);
    }
    int pub_port = atoi(argv[1]);
    int sub_port = atoi(argv[2]);

    int pub_listen = create_and_bind(pub_port);
    int sub_listen = create_and_bind(sub_port);

    printf("[BROKER] Listening pub:%d sub:%d\n", pub_port, sub_port);

    // Start acceptor threads
    acceptor_arg_t *ap1 = malloc(sizeof(acceptor_arg_t));
    ap1->listenfd = pub_listen; ap1->is_publisher = 1;
    pthread_t t1;
    pthread_create(&t1, NULL, acceptor_loop, ap1);

    acceptor_arg_t *ap2 = malloc(sizeof(acceptor_arg_t));
    ap2->listenfd = sub_listen; ap2->is_publisher = 0;
    pthread_t t2;
    pthread_create(&t2, NULL, acceptor_loop, ap2);

    // Main thread can wait
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    close(pub_listen);
    close(sub_listen);
    return 0;
}
