#include <dirent.h>
#include <grp.h>
#include <langinfo.h>
#include <limits.h>  //For PATH_MAX
#include <locale.h>
#include <pwd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

struct tm *tm;
char datestring[256];

/* NOTA se metio esta función, en el ejemplo de la pagina se llama
en los ejemplos para obtener los permisos de un archivo,
pero en mi caso me daba error aunque tuviera las mismas librerias en el codigo,
por lo que busqué la funcion y la copie directamente
*/
char const *sperm(__mode_t mode) {
    static char local_buff[16] = {0};
    int i = 0;
    // user permissions
    if ((mode & S_IRUSR) == S_IRUSR)
        local_buff[i] = 'r';
    else
        local_buff[i] = '-';
    i++;
    if ((mode & S_IWUSR) == S_IWUSR)
        local_buff[i] = 'w';
    else
        local_buff[i] = '-';
    i++;
    if ((mode & S_IXUSR) == S_IXUSR)
        local_buff[i] = 'x';
    else
        local_buff[i] = '-';
    i++;
    // group permissions
    if ((mode & S_IRGRP) == S_IRGRP)
        local_buff[i] = 'r';
    else
        local_buff[i] = '-';
    i++;
    if ((mode & S_IWGRP) == S_IWGRP)
        local_buff[i] = 'w';
    else
        local_buff[i] = '-';
    i++;
    if ((mode & S_IXGRP) == S_IXGRP)
        local_buff[i] = 'x';
    else
        local_buff[i] = '-';
    i++;
    // other permissions
    if ((mode & S_IROTH) == S_IROTH)
        local_buff[i] = 'r';
    else
        local_buff[i] = '-';
    i++;
    if ((mode & S_IWOTH) == S_IWOTH)
        local_buff[i] = 'w';
    else
        local_buff[i] = '-';
    i++;
    if ((mode & S_IXOTH) == S_IXOTH)
        local_buff[i] = 'x';
    else
        local_buff[i] = '-';
    return local_buff;
}

char *magic(int level, char *pathy) {
    char buf[PATH_MAX + 1];
    DIR *dir;
    struct dirent *dp;
    struct stat statbuf;
    struct passwd *pwd;
    struct group *grp;

    char *server_reply = malloc(sizeof(char) * 2000);  // aqui metemos el string

    char space[] = "    ";
    char branch[] = "│   ";
    char tee[] = "├── ";
    char last[] = "└── ";

    dir = opendir(pathy);  // abrimos la carpeta

    while ((dp = readdir(dir)) != NULL) {
        if (strcmp(dp->d_name, "..") != 0 && strcmp(dp->d_name, ".") != 0 && dp->d_name[0] != '.')  // para ignorar folder actual y superior
        {
            // revisa la integridad del archivo y obtiene los stats
            stat(dp->d_name, &statbuf);

            if (level == 0)
                strcat(server_reply, tee);

            for (int i = 0; i < level; i++)  // esto se encarga de manejar la indentacion segun el nivel que tengamos
                strcat(server_reply, space);

            if (level != 0)
                strcat(server_reply, tee);

            // colores
            if (dp->d_type == DT_DIR)
                strcat(server_reply, "\033[0;34m\e[1m");

            strcat(server_reply, dp->d_name);
            strcat(server_reply, "\033[0m");
            strcat(server_reply, "\n");
        }
    }
    return server_reply;
}

char *print_directorio() {
    // revisamos si hay mas de 1 argumentos, el argumento 0 es el nombre del programa
    char *server_reply = malloc(sizeof(char) * 2000);
    strcpy(server_reply, "\033[1;33mArchivos en el directorio Actual: \033[0m\n");
    strcat(server_reply, magic(0, "."));
    return server_reply;
}
