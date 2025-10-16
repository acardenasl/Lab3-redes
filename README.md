# Laboratorio Tres
Andrés Cárdenas Layton 202122083  
David Felipe Velasquez Parra 202321492  
Maria Paula Ospina Plazas 202123208  


# Libreria para UDP

En este proyecto (archivos broker_udp.c, publisher_udp.c y subscriber_udp.c) solo se emplea la librería estándar <stdio.h>, la cual pertenece al estándar ANSI C y proporciona funciones para la entrada y salida estándar (E/S), como printf(), putchar() o fgets().
Su uso para este laboratorio se limita a mostrar mensajes informativos en la terminal, ya que todas las operaciones de red (creación de sockets, envío y recepción de datagramas) se implementan directamente mediante llamadas al sistema (syscall), sin incluir otras librerías como <sys/socket.h> o <arpa/inet.h>.

Es importante mencionar que se utilizo la funcion printf(), por lo cual se realiza la siguiente documentacion:

- Función printf(): Es una de las más utilizadas del encabezado <stdio.h> y permite imprimir texto formateado en la salida estándar (por defecto, la consola).

	- Su prototipo es el siguiente: int printf(const char *formato, ***);
	
	- Parámetros:
	
		- formato: cadena de texto con posibles especificadores (%d, %s, %lu, etc.).
	
	    - *** : lista de valores que reemplazan los especificadores dentro de la cadena.
	
	- Funcionamiento interno:
	
		- printf() analiza la cadena de formato.
		- Sustituye los valores de los argumentos variables.
		- Envía el texto resultante al file descriptor 1 (stdout) mediante una llamada interna a write() del kernel.
	
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

- Finalización del proceso: El programa se ejecuta en un bucle infinito, gestionando continuamente la comunicación entre publicadores y suscriptores.
En caso de finalización manual, el socket se cierra con SYS_close, liberando los recursos del sistema.

- Funciones principales

	main(): Controla todo el flujo del programa: inicializa el socket UDP, gestiona las estructuras de red (sockaddr_in), recibe datagramas, identifica comandos de suscripción y reenvía mensajes a los suscriptores.
	
	htons(): Implementa manualmente la conversión de orden de bytes de host a red (big-endian), necesaria para definir correctamente el puerto de escucha del broker.
	
	syscall(): Utilizada en lugar de funciones estándar de biblioteca (como socket(), bind(), sendto(), etc.), permite realizar llamadas directas al sistema Linux, mostrando un enfoque de bajo nivel para la comunicación UDP.

**Suscriptor (subscriber_udp.c)**: El suscriptor representa a un usuario que desea recibir actualizaciones en tiempo real.
   
		•	Crea un socket UDP y define la dirección del broker (127.0.0.1:8080).

		•	Envía una solicitud de suscripción ("SUBSCRIBE") al broker.

		•	Luego, permanece escuchando indefinidamente con SYS_recvfrom.

		•	Cada datagrama recibido se imprime en pantalla como una nueva noticia.

- Finalización del proceso: Aunque el programa está diseñado para ejecutarse indefinidamente, si se interrumpe manualmente, el socket se cierra mediante SYS_close, liberando los recursos del sistema.

- Funciones principales:

	main(): Controla todo el flujo de ejecución: crea el socket, define la estructura de dirección, envía la suscripción inicial y gestiona el ciclo continuo de recepción de mensajes.
	
	htons(): Realiza la conversión de orden de bytes de host a red (big-endian) para el puerto, garantizando compatibilidad con el formato estándar de comunicación en redes TCP/IP.
	
	syscall(): Permite realizar llamadas directas al sistema operativo Linux, reemplazando funciones de biblioteca como socket(), sendto() o recvfrom(), para lograr una implementación de bajo nivel del protocolo UDP.

**Publicador (publisher_udp.c)**: El publicador simula al periodista deportivo que reporta los eventos del partido.
     
		•	Crea un socket UDP y configura la dirección destino (el broker).
 
		•	En un bucle infinito, lee texto desde la entrada estándar (teclado) usando SYS_read.

		•	Cada mensaje se envía como datagrama UDP al broker usando SYS_sendto.

		•	No requiere conexión previa ni confirmación: simplemente “lanza” cada datagrama.  

