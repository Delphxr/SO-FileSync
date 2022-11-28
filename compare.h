#include <dirent.h>
#include <limits.h>  //For PATH_MAX
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

struct tm* tm;
char datestring[256];

void print_data(int level, struct dirent* dp, char* out) {
    struct stat statbuf;
    struct passwd* pwd;
    struct group* grp;
    char str[256];
    char date[64];
    // revisa la integridad del archivo y obtiene los stats

    // tamanno
    stat(dp->d_name, &statbuf);
    sprintf(str, "%ld", statbuf.st_size);

    // fecha modificacion
    sprintf(date, "%ld", (intmax_t)statbuf.st_mtim.tv_sec);

    strcat(out, "/");
    strcat(out, dp->d_name);
    strcat(out, " ");
    // printf("/%s",dp->d_name);
    strcat(out, str);
    strcat(out, " ");
    strcat(out, date);
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
char* getname(char* node) {
    char* token;
    char* rest = node;
    int index = 0;
    token = strtok_r(rest, " ", &rest);
    return node;
}
char* getsize(char* node) {
    char* token;
    char* rest = node;
    token = strtok_r(rest, " ", &rest);
    // token = strtok_r(rest, " ", &rest);
    strcpy(node, rest);
    return rest;
}
int findNode(char* string, char* list, char* name, char* size, char* name2, char* size2) {
    char* token;
    char* rest = list;
    int find = 0;
    while ((token = strtok_r(rest, "\n", &rest))) {
        if (strcmp(token, string) == 0) {
            find = 1;
            break;
        } else {
            strcpy(name, string);
            strcpy(size, string);
            strcpy(name2, token);
            strcpy(size2, token);
            getname(name);
            getsize(size);
            getname(name2);
            getsize(size2);
            if (strcmp(name, name2) == 0) {
                if (strcmp(size2, size) != 0)
                    return 0;
            }
        }
    }
    if (find == 0) {
        strcpy(name, string);
        strcpy(size, string);
        getname(name);
        getsize(size);
        return 1;
    }
    return 50;
}

// type 0 Elimincion, 1 agregacion
void compareByNOdes(char* data1, char* data2, int type, char* changes) {
    // printf("Datos Existentes \n%s \n",data1);
    // printf("Datos Leidos \n%s ",data2);
    char buff[255];
    char buff1[2048];

    // loop through the string to extract all other tokens
    char* token;
    char* rest = data1;
    while ((token = strtok_r(rest, "\n", &rest))) {
        char name1[1024] = "";
        char size1[1024] = "";
        char name2[1024] = "";
        char size2[1024] = "";
        strcpy(buff, token);
        strcpy(buff1, data2);
        int estade = findNode(buff, buff1, name1, size1, name2, size2);
        if (estade == 0 && type == 1) {
            strcat(changes, "Modificado");
            strcat(changes, name1);
            strcat(changes, " ");
            strcat(changes, size1);
            printf("Modificado %s %s\n", name1, size1);
        }
        if (estade == 1) {
            if (type == 1) {
                strcat(changes, "Agregado");
                strcat(changes, name1);
                strcat(changes, " ");
                strcat(changes, size1);
                printf("Agregado %s %s\n", name1, size1);
            }

            else {
                strcat(changes, "Eliminado");
                strcat(changes, name1);
                strcat(changes, " ");
                strcat(changes, size1);
                printf("Eliminado %s %s\n", name1, size1);
            }
        }
    }
}

void compare(char* changes) {
    char* route = ".config";
    char grafo[2048];
    char dataRead[2048];

    FILE* filePointer;
    filePointer = fopen(route, "r");
    if (filePointer == NULL) {
        printf("entrando al if!");
        memset(grafo, 0, sizeof(grafo));
        magic(0, ".", grafo);
        saveData(route, grafo);
    } else {
        printf("entrando al else!");
        memset(grafo, 0, sizeof(grafo));
        memset(dataRead, 0, sizeof(dataRead));
        magic(0, ".", grafo);
        readFile(".config", dataRead);

        compareByNOdes(dataRead, grafo, 0, changes);

        memset(grafo, 0, sizeof(grafo));
        memset(dataRead, 0, sizeof(dataRead));
        magic(0, ".", grafo);
        readFile(".config", dataRead);

        compareByNOdes(grafo, dataRead, 1, changes);
    }
}
/*
int main(int argc, char* argv[]) {
    // revisamos si hay mas de 1 argumentos, el argumento 0 es el nombre del programa
    if (argc >= 2) {
        char changes[4096];
        memset(changes, 0, sizeof(changes));
        compare(changes);

    } else
        printf("Se espera que haya al menos un argumento.\n");

    exit(0);
}
*/