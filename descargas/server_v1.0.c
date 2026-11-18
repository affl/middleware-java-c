/*
 * INTEGRANTES:
 * Daniel Alejandro Guerrero Lopez
 * Jose Manuel Nava Cervantes
 * Carlos Manuel Villaseñor Vazquez
 * Anastacio Favián Flores Lira
 * Sistemas Distribuidos
 * Actividad: Proyecto Integrador - (Servidor)
 * Propósito: Desarrollar un servidor en C que reciba solicitudes mediante sockets TCP, realice operaciones de suma de matrices y obtención del valor mayor de un vector, y devuelva los resultados.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <stdint.h>
#include <signal.h>

#define PORT 5001
#define BACKLOG 10

/*
 * Se ejecuta automáticamente cuando un proceso hijo termina para evitar acumulación de procesos "zombie" en la tabla de procesos del sistema operativo.
 */
 
void sigchld_handler(int s) {
    (void)s; // Evita la advertencia de parámetro no usado
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

/*
 * Garantiza la lectura completa de 'len' bytes del socket TCP.
 * Resuelve la fragmentación de paquetes propia de TCP asegurando que todo el buffer sea llenado.
 */
int read_fully(int sock, void *buf, size_t len) {
    size_t total = 0;
    char *p = (char *)buf;
    while (total < len) {
        ssize_t n = recv(sock, p + total, len - total, 0);
        if (n <= 0) return -1; /* Error en el socket o cliente desconectado */
        total += n;
    }
    return 0;
}

/*
 * Garantiza el envío completo de 'len' bytes a través del socket TCP.
 */
int write_fully(int sock, const void *buf, size_t len) {
    size_t total = 0;
    const char *p = (const char *)buf;
    while (total < len) {
        ssize_t n = send(sock, p + total, len - total, 0);
        if (n <= 0) return -1; /* Error al escribir */
        total += n;
    }
    return 0;
}

/*
 * Lee un entero entero de 32 bits desde la red y lo convierte de Big-Endian a Host-Byte-Order.
 */
int read_int(int sock, int32_t *value) {
    uint32_t net;
    if (read_fully(sock, &net, sizeof(net)) < 0) return -1;
    *value = (int32_t)ntohl(net);
    return 0;
}

/*
 * Escribe un entero de punto flotante de 32 bits a la red en formato Big-Endian (estándar Java/Red).
 */
int write_float(int sock, float value) {
    uint32_t bits;
    memcpy(&bits, &value, sizeof(bits));
    uint32_t net_bits = htonl(bits);
    return write_fully(sock, &net_bits, sizeof(net_bits));
}

/*
 * Lee un bloque masivo de flotantes desde el socket y revierte el orden de bytes de Big-Endian a Little-Endian.
 */
int read_float_array(int sock, float *data, size_t count) {
    size_t bytes_needed = count * sizeof(float);
    if (read_fully(sock, data, bytes_needed) < 0) return -1;

    // Convertir de formato de Red (Big-Endian) al endianness local de la CPU
    for (size_t i = 0; i < count; i++) {
        uint32_t bits;
        memcpy(&bits, &data[i], sizeof(uint32_t));
        bits = ntohl(bits);
        memcpy(&data[i], &bits, sizeof(float));
    }
    return 0;
}

/*
 * Convierte un arreglo entero de flotantes a formato Big-Endian y lo envía de una sola vez por la red.
 */
int write_float_array(int sock, const float *data, size_t count) {
    float *buffer = (float *)malloc(count * sizeof(float));
    if (!buffer) return -1;

    // Convertir el arreglo local a Big-Endian en el buffer temporal
    for (size_t i = 0; i < count; i++) {
        uint32_t bits;
        memcpy(&bits, &data[i], sizeof(float));
        uint32_t net_bits = htonl(bits);
        memcpy(&buffer[i], &net_bits, sizeof(uint32_t));
    }

    int res = write_fully(sock, buffer, count * sizeof(float));
    free(buffer);
    return res;
}

/*
 * Operación 1: Suma elemento a elemento de dos matrices cuadradas aplanadas de N x N.
 */
void addition(const float *a, const float *b, float *r, int n) {
    int total = n * n;
    for (int i = 0; i < total; i++) {
        r[i] = a[i] + b[i];
    }
}

/*
 * Operación 2: Búsqueda lineal para encontrar el número con valor máximo en un vector de M flotantes.
 */
float greatest(const float *v, int m) {
    float max = v[0];
    for (int i = 1; i < m; i++) {
        if (v[i] > max) max = v[i];
    }
    return max;
}

/*
 * Gestiona el protocolo de comunicación y ciclo de vida de peticiones con un cliente en específico.
 */
void handle_client(int client_sock) {
    int32_t op;
    while (1) {
        // Lee el código de la operación que solicita el cliente
        if (read_int(client_sock, &op) < 0) break;

        if (op == 1) { /* Suma de Matrices */
            int32_t n;
            // Lee y valida la dimensión de la matriz
            if (read_int(client_sock, &n) < 0 || n <= 0 || n > 1000) break;

            size_t elements = (size_t)n * n;
            // Reserva de memoria contigua para las matrices A, B y R
            float *a = (float *)malloc(elements * sizeof(float));
            float *b = (float *)malloc(elements * sizeof(float));
            float *r = (float *)malloc(elements * sizeof(float));

            if (!a || !b || !r) {
                free(a); free(b); free(r);
                break;
            }

            // Recibir arreglos serializados A y B
            if (read_float_array(client_sock, a, elements) < 0 ||
                read_float_array(client_sock, b, elements) < 0) {
                free(a); free(b); free(r);
                break;
            }

            // Realizar la suma de las matrices
            addition(a, b, r, n);

            // Transmitir la matriz resultado R de regreso al cliente
            if (write_float_array(client_sock, r, elements) < 0) {
                free(a); free(b); free(r);
                break;
            }

            // Liberación limpia de recursos en el heap
            free(a); free(b); free(r);

        } else if (op == 2) { /* Elemento Máximo */
            int32_t m;
            // Lee y valida la longitud del vector
            if (read_int(client_sock, &m) < 0 || m <= 0 || m > 100000) break;

            float *v = (float *)malloc(m * sizeof(float));
            if (!v) break;

            // Recibir los elementos del vector
            if (read_float_array(client_sock, v, m) < 0) {
                free(v);
                break;
            }

            // Obtener el valor máximo y liberar el arreglo
            float res = greatest(v, m);
            free(v);

            // Responder con la respuesta escalar de 32 bits
            if (write_float(client_sock, res) < 0) break;

        } else {
            // Opción inválida o solicitud de desconexión
            break;
        }
    }
    close(client_sock); // Cierra el socket del cliente al salir del bucle
}

/*
 * Punto de entrada del Servidor: configura los sockets, bind, listen y despacha clientes mediante fork().
 */
int main(void) {
    int server_fd, client_sock;
    struct sockaddr_in addr;
    int opt = 1;

    /* 1. Configuración de manejo de señales para prevenir procesos Zombies */
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa, NULL);

    /* 2. Crear Socket TCP */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    /* 3. Permitir reutilización rápida del puerto (Evita errores 'Address already in use') */
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    /* 4. Asignar Dirección IP y Puerto al Socket */
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind error");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    /* 5. Escuchar por conexiones entrantes */
    if (listen(server_fd, BACKLOG) < 0) {
        perror("listen error");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Servidor optimizado escuchando en el puerto %d...\n", PORT);

    /* 6. Bucle de aceptación de clientes concurrente mediante fork() */
    while (1) {
        client_sock = accept(server_fd, NULL, NULL);
        if (client_sock < 0) continue;

        /* Crear proceso hijo dedicado exclusivamente para el cliente aceptado */
        pid_t pid = fork();
        if (pid == 0) {
            // --- CÓDIGO DEL PROCESO HIJO ---
            close(server_fd); /* El hijo no administra el socket listener del servidor */
            handle_client(client_sock);
            exit(0); /* Termina el proceso hijo al desconectarse el cliente */
        } else if (pid > 0) {
            // --- CÓDIGO DEL PROCESO PADRE ---
            close(client_sock); /* El padre libera su descriptor de este cliente y vuelve a escuchar */
        }
    }

    close(server_fd);
    return 0;
}