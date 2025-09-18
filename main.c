#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "funciones.h"

#define MAX_COMMITS 100
#define MAX_ARCHIVOS 100


// Estructura de commit
typedef struct {
    int id;
    char autor[100];
    char mensaje[256];
    char hora[30];
} Commit;

// estructura del archivo
typedef struct {
    char nombre[100];
} Archivo;

// Variables globales
Archivo archivos[MAX_ARCHIVOS];
int archivo_count = 0;
Commit commits[MAX_COMMITS];
int commit_count = 0;

int main() {
    char comando[512];

    while (1) {
        printf("ugit> ");
        if (fgets(comando, sizeof(comando), stdin) == NULL) {
            break;
        }
        comando[strcspn(comando, "\n")] = 0; // quitar salto de línea

        if (strcmp(comando, "exit") == 0) {
            break;
        }
        else if (strcmp(comando, "git init") == 0) {
            init_repo();
        }
        else if (strcmp(comando, "git help") == 0) {
            help();
        }
        else if (strncmp(comando, "git commit -m \"", 15) == 0) {
            char *mensaje = comando + 15;
            char *fin = strrchr(mensaje, '"');
            if (fin) *fin = '\0'; // cerrar la cadena del mensaje
            commit(mensaje);
        }
        else if (strcmp(comando, "git log") == 0) {
            log_commits();
        }
        else if (strncmp(comando, "git create ", 11) == 0) {
            char *nombre = comando + 11;
            create_archivo(nombre); 
        }      
        else {
            printf("Comando no reconocido: %s\n", comando);
        }
    }

    return 0;
}



void init_repo() {
    printf("Repositorio inicializado.\n");
}

void delete_repo() {
    printf("Repositorio eliminado.\n");
}

void help() {
    printf("Comandos disponibles:\n");
    printf("  git init               -> Inicializa un repositorio\n");
    printf("  git add \"archivo\"    -> Añade un archivo\n");
    printf("  git commit -m \"msg\"   -> Crea un commit con mensaje\n");
    printf("  git log                -> Muestra historial de commits\n");
    printf("  git help               -> Muestra este mensaje\n");
    printf("  git delete             -> Elimina el repositorio\n");
    printf("  exit                   -> Salir\n");
}

void commit(char *mensaje) {
    if (commit_count >= MAX_COMMITS) {
        printf("Error:Limites de Commits alcanzado\n");
        return;
    }

    commits[commit_count].id = commit_count + 1;

    printf("Autor:");
    fgets(commits[commit_count].autor, sizeof(commits[commit_count].autor), stdin);
    commits[commit_count].autor[strcspn(commits[commit_count].autor, "\n")] = '\0';

    strncpy(commits[commit_count].mensaje, mensaje, sizeof(commits[commit_count].mensaje) - 1);
    commits[commit_count].mensaje[sizeof(commits[commit_count].mensaje) - 1] = '\0';

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    snprintf(commits[commit_count].hora, sizeof(commits[commit_count].hora),"%02d/%02d/%04d %02d:%02d:%02d",tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900,tm.tm_hour, tm.tm_min, tm.tm_sec);
    printf("[Commit %d] Autor: %s | Mensaje: %s | Hora: %s\n",
           commits[commit_count].id,
           commits[commit_count].autor,
           commits[commit_count].mensaje,
           commits[commit_count].hora);
    commit_count++;
}

void log_commits() {
    if (commit_count == 0) {
        printf("No hay commits.\n");
        return;
    }

    printf("Historial de commits:\n");
    for (int i = commit_count - 1; i >= 0; i--) {
        printf("Commit %d | Autor: %s | Mensaje: %s | Hora: %s\n",
               commits[i].id,
               commits[i].autor,
               commits[i].mensaje,
               commits[i].hora);
    }
}

int buscar_archivo(char *nombre) {
    for (int i = 0; i < archivo_count; i++) {
        if (strcmp(archivos[i].nombre, nombre) == 0) {
            return i; 
        }
    }
    return -1;
}

void create_archivo(char *nombre) {
    if (archivo_count >= MAX_ARCHIVOS) {
        printf("Error: límite de archivos alcanzado.\n");
        return;
    }
    if (buscar_archivo(nombre) != -1) {
        printf("Error: el archivo '%s' ya existe.\n", nombre);
        return;
    }

    strncpy(archivos[archivo_count].nombre, nombre, sizeof(archivos[archivo_count].nombre) - 1);
    archivos[archivo_count].nombre[sizeof(archivos[archivo_count].nombre) - 1] = '\0';
    archivo_count++;

    printf("Archivo '%s' creado.\n", nombre);
}

