#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

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

#include "funciones.h"

/* Estructuras */
typedef struct {
    char nombre[100];
    int preparado; /* 0 = no staged, 1 = staged */
} Archivo;

typedef struct {
    int id;
    char hash[32];
    char autor[100];
    char mensaje[256];
    char hora[30];
    int file_count;
    char archivos_incluidos[MAX_FILES_PER_COMMIT][100];
} Commit;

typedef struct {
    char name[BRANCH_NAME_LEN];
    int head_commit; /* id del commit al que apunta la rama (0 = ninguno) */
} Branch;

/* Globals */
static Archivo archivos[MAX_ARCHIVOS];
static int archivo_count = 0;

static Commit commits[MAX_COMMITS];
static int commit_count = 0;

static Branch branches[MAX_BRANCHES];
static int branch_count = 0;
static int current_branch_index = 0; /* por defecto la rama 0 (main) */

static void generate_hash(const char *a, const char *b, char *out, size_t outlen) {
    uint64_t h = 1469598103934665603ULL;
    const unsigned char *p;
    for (p = (const unsigned char*)a; *p; ++p) { h ^= *p; h *= 1099511628211ULL; }
    for (p = (const unsigned char*)b; *p; ++p) { h ^= *p; h *= 1099511628211ULL; }
    snprintf(out, outlen, "%016llx", (unsigned long long)h);
}

/* búsqueda lineal de archivo (devuelve índice o -1) */
static int buscar_archivo_linear(const char *nombre) {
    for (int i = 0; i < archivo_count; i++) {
        if (strcmp(archivos[i].nombre, nombre) == 0) return i;
    }
    return -1;
}

/* búsqueda binaria de commit por id (commits ordenados por id asc) */
static int buscar_commit_por_id(int id) {
    int l = 0, r = commit_count - 1;
    while (l <= r) {
        int m = (l + r) / 2;
        if (commits[m].id == id) return m;
        if (commits[m].id < id) l = m + 1;
        else r = m - 1;
    }
    return -1;
}

/* --- Comandos y funcionalidades --- */

void help() {
    printf("Comandos disponibles:\n");
    printf("  git init [nombre]      -> inicializa repositorio (por defecto 'repo')\n");
    printf("  git delete             -> elimina repositorio\n");
    printf("  git create [file]      -> crea archivo en working directory\n");
    printf("  git add [file]         -> añade archivo al staging\n");
    printf("  git add .              -> añade todos los archivos al staging\n");
    printf("  git commit -m \"msg\"   -> crea commit con archivos staged (pedirá autor)\n");
    printf("  git log                -> muestra historial de commits\n");
    printf("  git status             -> muestra archivos y staging\n");
    printf("  git push               -> simula subir commits al remoto\n");
    printf("  git pull               -> simula bajar cambios del remoto\n");
    printf("  git branch             -> lista ramas\n");
    printf("  git branch <name>      -> crea una rama\n");
    printf("  git checkout <id|name> -> checkout por id de commit o cambiar a rama\n");
    printf("  git help               -> muestra esta ayuda\n");
    printf("  exit                   -> salir\n");
}

void init_repo(char *nombre) {
    char repo_name[120];
    if (nombre == NULL || strlen(nombre) == 0) strcpy(repo_name, "repo");
    else strncpy(repo_name, nombre, sizeof(repo_name)-1), repo_name[sizeof(repo_name)-1]=0;

    if (crear_directorio(repo_name) == 0) {
        char path_commits[200];
        snprintf(path_commits, sizeof(path_commits), "%s/commits", repo_name);
        crear_directorio(path_commits);

        /* crear rama main inicial */
        branch_count = 0;
        current_branch_index = 0;
        strncpy(branches[0].name, "main", BRANCH_NAME_LEN-1);
        branches[0].head_commit = 0;
        branch_count = 1;

        /* crear archivo de log (vacío) */
        char logpath[200];
        snprintf(logpath, sizeof(logpath), "%s/commits.log", repo_name);
        FILE *f = fopen(logpath, "a");
        if (f) fclose(f);

        printf("Repositorio '%s' creado (carpeta creada).\n", repo_name);
    } else {
        printf("El repositorio ya existe o hubo un error al crear '%s'.\n", repo_name);
    }
}

