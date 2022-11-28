#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "compare.h"

#define BUFFER_SIZE 2000

// envia un archivo, recibe el socket al que enviar, un buffer, y el archivo abierto
int send_file(int *destination_socket, char *message_buffer[BUFFER_SIZE], FILE **file) {
    int sended_data;
    int binary_data = fread(*message_buffer, 1, sizeof(*message_buffer), *file);
    while (1) {
        sended_data = 0;
        while (sended_data < binary_data) {
            int l = send(*destination_socket, *message_buffer, binary_data, 0);
            if (l < 0) {
                puts("Send failed");
                return 1;
            }
            //printf("%d", l);
            sended_data += l;
        }
        memset(*message_buffer, 0, BUFFER_SIZE);
        if (feof(*file)) break;
        binary_data = fread(*message_buffer, 1, sizeof(*message_buffer), *file);
    }

    printf("Archivo enviado! \n");
}

// recibimos un archivo, recibe el socket del que vamos a recibir, nombre de archivo como lo vamos a guardar, y un buffer
int get_file(int *self_socket, char file_name[], char *message_buffer[BUFFER_SIZE]) {
    // un select para verificar dato no quedarse esperando infinitamente
    struct timeval tv;
    fd_set readfds;
    tv.tv_sec = 2;  // 2 segundos de timeout para respuesta
    tv.tv_usec = 0;
    FD_ZERO(&readfds);
    FD_SET(*self_socket, &readfds);
    int n = *self_socket + 1;

    int bytes_read;

    FILE *f = fopen(file_name, "wb");  // archivo para meter lo que leamos
    if (f == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }

    do {
        select(n, &readfds, NULL, NULL, &tv);

        if (FD_ISSET(*self_socket, &readfds)) {  // revisamos si llegaron datos, si si, los imprimimos/guardamos
            bytes_read = recv(*self_socket, *message_buffer, BUFFER_SIZE, 0);
            if (bytes_read == -1) {
                perror("recv");
                exit(1);
            } else {
                fwrite(*message_buffer, 1, strlen(*message_buffer), f);
                memset(*message_buffer, 0, strlen(*message_buffer));
            }
        } else {
            printf("Tiempo de espera terminado!\n");
            break;
        }  // pasaron 5 segundos y no ha llegado ningun otro stream de datos

    } while (bytes_read > 0);

    fclose(f);
}

