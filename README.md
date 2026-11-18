# Middleware · Java / TCP / C

Presentación web individual del proyecto de Sistemas Distribuidos.

## Publicación en GitHub Pages

1. Descomprime este paquete.
2. Sube `index.html`, `styles.css`, `script.js`, `README.md` y la carpeta `descargas/` a la raíz del repositorio.
3. En GitHub, abre Settings > Pages.
4. Selecciona Deploy from a branch, rama `main` y carpeta `/ (root)`.
5. Guarda y espera la publicación.

## Descargas incluidas

La sección Descargas ofrece dos implementaciones completas:

- **Proyecto A — Servidor secuencial sin fork():** `descargas/Proyecto_A_Secuencial_Sin_Fork.zip`
- **Proyecto B — Servidor concurrente con fork():** `descargas/Proyecto_B_Concurrente_Con_Fork.zip`

Cada ZIP incluye `Cliente.java`, `Servidor.c` y `LEEME.txt`. También se puede descargar cada archivo individual desde su respectiva carpeta.

## Probar los proyectos

GitHub Pages es un servicio de alojamiento estático. Permite consultar la presentación y descargar los archivos, pero no ejecuta Java o C ni establece sockets TCP desde el navegador.

Para probar cualquiera de las versiones:
1. Descarga el ZIP correspondiente y descomprímelo.
2. Abre una terminal en la carpeta extraída.
3. Compila y ejecuta el servidor C:
   `clang Servidor.c -o servidor`
   `./servidor`
4. Abre una segunda terminal en la misma carpeta, compila y ejecuta el cliente:
   `javac Cliente.java`
   `java Cliente`

Consulta el `LEEME.txt` de cada proyecto para sus detalles.

## Estructura de descargas

- `descargas/Proyecto_A_Secuencial_Sin_Fork.zip`
- `descargas/Proyecto_B_Concurrente_Con_Fork.zip`
- `descargas/A_Secuencial_sin_fork/` — archivos individuales de A
- `descargas/B_Concurrente_con_fork/` — archivos individuales de B
