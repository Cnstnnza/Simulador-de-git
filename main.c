/**
 * @file main.c
 * @brief Archivo principal de UGit Simulator.
 *
 * Este archivo contiene el punto de entrada principal de
 * la aplicacion simulada de Git llamada UGit.
 * Maneja el interprete de comandos y llama a funciones
 * definidas en funciones.c
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "funciones.h"

/* Colores para salida en consola */
#define RED     "\x1b[31m" /**< Texto rojo */
#define GREEN   "\x1b[32m" /**< Texto verde */
#define YELLOW  "\x1b[33m" /**< Texto amarillo */
#define CYAN    "\x1b[36m" /**< Texto cyan */
#define RESET   "\x1b[0m"  /**< Resetear color */

/**
 * @brief Verifica si una cadena contiene solo numeros.
 * @param s Cadena de texto a verificar.
 * @return 1 si la cadena contiene solo numeros, 0 en caso contrario.
 */
static int is_number(const char *s) {
    if (!s || *s == '\0') return 0;
    for (; *s; s++) if (*s < '0' || *s > '9') return 0;
    return 1;
}

/**
 * @brief Funcion principal (entry point) del simulador UGit.
 *
 * Esta funcion implementa un bucle interactivo que recibe comandos del usuario,
 * los interpreta y llama a las funciones correspondientes para ejecutar comandos
 * como init, commit, add, push, pull, checkout, branch, entre otros.
 *
 * @return Codigo de salida (0 si todo fue correcto).
 */
int main() {
    char comando[512];

    /* Mensaje de bienvenida */
    printf(GREEN "==============================\n");
    printf("   Bienvenido a UGit Simulator\n");
    printf("==============================\n" RESET);
    printf(CYAN "Escribe 'git help' para ver los comandos disponibles\n\n" RESET);

    /* Cargar estado previo del repositorio */
    cargar_estado();

    /* Bucle principal del interprete */
    while (1) {
        printf(YELLOW "ugit> " RESET);

        if (fgets(comando, sizeof(comando), stdin) == NULL) break;

        comando[strcspn(comando, "\n")] = 0; /**< Eliminar salto de linea */

        if (strcmp(comando, "exit") == 0) {
            guardar_estado();
            printf(GREEN "¡Gracias por usar UGit! Hasta luego.\n" RESET);
            break;

        } else if (strncmp(comando, "git init", 8) == 0) {
            if (strlen(comando) > 9) {
                char *nombre = comando + 9; /**< nombre especificado */
                init_repo(nombre);
            } else {
                init_repo(NULL); /**< nombre por defecto */
            }

        } else if (strcmp(comando, "git help") == 0) {
            help();

        } else if (strncmp(comando, "git commit -m \"", 15) == 0) {
            char *mensaje = comando + 15;
            char *fin = strrchr(mensaje, '"');
            if (!fin || mensaje == fin) {
                printf(RED "Error: Debe ingresar un mensaje entre comillas.\n" RESET);
                continue;
            }
            *fin = '\0';
            commit(mensaje);

        } else if (strcmp(comando, "git log") == 0) {
            log_commits();

        } else if (strncmp(comando, "git create ", 11) == 0) {
            char *nombre = comando + 11;
            if (strpbrk(nombre, "<>:\"/\\|?*")) {
                printf(RED "Error: Nombre de archivo invalido.\n" RESET);
                continue;
            }
            create_archivo(nombre);

        } else if (strncmp(comando, "git add ", 8) == 0) {
            char *arg = comando + 8;
            add_archivo(arg);

        } else if (strcmp(comando, "git status") == 0) {
            status();

        } else if (strcmp(comando, "git add .") == 0) {
            add_todos();

        } else if (strcmp(comando, "git push") == 0) {
            push_repo();

        } else if (strcmp(comando, "git pull") == 0) {
            pull_repo();

        } else if (strncmp(comando, "git branch", 10) == 0) {
            if (strlen(comando) > 11) {
                char *name = comando + 11;
                branch_cmd(name);
            } else {
                branch_cmd(NULL);
            }

        } else if (strncmp(comando, "git checkout ", 13) == 0) {
            char *arg = comando + 13;
            if (is_number(arg)) {
                int id = atoi(arg);
                checkout_commit(id);
            } else {
                checkout_branch_name(arg);
            }

        } else {
            printf(RED "Comando no reconocido: %s\n" RESET, comando);
            printf(CYAN "Escribe 'git help' para ver los comandos disponibles.\n" RESET);
        }
    }

    return 0;
}
