#ifndef FUNCIONES_H
#define FUNCIONES_H

#define MAX_COMMITS 100
#define MAX_ARCHIVOS 200
#define MAX_FILES_PER_COMMIT 100
#define MAX_BRANCHES 10
#define BRANCH_NAME_LEN 50

/* Prototipos */
void help();
void init_repo(char *nombre);
void delete_repo();

void create_archivo(char *nombre);
void add_archivo(char *nombre);
void add_todos();

void commit(char *mensaje);
void log_commits();
void status();

void push_repo();
void pull_repo();

void branch_cmd(char *arg); 
void checkout_commit(int commit_id);
void checkout_branch_name(char *name);

void cargar_estado(void);
void guardar_estado(void);

#endif
