#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "funciones.h"

#define MAX_COMMITS 100

// Estructura de commit
typedef struct {
    int id;
    char mensaje[256];
} Commit;

// Variables globales
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
            init();
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
        printf("Error: límite de commits alcanzado.\n");
        return;
    }

    commits[commit_count].id = commit_count + 1;
    strncpy(commits[commit_count].mensaje, mensaje, sizeof(commits[commit_count].mensaje) - 1);
    commits[commit_count].mensaje[sizeof(commits[commit_count].mensaje) - 1] = '\0';

    printf("[Commit %d] %s\n", commits[commit_count].id, commits[commit_count].mensaje);

    commit_count++;
}

void log_commits() {
    if (commit_count == 0) {
        printf("No hay commits.\n");
        return;
    }

    printf("Historial de commits:\n");
    for (int i = commit_count - 1; i >= 0; i--) {
        printf("Commit %d: %s\n", commits[i].id, commits[i].mensaje);
    }
}