- Finalización del proceso: Una vez completado el envío de todos los mensajes, el programa muestra un mensaje resumen indicando la cantidad total transmitida y cierra el socket mediante SYS_close, liberando los recursos del sistema.

- Funciones principales

	main(): Contiene toda la lógica del programa: creación del socket, configuración de la dirección de destino, generación de mensajes, envío secuencial y reporte del progreso del envío.
	
	htons(): Convierte el número de puerto del formato de host al formato de red (host-to-network short), garantizando compatibilidad con las estructuras de red en sistemas basados en TCP/IP.
	
	syscall(): Se utiliza para realizar directamente las llamadas al sistema operativo (SYS_socket, SYS_sendto, SYS_close), reemplazando funciones de biblioteca y ofreciendo un control de bajo nivel sobre las operaciones de red.

# Explicación del sistema TCP

Este proyecto implementa un sistema Publish/Subscribe sobre TCP en C, utilizando sockets y manejo de hilos con pthread. El objetivo fue construir un broker que recibe mensajes de múltiples publishers y los reenvía a todos los subscribers suscritos al mismo tópico, garantizando entrega confiable gracias a TCP.

**Broker (broker_tcp.c)**: El broker recibe dos puertos como parámetros, uno para publicadores y otro para suscriptores. En cada puerto, el programa crea un socket TCP de escucha y lanza un hilo de aceptación que atiende las nuevas conexiones entrantes. Cada cliente (publicador o suscriptor) se maneja en un hilo independiente, lo que permite la comunicación concurrente.

- Estructura de datos: El sistema organiza los datos mediante listas enlazadas. Cada tema (Topic) contiene su nombre y una lista de suscriptores (Subscriber) asociados. La lista global de temas está protegida por un mutex (topics_lock) para evitar condiciones de carrera cuando múltiples hilos modifican las suscripciones o envían mensajes simultáneamente.

- Concurrencia: Cada conexión entrante (publicador o suscriptor) se atiende en un hilo separado creado con pthread_create(). El acceso concurrente a la lista global de temas y suscriptores está sincronizado mediante bloqueos mutex, garantizando la integridad de los datos durante altas cargas o desconexiones.

- Funciones principales: 

	create_and_bind(): crea y configura un socket TCP en el puerto especificado.
	
	acceptor_loop(): acepta clientes y lanza un hilo por conexión.
	
	publisher_handler(): procesa mensajes PUBLISH de los publicadores.
	
	subscriber_handler(): procesa comandos SUBSCRIBE y gestiona desconexiones.
	
	add_subscriber() / remove_subscriber_from_all(): añaden o eliminan suscriptores de los temas.
	
	broadcast_to_topic(): reenvía un mensaje a todos los suscriptores de un tema.

**Publisher (publisher_tcp.c)**: El publicador se ejecuta desde la línea de comandos con los parámetros, dirección IP del broker, puerto de publicación, nombre del tema, número de mensajes y, opcionalmente, el intervalo entre mensajes en milisegundos. Una vez conectado al broker, envía los mensajes usando el formato textual:

```PUBLISH <topic> <mensaje>```

Cada mensaje se construye dinámicamente y contiene un identificador incremental junto con el PID del proceso que lo envía.

- Proceso de conexión: El programa crea un socket TCP cliente mediante socket(AF_INET, SOCK_STREAM, 0) y establece la conexión con el broker usando connect(). Si la conexión falla, el programa muestra un mensaje de error y finaliza. En caso de éxito, el publicador informa por consola la conexión y el tema al que enviará los mensajes.

- Envío de mensajes: Dentro de un bucle, el publicador genera los mensajes usando snprintf() y los envía al broker mediante la función auxiliar send_all(), la cual garantiza que todo el contenido del mensaje sea transmitido, incluso si send() devuelve envíos parciales. Entre cada envío, el programa pausa la ejecución con usleep(interval * 1000) para mantener el intervalo especificado entre mensajes.

