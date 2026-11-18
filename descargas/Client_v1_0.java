/*
 * Anastacio Favián Flores Lira
 * Sistemas Distribuidos
 * Actividad: Proyecto Integrador - (Cliente)
 * Propósito: Desarrollar un cliente en Java que permita ingresar matrices y vectores, enviar los datos al servidor mediante sockets TCP y mostrar los resultados de las operaciones.
 */

import java.io.*;
import java.net.Socket;
import java.util.Random;
import java.util.Scanner;

public class Client_v1_0 {

    // Configuración de red del servidor remoto
    private static final String HOST = "localhost";
    private static final int PORT = 5001;
    
    // Generador de números aleatorios para autocompletar arreglos/matrices
    private static final Random random = new Random();

    /*
     * Gestiona la conexión del Socket y el ciclo de vida de la aplicación.
     */
    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);

        System.out.println("Intentando conectar al servidor " + HOST + ":" + PORT + "...");

        try (Socket socket = new Socket(HOST, PORT);
             DataOutputStream out = new DataOutputStream(new BufferedOutputStream(socket.getOutputStream()));
             DataInputStream in = new DataInputStream(new BufferedInputStream(socket.getInputStream()))) {

            System.out.println("¡Conexión establecida exitosamente!");

            boolean running = true;
            while (running) {
                printMenu();
                // Solicita una opción entre 1 y 3 validando que sea válida
                int choice = readInt(sc, "Seleccione una opción: ", 1, 3);

                switch (choice) {
                    case 1:
                        handleAddition(sc, in, out);
                        break;
                    case 2:
                        handleGreatest(sc, in, out);
                        break;
                    case 3:
                        System.out.println("Cerrando conexión y saliendo...");
                        running = false;
                        break;
                }
            }

        } catch (EOFException e) {
            System.err.println("\n[Error] El servidor cerró la conexión inesperadamente.");
        } catch (IOException e) {
            System.err.println("\n[Error de E/S] No se pudo comunicar con el servidor: " + e.getMessage());
        } finally {
            sc.close();
        }
    }

    private static void printMenu() {
        System.out.println("\n================ MENU ================");
        System.out.println("1. Suma de Matrices (N x N)");
        System.out.println("2. Obtener Elemento Mayor de un Vector");
        System.out.println("3. Salir");
        System.out.println("======================================");
    }

    /*
     * Suma de Matrices.
     * Solicita tamaño, genera o lee matrices A y B, las envía al servidor y lee la matriz resultante R.
     */
    private static void handleAddition(Scanner sc, DataInputStream in, DataOutputStream out) throws IOException {
        int n = readInt(sc, "Ingrese el dimensión N de las matrices (1 - 1000): ", 1, 1000);
        boolean autoFill = readBoolean(sc, "¿Desea generar valores aleatorios para las matrices? (s/n): ");

        float[][] a = new float[n][n];
        float[][] b = new float[n][n];

        if (autoFill) {
            fillRandom(a);
            fillRandom(b);
            System.out.println("\nMatriz A generada:");
            print(a);
            System.out.println("\nMatriz B generada:");
            print(b);
        } else {
            System.out.println("\n--- Ingreso de Matriz A ---");
            readMatrixInput(sc, a);
            System.out.println("\n--- Ingreso de Matriz B ---");
            readMatrixInput(sc, b);
        }

        // Protocolo de transmisión:

        // 1. Enviar el código de la operación (1) y la dimensión N
        out.writeInt(1);
        out.writeInt(n);

        // 2. Transmitir los elementos de la Matriz A y Matriz B serializados
        sendMatrix(out, a);
        sendMatrix(out, b);
        out.flush(); // Fuerza el envío inmediato del búfer a través de la red

        // 3. Recibir la matriz resultado desde el servidor
        float[][] result = new float[n][n];
        readMatrix(in, result);

        System.out.println("\n>>> Resultado de la Suma (enviado por el Servidor C):");
        print(result);
    }

    /*
     * Búsqueda del Elemento Mayor.
     * Solicita el tamaño del vector M, transmite los elementos y recibe el resultado escalar.
     */
    private static void handleGreatest(Scanner sc, DataInputStream in, DataOutputStream out) throws IOException {
        int m = readInt(sc, "Ingrese el tamaño M del vector (1 - 100000): ", 1, 100000);
        boolean autoFill = readBoolean(sc, "¿Desea generar valores aleatorios para el vector? (s/n): ");

        float[] v = new float[m];

        if (autoFill) {
            fillRandom(v);
        } else {
            System.out.println("\n--- Ingreso de Vector ---");
            for (int i = 0; i < m; i++) {
                v[i] = readFloat(sc, "v[" + i + "] = ");
            }
        }

        System.out.println("\nVector a procesar:");
        print(v);

        // Protocolo de transmisión:
        // 1. Enviar el código de la operación (2) y el tamaño M
        out.writeInt(2);
        out.writeInt(m);

        // 2. Transmitir el vector flotante
        for (float val : v) {
            out.writeFloat(val);
        }
        out.flush();

        // 3. Recibir el número flotante máximo
        float maxVal = in.readFloat();
        System.out.println("\n>>> Valor máximo calculado por el servidor: " + maxVal);
    }

    // ===============================================
    // MÉTODOS DE TRANSMISIÓN DE RED (SERIALIZACIÓN)
    // ===============================================

    /*
     * Serializa una matriz bidimensional enviándola elemento a elemento (Row-Major Order) al flujo de red.
     */
    private static void sendMatrix(DataOutputStream out, float[][] matrix) throws IOException {
        for (float[] row : matrix) {
            for (float val : row) {
                out.writeFloat(val);
            }
        }
    }

    /*
     * Lee desde el flujo de red una secuencia de números flotantes y la reconstruye en una matriz bidimensional.
     */
    private static void readMatrix(DataInputStream in, float[][] matrix) throws IOException {
        for (int i = 0; i < matrix.length; i++) {
            for (int j = 0; j < matrix[i].length; j++) {
                matrix[i][j] = in.readFloat();
            }
        }
    }

    // ===============================================
    // MÉTODOS AUXILIARES DE LECTURA SEGURA DE CONSOLA
    // ===============================================

    /**
     * Solicita un entero por consola y valida que esté dentro del rango [min, max].
     */
    private static int readInt(Scanner sc, String prompt, int min, int max) {
        while (true) {
            System.out.print(prompt);
            try {
                int val = Integer.parseInt(sc.nextLine().trim());
                if (val >= min && val <= max) return val;
                System.out.println("Por favor ingrese un número entre " + min + " y " + max + ".");
            } catch (NumberFormatException e) {
                System.out.println("Entrada inválida. Ingrese un número entero válido.");
            }
        }
    }

    /*
     * Solicita un número flotante por consola evitando excepciones de formato.
     */
    private static float readFloat(Scanner sc, String prompt) {
        while (true) {
            System.out.print(prompt);
            try {
                return Float.parseFloat(sc.nextLine().trim());
            } catch (NumberFormatException e) {
                System.out.println("Número flotante inválido. Intente de nuevo.");
            }
        }
    }

    /*
     * Solicita una respuesta afirmativa o negativa (sí/no) al usuario.
     */
    private static boolean readBoolean(Scanner sc, String prompt) {
        while (true) {
            System.out.print(prompt);
            String input = sc.nextLine().trim().toLowerCase();
            if (input.equals("s") || input.equals("si") || input.equals("y")) return true;
            if (input.equals("n") || input.equals("no")) return false;
            System.out.println("Responda con 's' (sí) o 'n' (no).");
        }
    }

    /*
     * Permite llenar una matriz elemento por elemento de forma manual.
     */
    private static void readMatrixInput(Scanner sc, float[][] m) {
        for (int i = 0; i < m.length; i++) {
            for (int j = 0; j < m[i].length; j++) {
                m[i][j] = readFloat(sc, "[" + i + "][" + j + "] = ");
            }
        }
    }

    // ==========================================
    // MÉTODOS DE UTILIDAD Y FORMATO DE IMPRESIÓN
    // ==========================================

    /*
     * Llena el vector con números aleatorios entre -100 y 100, redondeados a dos decimales..
     */
    private static void fillRandom(float[] v) {
        for (int i = 0; i < v.length; i++) {
            v[i] = Math.round((random.nextFloat() * 200.0f - 100.0f) * 100.0f) / 100.0f;
        }
    }

    /*
     * Llena cada fila de una matriz bidimensional con valores flotantes aleatorios.
     */
    private static void fillRandom(float[][] m) {
        for (float[] row : m) {
            fillRandom(row);
        }
    }

    /*
     * Formatea e imprime un vector flotante unidimensional. Muestra los valores de un vector en pantalla o un aviso si supera los 20 elementos.
     */
    public static void print(float[] v) {
        if (v.length > 20) {
            System.out.println("[ Vector de " + v.length + " elementos (demasiado grande para mostrar) ]");
            return;
        }
        System.out.print("[ ");
        for (int i = 0; i < v.length; i++) {
            System.out.printf("%.4f", v[i]);
            if (i < v.length - 1) System.out.print(", ");
        }
        System.out.println(" ]");
    }

    /*
     * Muestra los valores de una matriz en pantalla o un aviso si supera los 15x15 elementos.
     */
    public static void print(float[][] m) {
        if (m.length > 15) {
            System.out.println("[ Matriz de " + m.length + "x" + m.length + " (demasiado grande para mostrar) ]");
            return;
        }
        for (float[] row : m) {
            System.out.print("[ ");
            for (int j = 0; j < row.length; j++) {
                System.out.printf("%8.4f", row[j]);
                if (j < row.length - 1) System.out.print(" ");
            }
            System.out.println(" ]");
        }
    }
}