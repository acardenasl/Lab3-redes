# Laboratorio Tres

Laboratorio 3 hecho por:
Andrés Cárdenas Layton 202122083  
David Felipe Velasquez Parra 202321492  
Maria Paula Ospina Plazas 202123208  


# Libreria para UDP

Librería utilizada

En este proyecto (archivos broker_udp.c, publisher_udp.c y subscriber_udp.c) solo se emplea la librería estándar <stdio.h>.
Esta librería pertenece al estándar ANSI C y proporciona funciones para la entrada y salida estándar (E/S), como printf(), putchar() o fgets().
Su uso aquí se limita a mostrar mensajes informativos en la terminal, ya que todas las operaciones de red (creación de sockets, envío y recepción de datagramas) se implementan directamente mediante llamadas al sistema (syscall), sin incluir otras librerías como <sys/socket.h> o <arpa/inet.h>.

Función printf()

La función printf() es una de las más utilizadas del encabezado <stdio.h> y permite imprimir texto formateado en la salida estándar (por defecto, la consola).
Su prototipo es el siguiente:

int printf(const char *formato, ...);

Parámetros:

formato → cadena de texto con posibles especificadores (%d, %s, %lu, etc.).

... → lista de valores que reemplazan los especificadores dentro de la cadena.

Funcionamiento interno:

1. printf() analiza la cadena de formato.
2. Sustituye los valores de los argumentos variables.

3. Envía el texto resultante al file descriptor 1 (stdout) mediante una llamada interna a write() del kernel.
Ejemplo en este proyecto:
printf("Mensaje recibido: %s\n", buffer);