- Funciones principales:

	main(): gestiona los argumentos de entrada, establece la conexión TCP y envía los mensajes al broker.
	
	send_all(): asegura la transmisión completa de un buffer por el socket, repitiendo las llamadas a send() hasta completar el envío o encontrar un error.

- Cierre de conexión: Una vez enviados todos los mensajes, o si ocurre un error, el socket se cierra con close(sock) liberando los recursos asociados. 

**Subscriber (subscriber_tcp.c)**: El suscriptor valida los parámetros de entrada (broker_ip, sub_port, topic) y crea un socket TCP configurado con la dirección IP y puerto del broker. Mediante connect() establece la conexión con el servidor, permitiendo el intercambio bidireccional de datos. Si la conexión falla, el proceso termina mostrando un mensaje de error.

- Suscripción al tópico: Una vez conectado, el programa construye un mensaje de suscripción con el formato SUBSCRIBE <topic>\n.
Este mensaje se envía al broker mediante la función send_all(), asegurando que todos los bytes del comando sean transmitidos correctamente, incluso si el envío se realiza en fragmentos.

- Recepción de mensajes: Luego de suscribirse, el programa entra en un bucle infinito donde escucha continuamente mensajes del broker.
Utiliza la función read_line() para recibir datos carácter a carácter hasta detectar un salto de línea (\n), marcando el final del mensaje.
Cada mensaje recibido se muestra en pantalla con el formato:
[SUBSCRIBER] Received: <mensaje>.

- Finalización del proceso: Cuando el broker cierra la conexión o ocurre un error en la recepción, el bucle termina. El programa muestra el mensaje [SUBSCRIBER] Disconnected from broker, cierra el socket con close() y libera los recursos.

- Funciones principales: 

	main(): Gestiona el flujo principal del programa. Se encarga de procesar los argumentos, crear y conectar el socket, enviar la suscripción y manejar el ciclo de recepción de mensajes.
	
	send_all(): Envía un buffer completo de datos por el socket, garantizando la transmisión total incluso cuando send() devuelve menos bytes de los esperados.
	
	read_line(): Lee datos del socket hasta encontrar un salto de línea. Permite procesar mensajes completos enviados por el broker, uno por uno.

# Compilación
Cada componente del sistema debe compilarse de forma independiente utilizando el compilador gcc. Los siguientes comandos permiten generar los ejecutables correspondientes a cada módulo del sistema:

```bash
gcc -o broker_tcp broker_tcp.c -lpthread
gcc -o publisher_tcp publisher_tcp.c
gcc -o subscriber_tcp subscriber_tcp.c
```

## Comandos
Se ejemplifica con el protocolo TCP, aunque son los mismos para UDP
- **./broker_tcp <pub_port> <sub_port>**: Se ejecuta primero, porque debe estar escuchando antes que los clientes se conecten
  
```./broker_tcp 5000 5001```

- **./subscriber_tcp <broker_ip> <sub_port> <topic>**: Cada suscriptor se conecta al broker y se suscribe a un tema (topic) específico para recibir mensajes relacionados con ese tema.
  
```./subscriber_tcp 127.0.0.1 5001 MatchA```  
```./subscriber_tcp 127.0.0.1 5001 MatchB```  

- **./publisher_tcp <broker_ip> <pub_port> <topic> <num_msgs> [interval_ms]**: Los publicadores envían mensajes al broker indicando el tema y la cantidad de mensajes a transmitir.

```./publisher_tcp 127.0.0.1 5000 MatchA 5000 1```  
```./publisher_tcp 127.0.0.1 5000 MatchB 5000 1```  

## Creación y captura del pcap
Para analizar el intercambio de mensajes entre publicadores, broker y suscriptores, se puede capturar el tráfico TCP con tcpdump y visualizarlo en Wireshark.

```sudo tcpdump -i lo port 5000 or port 5001 -w pubsub_tcp.pcap```  
```wireshark pubsub_tcp.pcap```
