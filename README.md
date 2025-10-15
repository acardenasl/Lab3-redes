# Laboratorio Tres
Andrés Cárdenas Layton 202122083  
David Felipe Velasquez Parra 202321492  
Maria Paula Ospina Plazas 202123208  


# Libreria para UDP

En este proyecto (archivos broker_udp.c, publisher_udp.c y subscriber_udp.c) solo se emplea la librería estándar <stdio.h>, la cual pertenece al estándar ANSI C y proporciona funciones para la entrada y salida estándar (E/S), como printf(), putchar() o fgets().
Su uso para este laboratorio se limita a mostrar mensajes informativos en la terminal, ya que todas las operaciones de red (creación de sockets, envío y recepción de datagramas) se implementan directamente mediante llamadas al sistema (syscall), sin incluir otras librerías como <sys/socket.h> o <arpa/inet.h>.

Es importante mencionar que se utilizo la funcion printf(), por lo cual se realiza la siguiente documentacion:

- Función printf(): Es una de las más utilizadas del encabezado <stdio.h> y permite imprimir texto formateado en la salida estándar (por defecto, la consola).

	- Su prototipo es el siguiente: int printf(const char *formato, ...);
	
	- Parámetros:
	
		formato → cadena de texto con posibles especificadores (%d, %s, %lu, etc.).
	
		... → lista de valores que reemplazan los especificadores dentro de la cadena.
	
	- Funcionamiento interno:
	
		1. printf() analiza la cadena de formato.
		2. Sustituye los valores de los argumentos variables.
		3. Envía el texto resultante al file descriptor 1 (stdout) mediante una llamada interna a write() del kernel.
	
	- Ejemplo en este proyecto: printf("Mensaje recibido: %s\n", buffer);

# Explicación del sistema UDP

El sistema implementa un modelo Publicador–Suscriptor (Publish/Subscribe) usando sockets UDP en lenguaje C, sin librerías externas, únicamente a través de llamadas de sistema (syscall). Este enfoque permite comprender cómo se comunican procesos mediante datagramas no orientados a conexión.

**Broker (broker_udp.c)**: El broker actúa como el intermediario central que recibe todos los mensajes de los publicadores y los reenvía a los suscriptores registrados.
     
		•	Crea un socket UDP con SYS_socket(AF_INET, SOCK_DGRAM, 0).

		•	Lo asocia al puerto 8080 mediante SYS_bind.

		•	Escucha permanentemente con SYS_recvfrom cualquier datagrama entrante.
 
		•	Si el mensaje recibido es "SUBSCRIBE", guarda la dirección del cliente en un arreglo de suscriptores (subs[]).
 
		•	Si el mensaje proviene de un publicador, lo imprime en pantalla y lo reenvía a todos los suscriptores mediante SYS_sendto.

		•	No hay control de flujo, confirmaciones ni retransmisión: los mensajes pueden perderse o llegar fuera de orden, lo cual refleja el comportamiento real de UDP.

**Suscriptor (subscriber_udp.c)**: El suscriptor representa a un usuario que desea recibir actualizaciones en tiempo real.
   
		•	Crea un socket UDP y define la dirección del broker (127.0.0.1:8080).

		•	Envía una solicitud de suscripción ("SUBSCRIBE") al broker.

		•	Luego, permanece escuchando indefinidamente con SYS_recvfrom.

		•	Cada datagrama recibido se imprime en pantalla como una nueva noticia.

**Publicador (publisher_udp.c)**: El publicador simula al periodista deportivo que reporta los eventos del partido.
     
		•	Crea un socket UDP y configura la dirección destino (el broker).
 
		•	En un bucle infinito, lee texto desde la entrada estándar (teclado) usando SYS_read.

		•	Cada mensaje se envía como datagrama UDP al broker usando SYS_sendto.

		•	No requiere conexión previa ni confirmación: simplemente “lanza” cada datagrama.  

# Explicación del sistema TCP

Este proyecto implementa un sistema Publish/Subscribe sobre TCP en C, utilizando sockets y manejo de hilos con pthread. El objetivo fue construir un broker que recibe mensajes de múltiples publishers y los reenvía a todos los subscribers suscritos al mismo tópico, garantizando entrega confiable gracias a TCP.

- **Broker (broker_tcp.c)** 
  - Escucha en dos puertos: uno para publishers y otro para subscribers.
    
  - Mantiene una lista enlazada de tópicos y suscriptores.
    
  - Cada conexión se maneja en un hilo independiente.
    
  - Reenvía mensajes recibidos de publishers a todos los subscribers suscritos al tópico correspondiente.  

- **Publisher (publisher_tcp.c)**  
  - Se conecta al puerto de publicación del broker.
    
  - Envía un número configurable de mensajes en un tópico.
    
  - Permite definir un intervalo opcional entre mensajes.  

- **Subscriber (subscriber_tcp.c)**  
  - Se conecta al puerto de subscripción del broker.
    
  - Envía un comando `SUBSCRIBE <topic>` para registrarse.
    
  - Recibe y muestra en consola todos los mensajes publicados en ese tópico.  

## Compilación

Compilar cada componente con gcc:

bash
gcc -o broker_tcp broker_tcp.c -lpthread
gcc -o publisher_tcp publisher_tcp.c
gcc -o subscriber_tcp subscriber_tcp.c

## Comandos
./broker_tcp <pub_port> <sub_port>

# Ejemplo:
./broker_tcp 5000 5001
./subscriber_tcp <broker_ip> <sub_port> <topic>

# Ejemplo:
./subscriber_tcp 127.0.0.1 5001 MatchA
./subscriber_tcp 127.0.0.1 5001 MatchB
./publisher_tcp <broker_ip> <pub_port> <topic> <num_msgs> [interval_ms]

# Ejemplo:
./publisher_tcp 127.0.0.1 5000 MatchA 5000 1
./publisher_tcp 127.0.0.1 5000 MatchB 5000 1

# Creación y captura del pcap
sudo tcpdump -i lo port 5000 or port 5001 -w pubsub_tcp.pcap
wireshark pubsub_tcp.pcap
