#ifndef FUNCIONES_H   // protección para no incluirlo dos veces
#define FUNCIONES_H

void help();
void init_repo(char *nombre);
void delete_repo(char *nombre);
void commit(char *mensaje);
void log_commits();

#endif