int main(int argc, char *argv[]) {
    int self_socket, client_socket;
    struct sockaddr_in server, client;
    int read_size;
    FILE *file;

    // hay solo un argumento!! (recordar que arg[0] es el nombre del programa)
    // ejecutar como servidor
    if (argc == 2) {
        printf("[+] Iniciando programa como servidor...\n");

        char client_message[BUFFER_SIZE];

        // Create socket
        self_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (self_socket == -1) {
            printf("Could not create socket");
        }
        puts("Socket created");
        // Prepare the sockaddr_in structure
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = htons(8889);
        // Bind
        if (bind(self_socket, (struct sockaddr *)&server, sizeof(server)) < 0) {
            perror("bind failed. Error");
            return 1;
        }
        puts("bind done");
        // Listen
        listen(self_socket, 3);
        // Accept and incoming connection
        puts("Waiting for incoming connections...");

        int c = sizeof(struct sockaddr_in);
        // accept connection from an incoming client
        client_socket = accept(self_socket, (struct sockaddr *)&client, (socklen_t *)&c);
        if (client_socket < 0) {
            perror("accept failed");
            return 1;
        }
        puts("Connection accepted");
        // Receive a message from client
        memset(client_message, 0, BUFFER_SIZE);
        char command[60];    // comando prncipal (open, close, cd, get, etc)
        char parameter[60];  // parametros del commando (ip, archivo, etc)
        while ((read_size = recv(client_socket, client_message, BUFFER_SIZE, 0)) > 0) {
            sscanf(client_message, "%s %s", command, parameter);
            char path_to_file[256];
            strcpy(path_to_file, argv[1]);
            strcat(path_to_file, "/");
            strcat(path_to_file, parameter);

            puts(client_message);

            // enviar un archivo
            if (strcmp(command, "get") == 0) {
                file = fopen(path_to_file, "rb");

                if (file == NULL) {
                    printf("Error opening file %s!\n", client_message);
                    continue;
                }

                printf("Iniciando envio del archivo... \n");

                char *message_buffer_ptr = client_message;  // adapta el string a un puntero para poderlo pasar por referencia
                send_file(&client_socket, &message_buffer_ptr, &file);

            }  // recibir un archivo
            else if (strcmp(command, "put") == 0) {
                printf("Recibiendo archivo... \n");
                char *message_buffer_ptr = client_message;  // adapta el string a un puntero para poderlo pasar por referencia
                get_file(&client_socket, path_to_file, &message_buffer_ptr);

            }  // eliminar un archivo
            else if (strcmp(command, "delete") == 0) {
                if (remove(path_to_file) == 0) {
                    printf("Archivo eliminado correctamente");
                } else {
                    printf("Hubo un error al eliminar %s", path_to_file);
                }
            } else if (strcmp(command, "grafo") == 0) {
                printf("Enviando grafo... \n");
                char changes[2000];
                memset(changes, 0, sizeof(changes));
                get_remote(changes, argv[1]);
                send(client_socket, changes, strlen(changes), 0);
            }
            printf("\n");
        }
        if (read_size == 0) {
            puts("Client disconnected");
            fflush(stdout);
        } else if (read_size == -1) {
            perror("recv failed");
        }

    }
    // tenemos 2 argumentos (nombre del directorio local a sincronizar y el IP de la primer máquina)
    // ejecutar como cliente
    else if (argc == 3) {
        printf("[+] Iniciando programa como cliente...\n");

        char message[BUFFER_SIZE], server_reply[BUFFER_SIZE];

        client_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (client_socket == -1) {
            printf("Could not create socket");
        }
        puts("Socket created");
        server.sin_addr.s_addr = inet_addr(argv[2]);
        server.sin_family = AF_INET;
        server.sin_port = htons(8889);
        // Connect to remote server
        if (connect(client_socket, (struct sockaddr *)&server, sizeof(server)) < 0) {
            perror("connect failed. Error");
            return 1;
        }
        puts("Connected\n");
        // keep communicating with server
        while (1) {
            // ****************************
            // pedimos el grafo de archivos
            // ****************************

            memset(message, 0, BUFFER_SIZE);
            strcat(message, "grafo .");
            if (send(client_socket, message, strlen(message), 0) < 0) {
                puts("Send failed");
                return 1;
            }
            if (recv(client_socket, server_reply, BUFFER_SIZE, 0) < 0) {
                puts("recv failed");
                break;
            }

            char grafo_remoto[2000];
            strcpy(grafo_remoto, server_reply);

            // ****************************
            // mandamos el grafo a comparar
            // ****************************
            printf("grafo remoto: \n%s\n\n", grafo_remoto);
            char diferencias[4096];
            compare_remoto(diferencias, grafo_remoto, argv[1]);
            printf("\n Diferencias:\n%s\n---\n", diferencias);

            // ****************************
            // hacemos operaciones
            // ****************************

            // loop through the string to extract all other tokens
            char *token;
            char *rest = diferencias;

            char operacion[24];
            char lugar[24];
            char nombre[128];
            char size[24];
            sleep(1);
            while ((token = strtok_r(rest, "\n", &rest))) {
                sscanf(token, "%s %s %s %s", operacion, lugar, nombre, size);

                if (strcmp(operacion, "Agregado") == 0 || strcmp(operacion, "Modificado") == 0) {
                    printf("operacion de agregado de %s...\n", nombre);
                    if (strcmp(lugar, "remote") == 0) {
                        memset(message, 0, BUFFER_SIZE);
                        strcpy(message, "get ");
                        strcat(message, nombre);

                        // mandamos el nombre de archivo or recibir
                        if (send(client_socket, message, strlen(message), 0) < 0) {
                            puts("Send failed");
                            return 1;
                        }

                        sleep(1);
                        char path_to_file[256];
                        strcpy(path_to_file, argv[1]);
                        strcat(path_to_file, "/");
                        strcat(path_to_file, nombre);

                        char *message_buffer_ptr = server_reply;  // adapta el string a un puntero para poderlo pasar por referencia
                        get_file(&client_socket, path_to_file, &message_buffer_ptr);
                    } else {
                        memset(message, 0, BUFFER_SIZE);
                        strcpy(message, "put ");
                        strcat(message, nombre);

                        // mandamos el nombre de archivo or recibir
                        if (send(client_socket, message, strlen(message), 0) < 0) {
                            puts("Send failed");
                            return 1;
                        }

                        sleep(1);

                        //////////////////////////////////////////
                        char path_to_file[256];
                        strcpy(path_to_file, argv[1]);
                        strcat(path_to_file, "/");
                        strcat(path_to_file, nombre);
                        file = fopen(path_to_file, "rb");

                        if (file == NULL) {
                            printf("Error opening file %s!\n", path_to_file);
                            continue;
                        }

                        printf("Iniciando envio del archivo... \n");

                        memset(message, 0, BUFFER_SIZE);
                        char *message_buffer_ptr = message;  // adapta el string a un puntero para poderlo pasar por referencia
                        send_file(&client_socket, &message_buffer_ptr, &file);
                        fclose(file);
                    }
                }

                sleep(2.5);
            }
            break;

        }
        close(self_socket);
    }
    // manejo de errores
    else if (argc < 2) {
        printf("[!] ERROR: Debe ingresar al menos un argumento! \n");
    } else {
        printf("[!] ERROR: Cantidad de argumentos no valida! \n");
    }
}