void delete_repo() {
#ifdef _WIN32
    system("rmdir /S /Q repo");
#else
    system("rm -rf repo");
#endif
    printf("Repositorio eliminado (si existía).\n");
}

/* Crea archivo en working dir y lo registra en arreglo */
void create_archivo(char *nombre) {
    if (!nombre || strlen(nombre) == 0) {
        printf("Nombre de archivo inválido.\n");
        return;
    }
    if (archivo_count >= MAX_ARCHIVOS) {
        printf("Límite de archivos alcanzado.\n");
        return;
    }
    if (buscar_archivo_linear(nombre) != -1) {
        printf("Error: el archivo '%s' ya existe.\n", nombre);
        return;
    }

    FILE *f = fopen(nombre, "w");
    if (!f) {
        printf("No se pudo crear el archivo: %s\n", nombre);
        return;
    }
    /* archivo creado vacío */
    fclose(f);

    strncpy(archivos[archivo_count].nombre, nombre, sizeof(archivos[archivo_count].nombre)-1);
    archivos[archivo_count].preparado = 0;
    archivo_count++;

    printf("Archivo '%s' creado en working directory.\n", nombre);
}

/* Marca un archivo como staged */
void add_archivo(char *nombre) {
    if (!nombre || strlen(nombre) == 0) {
        printf("Nombre de archivo inválido.\n");
        return;
    }
    if (strcmp(nombre, ".") == 0) {
        add_todos();
        return;
    }
    int idx = buscar_archivo_linear(nombre);
    if (idx == -1) {
        printf("Archivo '%s' no existe.\n", nombre);
        return;
    }
    archivos[idx].preparado = 1;
    printf("Archivo '%s' agregado al staging.\n", nombre);
}

/* Añade todos los archivos al staging */
void add_todos() {
    if (archivo_count == 0) {
        printf("No hay archivos que agregar.\n");
        return;
    }
    for (int i = 0; i < archivo_count; i++) archivos[i].preparado = 1;
    printf("Todos los archivos agregados al staging.\n");
}

/* Crea commit: guarda metadata y copia archivos staged a repo/commits/<id> */
void commit(char *mensaje) {
    int any_staged = 0;
    for (int i = 0; i < archivo_count; i++) if (archivos[i].preparado) { any_staged = 1; break; }

    if (!any_staged) {
        printf("Error: no hay archivos preparados para commit.\n");
        return;
    }
    if (commit_count >= MAX_COMMITS) {
        printf("Límite de commits alcanzado.\n");
        return;
    }

    char autor[100];
    printf("Autor: ");
    if (!fgets(autor, sizeof(autor), stdin)) autor[0]=0;
    autor[strcspn(autor, "\n")] = 0;

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char hora[30];
    snprintf(hora, sizeof(hora), "%02d/%02d/%04d %02d:%02d:%02d",
             tm.tm_mday, tm.tm_mon+1, tm.tm_year+1900, tm.tm_hour, tm.tm_min, tm.tm_sec);

    /* preparar commit */
    Commit c;
    c.id = commit_count + 1;
    c.file_count = 0;
    strncpy(c.autor, autor, sizeof(c.autor)-1);
    strncpy(c.mensaje, mensaje, sizeof(c.mensaje)-1);
    strncpy(c.hora, hora, sizeof(c.hora)-1);

    char tempcat[400];
    snprintf(tempcat, sizeof(tempcat), "%s|%s|%s", autor, mensaje, hora);
    generate_hash(tempcat, "ugit", c.hash, sizeof(c.hash));

    /* crear carpeta del commit */
    char path[256];
    snprintf(path, sizeof(path), "repo/commits/%d", c.id);
    crear_directorio(path);

    /* copiar archivos staged */
    for (int i = 0; i < archivo_count; i++) {
        if (!archivos[i].preparado) continue;
        /* copiar nombre */
        strncpy(c.archivos_incluidos[c.file_count], archivos[i].nombre, sizeof(c.archivos_incluidos[0])-1);
        c.file_count++;

        /* copiar contenido real a repo/commits/<id>/<nombre> */
        char dest[400];
        snprintf(dest, sizeof(dest), "%s/%s", path, archivos[i].nombre);

        FILE *src = fopen(archivos[i].nombre, "r");
        FILE *dst = fopen(dest, "w");
        if (!src || !dst) {
            if (src) fclose(src);
            if (dst) fclose(dst);
            continue;
        }
        int ch;
        while ((ch = fgetc(src)) != EOF) fputc(ch, dst);
        fclose(src);
        fclose(dst);

        /* despejar staging del archivo */
        archivos[i].preparado = 0;
    }

    /* guardar commit en arreglo y en log persistente */
    commits[commit_count] = c;
    commit_count++;

    /* actualizar head de la rama actual */
    branches[current_branch_index].head_commit = commits[commit_count-1].id;

    /* escribir en repo/commits.log */
    FILE *log = fopen("repo/commits.log", "a");
    if (log) {
        fprintf(log, "Commit %d | Hash: %s | Autor: %s | Hora: %s | Mensaje: %s\n",
                c.id, c.hash, c.autor, c.hora, c.mensaje);
        /* incluir lista de archivos en línea (opcional) */
        if (c.file_count > 0) {
            fprintf(log, "    Archivos:");
            for (int i = 0; i < c.file_count; i++) fprintf(log, " %s", c.archivos_incluidos[i]);
            fprintf(log, "\n");
        }
        fclose(log);
    }

    printf("Commit %d creado (hash %s) con %d archivo(s).\n", c.id, c.hash, c.file_count);
}

