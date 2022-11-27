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

struct tm* tm;
char datestring[256];

struct list {
};

void print_data(int level, struct dirent* dp, char* out) {
    struct stat statbuf;
    struct passwd* pwd;
    struct group* grp;
    char str[256];
    // revisa la integridad del archivo y obtiene los stats

    // tamanno
    stat(dp->d_name, &statbuf);
    sprintf(str, "%ld", statbuf.st_size);

    // fecha modificacion
    tm = localtime(&statbuf.st_mtime);
    strftime(datestring, sizeof(datestring), nl_langinfo(D_T_FMT), tm);

    strcat(out, "/");
    strcat(out, dp->d_name);
    strcat(out, " ");
    // printf("/%s",dp->d_name);
    strcat(out, str);
    strcat(out, "\n");
    // printf(" %s  \n",str);
    //  nombre
}

void magic(int level, char* pathy, char* grafo) {
    char buf[PATH_MAX + 1];
    DIR* dir;
    struct dirent* dp;

    dir = opendir(pathy);  // abrimos la carpeta

    while ((dp = readdir(dir)) != NULL) {
        char out[2048] = "";
        if (strcmp(dp->d_name, "..") != 0 && strcmp(dp->d_name, ".") != 0 && dp->d_name[0] != '.' && strcmp(dp->d_name, ".config") != 0)  // para ignorar folder actual y superior
        {
            print_data(level, dp, grafo);
            if (dp->d_type == DT_DIR) {
                strcat(buf, pathy);
                strcat(buf, "/");
                strcat(buf, dp->d_name);

                // printf(buf);
                strcat(grafo, buf);

                magic(level + 1, buf, grafo);
                memset(buf, 0, sizeof(buf));
            }
        }
    }
}

void saveData(char* route, char* out) {
    FILE* filePointer;
    char buffer[255]; /* not ISO 90 compatible */
    filePointer = fopen(route, "w");
    if (filePointer == NULL) {
        printf("File error to write .config");
    } else {
        fputs(out, filePointer);

        fclose(filePointer);
    }
}

void readFile(char* route, char* out) {
    FILE* filePointer;
    int bufferLength = 255;
    char buffer[bufferLength]; /* not ISO 90 compatible */
    filePointer = fopen(route, "r");
    if (filePointer == NULL) {
        strcat(out, "File is not available \n");
    } else {
        while (fgets(buffer, bufferLength, filePointer)) {
            // bufferFile = (char *) realloc(bufferFile, bufferLength);
            strcat(out, buffer);
        }
        out[strlen(out)] = '\0';

        fclose(filePointer);
    }
}

void findNode(char* string, char* list) {
    char* token;
    char* rest = list;
    int find = 0;
    while ((token = strtok_r(rest, "\n", &rest))) {
        if (strcmp(token, string) == 0) {
            find = 1;
            break;
        }
    }
    if (find == 0)
        printf("%s\n", string);
}

void compareByNOdes(char* data1, char* data2) {
    // printf("Datos Existentes \n%s \n",data1);
    // printf("Datos Leidos \n%s ",data2);
    char buff[255];
    char buff1[2048];
    // loop through the string to extract all other tokens
    char* token;
    char* rest = data1;
    while ((token = strtok_r(rest, "\n", &rest))) {
        strcpy(buff, token);
        strcpy(buff1, data2);
        findNode(buff, buff1);
    }
}

void compare() {
    char* route = ".config";
    char grafo[2048];
    memset(grafo, 0, sizeof(grafo));
    FILE* filePointer;
    filePointer = fopen(route, "r");
    if (filePointer == NULL) {
        magic(0, ".", grafo);
        saveData(route, grafo);
    } else {
        char dataRead[2048];
        memset(dataRead, 0, sizeof(dataRead));
        magic(0, ".", grafo);

        readFile(".config", dataRead);
        printf("\nArchivos Eliminados\n");
        compareByNOdes(dataRead, grafo);

        memset(dataRead, 0, sizeof(dataRead));
        memset(grafo, 0, sizeof(grafo));
        readFile(".config", dataRead);
        magic(0, ".", grafo);
        saveData(route, grafo);
        printf("\nArchivos Nuevos\n");
        compareByNOdes(grafo, dataRead);

  
    }
}
/*
int main(int argc, char *argv[]) {

    // revisamos si hay mas de 1 argumentos, el argumento 0 es el nombre del programa
    if (argc >= 2) {
        compare();

    } else
        printf("Se espera que haya al menos un argumento.\n");

    exit(0);
    }
*/