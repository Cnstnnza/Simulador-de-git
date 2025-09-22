#ifndef FUNCIONES_H
#define FUNCIONES_H

void help();
void init_repo();
void delete_repo();
void commit(char *mensaje);
void log_commits();
void create_archivo(char *nombre);
void add_archivo(char *nombre);
void checkout_commit(int commit_id);

#endif