/* Imprime commits (del más reciente al más antiguo) */
void log_commits() {
    if (commit_count == 0) {
        printf("No hay commits.\n");
        return;
    }
    for (int i = commit_count - 1; i >= 0; i--) {
        Commit *c = &commits[i];
        printf("Commit %d | Hash: %s\n  Autor: %s | Hora: %s\n  Mensaje: %s\n",
               c->id, c->hash, c->autor, c->hora, c->mensaje);
        if (c->file_count > 0) {
            printf("  Archivos:");
            for (int j = 0; j < c->file_count; j++) printf(" %s", c->archivos_incluidos[j]);
            printf("\n");
        }
        printf("\n");
    }
}

/* Mostrar estado: archivos y staging */
void status() {
    printf("=== Estado del repositorio (working dir) ===\n");
    if (archivo_count == 0) {
        printf("No hay archivos creados.\n");
        return;
    }
    for (int i = 0; i < archivo_count; i++) {
        printf("Archivo: %s [%s]\n", archivos[i].nombre, archivos[i].preparado ? "staged" : "untracked/modified");
    }
    /* rama actual */
    printf("Rama actual: %s (HEAD -> %d)\n",
           branches[current_branch_index].name,
           branches[current_branch_index].head_commit);
}

/* Simulación de push: copia commits/log a repo/remote (sistema simple) */
void push_repo() {
#ifdef _WIN32
    system("mkdir repo\\remote 2>nul");
    system("xcopy /E /I repo\\commits repo\\remote\\commits >nul");
    system("copy repo\\commits.log repo\\remote\\commits.log >nul");
#else
    system("mkdir -p repo/remote");
    system("rm -rf repo/remote/commits 2>/dev/null || true");
    system("cp -r repo/commits repo/remote/commits 2>/dev/null || true");
    system("cp repo/commits.log repo/remote/commits.log 2>/dev/null || true");
#endif
    printf("Commits y log copiados al remoto (repo/remote).\n");
}

/* Simulación de pull: si existe repo/remote, copia al repo local */
void pull_repo() {
#ifdef _WIN32
    system("if exist repo\\remote\\commits xcopy /E /I repo\\remote\\commits repo\\commits >nul");
    system("if exist repo\\remote\\commits.log copy repo\\remote\\commits.log repo\\commits.log >nul");
#else
    system("if [ -d repo/remote/commits ]; then cp -r repo/remote/commits repo/commits 2>/dev/null; fi");
    system("if [ -f repo/remote/commits.log ]; then cp repo/remote/commits.log repo/commits.log 2>/dev/null; fi");
#endif
    printf("Archivos obtenidos del remoto.\n");
}

