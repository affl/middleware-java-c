/*
 * Sistemas Distribuidos - Proyecto integrador
 * Cliente Java
 *
 * Captura matrices y vectores, los envia al servidor por TCP
 * y muestra los resultados recibidos.
 */
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.util.Scanner;

public class Cliente {
    private static final String SERVIDOR = "127.0.0.1";
    private static final int PUERTO = 5001;

    // Imprime todos los elementos de un vector.
    public static void print(float v[]) {
        for (int i = 0; i < v.length; i++) {
            System.out.print(v[i] + " ");
        }
        System.out.println();
    }

    // Imprime una matriz fila por fila.
    public static void print(float m[][]) {
        for (int i = 0; i < m.length; i++) {
            for (int j = 0; j < m[i].length; j++) {
                System.out.print(m[i][j] + "\t");
            }
            System.out.println();
        }
    }

    public static void main(String[] args) {
        Scanner teclado = new Scanner(System.in);

        try {
            // Se conecta al servidor y prepara los flujos de entrada y salida.
            Socket socket = new Socket(SERVIDOR, PUERTO);
            DataInputStream entrada =
                new DataInputStream(socket.getInputStream());
            DataOutputStream salida =
                new DataOutputStream(socket.getOutputStream());

            int opcion = 0;

            // El menu se repite hasta seleccionar Exit.
            while (opcion != 3) {
                System.out.println("\n1. Addition");
                System.out.println("2. Greatest");
                System.out.println("3. Exit");
                System.out.print("Selecciona una opcion: ");
                opcion = teclado.nextInt();

                if (opcion == 1) {
                    System.out.print("Dimension de las matrices (N x N): ");
                    int n = teclado.nextInt();

                    if (n < 1 || n > 50) {
                        System.out.println("Dimension no valida (1 a 50).");
                        continue;
                    }

                    float a[][] = new float[n][n];
                    float b[][] = new float[n][n];
                    float r[][] = new float[n][n];

                    System.out.println("Valores de la matriz A:");
                    for (int i = 0; i < n; i++) {
                        for (int j = 0; j < n; j++) {
                            System.out.print("A[" + i + "][" + j + "]: ");
                            a[i][j] = teclado.nextFloat();
                        }
                    }

                    System.out.println("Valores de la matriz B:");
                    for (int i = 0; i < n; i++) {
                        for (int j = 0; j < n; j++) {
                            System.out.print("B[" + i + "][" + j + "]: ");
                            b[i][j] = teclado.nextFloat();
                        }
                    }

                    // 1 identifica Addition; luego se envia el tamaño.
                    salida.writeInt(1);
                    salida.writeInt(n);

                    // Los valores se envian como float binarios, no como texto.
                    for (int i = 0; i < n; i++) {
                        for (int j = 0; j < n; j++) {
                            salida.writeFloat(a[i][j]);
                        }
                    }
                    for (int i = 0; i < n; i++) {
                        for (int j = 0; j < n; j++) {
                            salida.writeFloat(b[i][j]);
                        }
                    }
                    salida.flush();

                    // Se recibe la matriz que calculo el servidor.
                    for (int i = 0; i < n; i++) {
                        for (int j = 0; j < n; j++) {
                            r[i][j] = entrada.readFloat();
                        }
                    }

                    System.out.println("Resultado de Addition:");
                    print(r);

                } else if (opcion == 2) {
                    System.out.print("Cantidad de elementos del vector: ");
                    int n = teclado.nextInt();

                    if (n < 1 || n > 10000) {
                        System.out.println("Tamaño no valido (1 a 10000).");
                        continue;
                    }

                    float v[] = new float[n];

                    for (int i = 0; i < n; i++) {
                        System.out.print("v[" + i + "]: ");
                        v[i] = teclado.nextFloat();
                    }

                    // 2 identifica Greatest.
                    salida.writeInt(2);
                    salida.writeInt(n);
                    for (int i = 0; i < n; i++) {
                        salida.writeFloat(v[i]);
                    }
                    salida.flush();

                    // El servidor devuelve el valor mayor.
                    System.out.println("Vector enviado:");
                    print(v);
                    System.out.println("Resultado de Greatest: "
                                       + entrada.readFloat());

                } else if (opcion == 3) {
                    // 3 indica al servidor que el cliente termina.
                    salida.writeInt(3);
                    salida.flush();
                    System.out.println("Fin de la conexion.");

                } else {
                    System.out.println("Opcion no valida.");
                }
            }

            entrada.close();
            salida.close();
            socket.close();

        } catch (IOException e) {
            System.out.println("Error de comunicacion: " + e.getMessage());
        } finally {
            teclado.close();
        }
    }
}
