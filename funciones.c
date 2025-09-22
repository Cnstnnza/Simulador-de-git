#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
    #include <direct.h>
    #include <io.h>
    #include <windows.h>
    #define crear_directorio(nombre) _mkdir(nombre)
#else
    #include <sys/stat.h>
    #include <unistd.h>
    #define crear_directorio(nombre) mkdir(nombre, 0777)
#endif

// Colores para terminal
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"

#include "funciones.h"

typedef struct {
    char nombre[100];
    int preparado;
} Archivo;

typedef struct {
    int id;
    char autor[100];
    char mensaje[256];
    char hora[30];
} Commit;

Archivo archivos[100];
int archivo_count = 0;
int commit_count = 0;

void help() {
    printf("Comandos disponibles:\n");
    printf("  git init                -> inicializa repositorio\n");
    printf("  git delete              -> elimina repositorio\n");
    printf("  git create [file]       -> crea archivo real\n");
    printf("  git add [file]          -> agrega archivo al staging\n");
    printf("  git commit -m [msg]     -> crea commit de archivos en staging\n");
    printf("  git log                 -> muestra historial de commits\n");
    printf("  git checkout [id]       -> restaura archivos de commit\n");
    printf("  git help                -> muestra esta ayuda\n");
    printf("  exit                    -> salir del programa\n");
}

void init_repo() {
    if (crear_directorio("repo") == 0) {
        crear_directorio("repo/commits");
        printf("Repositorio creado exitosamente.\n");
    } else {
        printf("El repositorio ya existe o hubo un error.\n");
    }
}

void delete_repo() {
#ifdef _WIN32
    system("rmdir /S /Q repo");
#else
    system("rm -rf repo");
#endif
    printf("Repositorio eliminado.\n");
}

void create_archivo(char *nombre) {
    FILE *f = fopen(nombre, "w");
    if (!f) {
        printf("No se pudo crear el archivo: %s\n", nombre);
        return;
    }

    fprintf(f, "");
    fclose(f);

    strncpy(archivos[archivo_count].nombre, nombre, sizeof(archivos[archivo_count].nombre) - 1);
    archivos[archivo_count].preparado = 0;
    archivo_count++;

    printf("Archivo '%s' creado.\n", nombre);
}

void add_archivo(char *nombre) {
    int encontrado = 0;

    for (int i = 0; i < archivo_count; i++) {
        if (strcmp(archivos[i].nombre, nombre) == 0) {
            archivos[i].preparado = 1;
            printf("Archivo '%s' agregado al staging.\n", nombre);
            encontrado = 1;
            break;
        }
    }

    if (!encontrado) {
        printf("Archivo '%s' no existe.\n", nombre);
    }
}

void commit(char *mensaje) {
    int archivos_listos = 0;

    for (int i = 0; i < archivo_count; i++) {
        if (archivos[i].preparado) {
            archivos_listos = 1;
        }
    }

    if (!archivos_listos) {
        printf(RED "Error: No hay archivos preparados para commit.\n" RESET);
        return;
    }

    if (commit_count >= 100) {
        printf("Límite de commits alcanzado.\n");
        return;
    }

    char autor[100];
    printf("Autor: ");
    fgets(autor, sizeof(autor), stdin);
    autor[strcspn(autor, "\n")] = 0;

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char hora[30];
    snprintf(hora, sizeof(hora), "%02d/%02d/%04d %02d:%02d:%02d",
             tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900,
             tm.tm_hour, tm.tm_min, tm.tm_sec);

    char path[200];
    snprintf(path, sizeof(path), "repo/commits/%d", commit_count + 1);
    crear_directorio(path);

    for (int i = 0; i < archivo_count; i++) {
        if (archivos[i].preparado) {
            char dest[300];
            snprintf(dest, sizeof(dest), "%s/%s", path, archivos[i].nombre);

            FILE *src = fopen(archivos[i].nombre, "r");
            FILE *dst = fopen(dest, "w");
            if (!src || !dst) continue;

            char c;
            while ((c = fgetc(src)) != EOF) {
                fputc(c, dst);
            }

            fclose(src);
            fclose(dst);
            archivos[i].preparado = 0;
        }
    }

    FILE *log = fopen("repo/commits.log", "a");
    fprintf(log, "Commit %d | Autor: %s | Hora: %s | Mensaje: %s\n",
            commit_count + 1, autor, hora, mensaje);
    fclose(log);

    commit_count++;
    printf("Commit %d creado.\n", commit_count);
}

void log_commits() {
    FILE *log = fopen("repo/commits.log", "r");
    if (!log) {
        printf("No hay commits o no existe el repositorio.\n");
        return;
    }

    char linea[512];
    while (fgets(linea, sizeof(linea), log)) {
        printf("%s", linea);
    }

    fclose(log);
}

void checkout_commit(int commit_id) {
    if (commit_id <= 0 || commit_id > commit_count) {
        printf(RED "Error: Commit %d no existe.\n" RESET, commit_id);
        return;
    }

    char path[200];
    snprintf(path, sizeof(path), "repo/commits/%d", commit_id);

#ifdef _WIN32
    DWORD ftyp = GetFileAttributes(path);
    if (ftyp == INVALID_FILE_ATTRIBUTES) {
        printf(RED "Error: Commit %d no existe.\n" RESET, commit_id);
        return;
    }
#else
    if (access(path, 0) != 0) {
        printf(RED "Error: Commit %d no existe.\n" RESET, commit_id);
        return;
    }
#endif

    printf("Restaurando archivos de commit %d...\n", commit_id);

    char archivo_origen[300], archivo_dest[300];
    for (int i = 0; i < archivo_count; i++) {
        snprintf(archivo_origen, sizeof(archivo_origen), "%s/%s", path, archivos[i].nombre);
        snprintf(archivo_dest, sizeof(archivo_dest), "%s", archivos[i].nombre);

        FILE *src = fopen(archivo_origen, "r");
        if (!src) continue;

        FILE *dst = fopen(archivo_dest, "w");
        if (!dst) {
            fclose(src);
            continue;
        }

        char c;
        while ((c = fgetc(src)) != EOF) {
            fputc(c, dst);
        }

        fclose(src);
        fclose(dst);
    }

    printf("Checkout completado. Ahora los archivos reflejan el commit %d.\n", commit_id);
}
