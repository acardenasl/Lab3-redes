#include <stdio.h>

long syscall(long n, ...);

#define SYS_socket 41
#define SYS_sendto 44
#define SYS_close 3
#define AF_INET 2
#define SOCK_DGRAM 2
#define PORT 8080

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

unsigned short htons(unsigned short x) { return (x << 8) | (x >> 8); }

int main() {
    const unsigned long total = 60000;          // Número total de mensajes
    const char *base = "Gol";                   // Mensaje base
    const char *team = "de Colombia";           // Equipo fijo

    int s = syscall(SYS_socket, AF_INET, SOCK_DGRAM, 0);
    if (s < 0) return 1;

    struct sockaddr_in a;
    a.sin_family = AF_INET;
    a.sin_port = htons(PORT);

    // IP codificada manualmente: 127.0.0.1 (localhost)
    a.sin_addr.s_addr = (127) | (0<<8) | (0<<16) | (1<<24);
    for (int i = 0; i < 8; i++) a.sin_zero[i] = 0;

    char msg[256];
    for (unsigned long i = 1; i <= total; i++) {
        int len = sprintf(msg, "%s #%lu %s", base, i, team);
        syscall(SYS_sendto, s, msg, len, 0, &a, sizeof(a));

        // Mostrar progreso cada 1000 envíos
        if (i % 1000 == 0) {
            printf("Enviados %lu/%lu\n", i, total);
        }
    }

    printf("Envío completado: %lu mensajes enviados.\n", total);
    syscall(SYS_close, s);
    return 0;
}
