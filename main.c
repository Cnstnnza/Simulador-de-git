// simulador git
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "funciones.h"


int main(){
    char carpeta[40];
    char comando[10];

    printf("Bienvenido a Git Simulator\n");
    printf("Si necesitas ayuda, escribe 'help'\n");

    while(1)
    {   
        printf("git-simulator> ");
        fgets(comando, sizeof(comando), stdin);
        comando[strcspn(comando, "\n")] = 0;

        if(strcmp(comando, "exit") == 0)
        {
            printf("Saliendo del simulador de Git. ¡Hasta luego!\n");
            break;
        }
        else if(strcmp(comando, "help") == 0)
        {
            help();
        }
        else if(strcmp(comando, "git init") == 0)
        {
            printf("Ingrese el nombre del repositorio a crear: ");
            scanf("%s", carpeta);
            init_repo(carpeta);
        }
        else if(strcmp(comando, "git delete") == 0)
        {
            printf("Ingrese el nombre del repositorio a eliminar: ");
            scanf("%s", carpeta);
            printf("¿Está seguro de que desea eliminar el repositorio '%s'? (S/N): ", carpeta);
            fgets(comando, sizeof(comando), stdin);
            comando[strcspn(comando, "\n")] = 0;
            if(strcmp(comando, "S") == 0)
            {
                delete_repo(carpeta);
            }
            else
            {
                printf("No se eliminó el repositorio '%s'.\n", carpeta);
            }
        }
        else 
        {
            printf("Comando no reconocido. Escriba 'help' para ver los comandos disponibles.\n");
        }
    }
    return 0;
}

void init_repo(char *nombre)
{
    if (mkdir(nombre, 0700) == 0) {
        printf("Repositorio '%s' creado correctamente.\n", nombre);
    } else {
        perror("Error al crear el repositorio");
    }
}

void delete_repo(char *nombre)
{
    if (rmdir(nombre) == 0) {
        printf("Repositorio '%s' eliminado correctamente.\n", nombre);
    } else {
        perror("Error al eliminar el repositorio");
    }
}

void help(){
    printf("COMANDOS DISPONIBLES\n");
    printf("git init\t\t\tInicializa un repositorio\n");
    printf("git pull\t\t\tConsigue los cambios del repositorio remoto\n");
    printf("git push\t\t\tEnvía los cambios al repositorio remoto\n");
    printf("git add\t\t\tAñade un archivo al repositorio\n");
    printf("git commit\t\t\tGuarda los cambios en el repositorio\n");
}