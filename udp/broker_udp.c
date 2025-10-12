#include <stdio.h>

long syscall(long n, ...);

#define SYS_socket 41
#define SYS_bind 49
#define SYS_recvfrom 45
#define SYS_sendto 44
#define SYS_close 3

#define AF_INET 2
#define SOCK_DGRAM 2
#define PORT 8080
#define BUF_SIZE 256
#define MAX_SUBS 10

typedef unsigned short sa_family_t;
typedef unsigned int socklen_t;
typedef unsigned int in_addr_t;

struct in_addr { in_addr_t s_addr; };
struct sockaddr_in {
    sa_family_t sin_family;
    unsigned short sin_port;
    struct in_addr sin_addr;
    unsigned char sin_zero[8];
};

struct sockaddr_in subs[MAX_SUBS];
int sub_count = 0;

unsigned short htons(unsigned short x) { return (x << 8) | (x >> 8); }

int main() {
    int sock;
    struct sockaddr_in addr, client;
    char buffer[BUF_SIZE];
    socklen_t len = sizeof(client);
    int n;

    sock = syscall(SYS_socket, AF_INET, SOCK_DGRAM, 0);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = 0; // INADDR_ANY
    for (int i = 0; i < 8; i++) addr.sin_zero[i] = 0;

    syscall(SYS_bind, sock, &addr, sizeof(addr));

    printf("Broker iniciado\n");

    while (1) {
        n = syscall(SYS_recvfrom, sock, buffer, BUF_SIZE, 0, &client, &len);
        if (n <= 0) continue;

        // Aceptar tanto SUBSCRIBE como SUBSCRIBE\n o en minúsculas
        if (n >= 9) {
            if ((buffer[0]=='S' || buffer[0]=='s') &&
                (buffer[1]=='U' || buffer[1]=='u') &&
                (buffer[2]=='B' || buffer[2]=='b') &&
                (buffer[3]=='S' || buffer[3]=='s') &&
                (buffer[4]=='C' || buffer[4]=='c') &&
                (buffer[5]=='R' || buffer[5]=='r') &&
                (buffer[6]=='I' || buffer[6]=='i') &&
                (buffer[7]=='B' || buffer[7]=='b') &&
                (buffer[8]=='E' || buffer[8]=='e')) {

                if (sub_count < MAX_SUBS) {
                    subs[sub_count++] = client;
                    printf("Nuevo suscriptor registrado (%d total)\n", sub_count);
                }
                continue;
            }
        }

        // Mostrar mensaje recibido del publicador
        printf("Mensaje recibido: ");
        for (int i = 0; i < n; i++) putchar(buffer[i]);
        printf("\n");

        // Reenviar a todos los suscriptores
        for (int i = 0; i < sub_count; i++) {
            syscall(SYS_sendto, sock, buffer, n, 0, &subs[i], sizeof(subs[i]));
        }

        printf("Mensaje reenviado a %d suscriptores\n", sub_count);
    }

    syscall(SYS_close, sock);
    return 0;
}
