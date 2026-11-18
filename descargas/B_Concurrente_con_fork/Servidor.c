/*
 * Sistemas Distribuidos - Proyecto integrador
 * Servidor C
 *
 * Recibe solicitudes TCP, ejecuta Addition o Greatest
 * y devuelve los resultados al cliente Java.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>

#define PUERTO 5001

/*
 * TCP es un flujo de bytes: recv puede entregar solo una parte.
 * Este ciclo continua hasta recibir todos los bytes solicitados.
 */
int recibirTodo(int cliente, void *datos, int cantidad) {
    int recibidos = 0;
    while (recibidos < cantidad) {
        int n = recv(cliente, (char *)datos + recibidos,
                     cantidad - recibidos, 0);
        if (n <= 0) return 0;
        recibidos += n;
    }
    return 1;
}

/* Repite send hasta enviar todos los bytes del dato. */
int enviarTodo(int cliente, const void *datos, int cantidad) {
    int enviados = 0;
    while (enviados < cantidad) {
        int n = send(cliente, (const char *)datos + enviados,
                     cantidad - enviados, 0);
        if (n <= 0) return 0;
        enviados += n;
    }
    return 1;
}

/* Convierte el entero recibido desde el orden de red. */
int recibirEntero(int cliente, int *valor) {
    uint32_t dato;
    if (!recibirTodo(cliente, &dato, sizeof(dato))) return 0;
    *valor = (int)ntohl(dato);
    return 1;
}

/* Recibe los cuatro bytes de un float enviado por Java. */
int recibirFloat(int cliente, float *valor) {
    uint32_t dato;
    if (!recibirTodo(cliente, &dato, sizeof(dato))) return 0;
    dato = ntohl(dato);
    memcpy(valor, &dato, sizeof(float));
    return 1;
}

/* Envia un float en el orden de bytes que utiliza Java. */
int enviarFloat(int cliente, float valor) {
    uint32_t dato;
    memcpy(&dato, &valor, sizeof(float));
    dato = htonl(dato);
    return enviarTodo(cliente, &dato, sizeof(dato));
}

/* Suma las matrices elemento por elemento.
   Los elementos llegan seguidos, por filas, en arreglos lineales. */
void addition(int n, float a[n][n], float b[n][n], float r[n][n]) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            r[i][j] = a[i][j] + b[i][j];
        }
    }
}

/* Recorre el vector y devuelve el numero mayor. */
float greatest(float v[], int n) {
    float mayor = v[0];
    for (int i = 1; i < n; i++) {
        if (v[i] > mayor) mayor = v[i];
    }
    return mayor;
}

/* Atiende las operaciones de un cliente hasta que este elige Exit. */
void atenderCliente(int cliente) {
    int operacion, n;

    while (recibirEntero(cliente, &operacion)) {
        if (operacion == 3) {
            printf("El cliente selecciono Exit.\n");
            fflush(stdout);
            break;
        }
        if (operacion != 1 && operacion != 2) break;
        if (!recibirEntero(cliente, &n)) break;

        if (operacion == 1) {
            if (n < 1 || n > 50) break;
            printf("Solicitud Addition: matrices de %d x %d\n", n, n);
            fflush(stdout);

            float a[n][n], b[n][n], r[n][n];
            int correcto = 1;

            /* Recibe A y B en el mismo orden por filas que envio Java. */
            for (int i = 0; i < n && correcto; i++) {
                for (int j = 0; j < n; j++) {
                    if (!recibirFloat(cliente, &a[i][j])) {
                        correcto = 0;
                        break;
                    }
                }
            }

            for (int i = 0; i < n && correcto; i++) {
                for (int j = 0; j < n; j++) {
                    if (!recibirFloat(cliente, &b[i][j])) {
                        correcto = 0;
                        break;
                    }
                }
            }

            if (correcto) {
                addition(n, a, b, r);

                /* Envia el resultado por filas, como una secuencia de float. */
                for (int i = 0; i < n && correcto; i++) {
                    for (int j = 0; j < n; j++) {
                        if (!enviarFloat(cliente, r[i][j])) {
                            correcto = 0;
                            break;
                        }
                    }
                }
            }

            if (!correcto) break;

        } else if (operacion == 2) {
            if (n < 1 || n > 10000) break;
            printf("Solicitud Greatest: vector de %d elementos\n", n);
            fflush(stdout);

            float v[10000];
            int correcto = 1;

            for (int i = 0; i < n; i++) {
                if (!recibirFloat(cliente, &v[i])) {
                    correcto = 0;
                    break;
                }
            }

            if (correcto) {
                float mayor = greatest(v, n);
                correcto = enviarFloat(cliente, mayor);
            }

            if (!correcto) break;
        }
    }

    close(cliente);
}

int main(void) {
    int servidor, cliente;
    struct sockaddr_in direccion;

    /* Los procesos hijos terminados se liberan automaticamente. */
    signal(SIGCHLD, SIG_IGN);

    servidor = socket(AF_INET, SOCK_STREAM, 0);
    if (servidor < 0) {
        perror("Error al crear socket");
        return 1;
    }

    memset(&direccion, 0, sizeof(direccion));
    direccion.sin_family = AF_INET;
    direccion.sin_addr.s_addr = INADDR_ANY;
    direccion.sin_port = htons(PUERTO);

    if (bind(servidor, (struct sockaddr *)&direccion,
             sizeof(direccion)) < 0) {
        perror("Error en bind");
        close(servidor);
        return 1;
    }

    if (listen(servidor, 5) < 0) {
        perror("Error en listen");
        close(servidor);
        return 1;
    }

    printf("Servidor con fork en el puerto %d...\n", PUERTO);

    /* El servidor principal acepta conexiones continuamente. */
    while (1) {
        cliente = accept(servidor, NULL, NULL);
        if (cliente < 0) {
            perror("Error en accept");
            continue;
        }

        pid_t proceso = fork();

        if (proceso < 0) {
            perror("Error en fork");
            close(cliente);

        } else if (proceso == 0) {
            /* El hijo atiende al cliente; no necesita el socket principal. */
            close(servidor);
            atenderCliente(cliente);
            exit(0);

        } else {
            /* El padre conserva el servidor y cierra su copia del cliente. */
            close(cliente);
        }
    }

    close(servidor);
    return 0;
}
