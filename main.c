// simulador git
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

void init_repo(char *nombre);
void delete_repo(char *nombre);

int main(){
    char carpeta[40];

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