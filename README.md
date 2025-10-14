# Laboratorio Tres
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

# Explicación del sistema UDP

El sistema implementa un modelo Publicador–Suscriptor (Publish/Subscribe) usando sockets UDP en lenguaje C, sin librerías externas, únicamente a través de llamadas de sistema (syscall). Este enfoque permite comprender cómo se comunican procesos mediante datagramas no orientados a conexión.

1. Broker (broker_udp.c)

El broker actúa como el intermediario central que recibe todos los mensajes de los publicadores y los reenvía a los suscriptores registrados.
	•	Crea un socket UDP con SYS_socket(AF_INET, SOCK_DGRAM, 0).
	•	Lo asocia al puerto 8080 mediante SYS_bind.
	•	Escucha permanentemente con SYS_recvfrom cualquier datagrama entrante.
	•	Si el mensaje recibido es "SUBSCRIBE", guarda la dirección del cliente en un arreglo de suscriptores (subs[]).
	•	Si el mensaje proviene de un publicador, lo imprime en pantalla y lo reenvía a todos los suscriptores mediante SYS_sendto.
	•	No hay control de flujo, confirmaciones ni retransmisión: los mensajes pueden perderse o llegar fuera de orden, lo cual refleja el comportamiento real de UDP.
2. Suscriptor (subscriber_udp.c)

El suscriptor representa a un usuario que desea recibir actualizaciones en tiempo real.
	•	Crea un socket UDP y define la dirección del broker (127.0.0.1:8080).
	•	Envía una solicitud de suscripción ("SUBSCRIBE") al broker.
	•	Luego, permanece escuchando indefinidamente con SYS_recvfrom.
	•	Cada datagrama recibido se imprime en pantalla como una nueva noticia.
3. Publicador (publisher_udp.c)

El publicador simula al periodista deportivo que reporta los eventos del partido.
	•	Crea un socket UDP y configura la dirección destino (el broker).
	•	En un bucle infinito, lee texto desde la entrada estándar (teclado) usando SYS_read.
	•	Cada mensaje se envía como datagrama UDP al broker usando SYS_sendto.
	•	No requiere conexión previa ni confirmación: simplemente “lanza” cada datagrama.


