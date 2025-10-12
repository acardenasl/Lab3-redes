#include <stdio.h>

long syscall(long n, ...);

#define SYS_socket 41
#define SYS_sendto 44
#define SYS_recvfrom 45
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
    int s = syscall(SYS_socket, AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in a;
    a.sin_family = AF_INET;
    a.sin_port = htons(PORT);
    a.sin_addr.s_addr = (127) | (0<<8)| (0<<16) | (1<<24)  ;
    for (int i = 0; i < 8; i++) a.sin_zero[i] = 0;

    syscall(SYS_sendto, s, "SUBSCRIBE", 9, 0, &a, sizeof(a));
    printf("Suscriptor conectado y esperando mensajes...\n");

    char b[256];
    while (1) {
        int n = syscall(SYS_recvfrom, s, b, 256, 0, 0, 0);
        if (n > 0) {
            // En este punto, el suscriptor ha recibido un mensaje del broker.
            // El contenido está en el arreglo 'b' con longitud 'n' bytes.
            b[n] = '\0'; 
	    printf("Mensaje recibido del broker: %s\n",b);
        }
    }

    syscall(SYS_close, s);
    return 0;
}