/* Branch: lista o crea */
void branch_cmd(char *arg) {
    if (!arg || strlen(arg) == 0) {
        /* listar ramas */
        printf("Ramas:\n");
        for (int i = 0; i < branch_count; i++) {
            printf("  %s%s (HEAD -> %d)\n", (i == current_branch_index) ? "* " : "  ", branches[i].name, branches[i].head_commit);
        }
        return;
    }
    /* crear nueva rama (no cambia archivos, solo crea referencia) */
    if (branch_count >= MAX_BRANCHES) {
        printf("Límite de ramas alcanzado.\n");
        return;
    }
    /* verificar si existe */
    for (int i = 0; i < branch_count; i++) {
        if (strcmp(branches[i].name, arg) == 0) {
            printf("La rama '%s' ya existe.\n", arg);
            return;
        }
    }
    strncpy(branches[branch_count].name, arg, BRANCH_NAME_LEN-1);
    branches[branch_count].head_commit = branches[current_branch_index].head_commit; /* apuntar al mismo head */
    branch_count++;
    printf("Rama '%s' creada apuntando a commit %d.\n", arg, branches[branch_count-1].head_commit);
}

/* Checkout por id: restaura archivos del commit */
void checkout_commit(int commit_id) {
    if (commit_id <= 0 || commit_id > commit_count) {
        printf("Error: commit %d no existe.\n", commit_id);
        return;
    }
    int idx = buscar_commit_por_id(commit_id);
    if (idx == -1) {
        printf("Error interno: commit no encontrado.\n");
        return;
    }
    Commit *c = &commits[idx];

    char path[256];
    snprintf(path, sizeof(path), "repo/commits/%d", c->id);

    /* restaurar los archivos listados en el commit */
    printf("Restaurando archivos del commit %d...\n", c->id);
    for (int i = 0; i < c->file_count; i++) {
        char origen[400], destino[200];
        snprintf(origen, sizeof(origen), "%s/%s", path, c->archivos_incluidos[i]);
        snprintf(destino, sizeof(destino), "%s", c->archivos_incluidos[i]);

        FILE *src = fopen(origen, "r");
        if (!src) continue;
        FILE *dst = fopen(destino, "w");
        if (!dst) { fclose(src); continue; }

        int ch;
        while ((ch = fgetc(src)) != EOF) fputc(ch, dst);
        fclose(src); fclose(dst);
    }

    printf("Checkout completado (archivos restaurados desde commit %d).\n", c->id);
}

/* Checkout por rama (cambia HEAD a rama) y si la rama apunta a commit lo restaura */
void checkout_branch_name(char *name) {
    if (!name || strlen(name) == 0) {
        printf("Nombre de rama inválido.\n");
        return;
    }
    int found = -1;
    for (int i = 0; i < branch_count; i++) {
        if (strcmp(branches[i].name, name) == 0) { found = i; break; }
    }
    if (found == -1) {
        printf("Rama '%s' no existe.\n", name);
        return;
    }
    current_branch_index = found;
    printf("Cambiada a rama '%s'. HEAD -> %d\n", branches[found].name, branches[found].head_commit);
    if (branches[found].head_commit > 0) checkout_commit(branches[found].head_commit);
}
void guardar_estado() {
    // guardar commits
    FILE *f = fopen("repo/commits.dat", "wb");
    if (f) {
        fwrite(&commit_count, sizeof(int), 1, f);
        fwrite(commits, sizeof(Commit), commit_count, f);
        fclose(f);
    }

    // guardar ramas
    f = fopen("repo/branches.dat", "wb");
    if (f) {
        fwrite(&branch_count, sizeof(int), 1, f);
        fwrite(branches, sizeof(Branch), branch_count, f);
        fwrite(&current_branch_index, sizeof(int), 1, f);
        fclose(f);
    }
}

void cargar_estado() {
    FILE *f = fopen("repo/commits.dat", "rb");
    if (f) {
        fread(&commit_count, sizeof(int), 1, f);
        fread(commits, sizeof(Commit), commit_count, f);
        fclose(f);
    }

    f = fopen("repo/branches.dat", "rb");
    if (f) {
        fread(&branch_count, sizeof(int), 1, f);
        fread(branches, sizeof(Branch), branch_count, f);
        fread(&current_branch_index, sizeof(int), 1, f);
        fclose(f);
    }
}