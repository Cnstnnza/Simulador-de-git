#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "funciones.h"

// Colores para terminal 
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"

int main() {
    char comando[512];

    printf(GREEN "==============================\n");
    printf("   Bienvenido a UGit Simulator\n");
    printf("==============================\n" RESET);
    printf(CYAN "Escribe 'git help' para ver los comandos disponibles\n\n" RESET);

    while (1) {
        printf(YELLOW "ugit> " RESET);

        if (fgets(comando, sizeof(comando), stdin) == NULL)
            break;

        comando[strcspn(comando, "\n")] = 0;

        if (strcmp(comando, "exit") == 0) {
            printf(GREEN "¡Gracias por usar UGit! Hasta luego.\n" RESET);
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

            if (!fin || mensaje == fin) {
                printf(RED "Error: Debe ingresar un mensaje entre comillas.\n" RESET);
                continue;
            }

            *fin = '\0';
            commit(mensaje);
        } 
        else if (strcmp(comando, "git log") == 0) {
            log_commits();
        } 
        else if (strncmp(comando, "git create ", 11) == 0) {
            char *nombre = comando + 11;

            // Validar caracteres no permitidos
            if (strpbrk(nombre, "<>:\"/\\|?*")) {
                printf(RED "Error: Nombre de archivo inválido.\n" RESET);
                continue;
            }

            create_archivo(nombre);
        } 
        else if (strncmp(comando, "git add ", 8) == 0) {
            char *nombre = comando + 8;
            add_archivo(nombre);
        } 
        else if (strncmp(comando, "git checkout ", 13) == 0) {
            int id = atoi(comando + 13);

            if (id <= 0) {
                printf(RED "Error: ID de commit inválido.\n" RESET);
                continue;
            }

            checkout_commit(id);
        } 
        else {
            printf(RED "Comando no reconocido: %s\n" RESET, comando);
            printf(CYAN "Escribe 'git help' para ver los comandos disponibles.\n" RESET);
        }
    }

    return 0;
}
