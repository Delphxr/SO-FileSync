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

#define BUFFER_SIZE 2000

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
        while ((read_size = recv(client_socket, client_message, BUFFER_SIZE, 0)) > 0) {
            file = fopen(client_message, "rb");

            if (file == NULL) {
                printf("Error opening file %s!\n", client_message);
                continue;
            }

            printf("Iniciando envio del archivo... \n");

            int sended_data;
            int binary_data = fread(client_message, 1, sizeof(client_message), file);
            while (1) {
                sended_data = 0;
                while (sended_data < binary_data) {
                    int l = send(client_socket, client_message, binary_data, 0);
                    sended_data += l;
                }
                memset(client_message, 0, BUFFER_SIZE);
                if (feof(file)) break;
                binary_data = fread(client_message, 1, sizeof(client_message), file);
            }

            printf("Archivo enviado! \n");
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

        self_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (self_socket == -1) {
            printf("Could not create socket");
        }
        puts("Socket created");
        server.sin_addr.s_addr = inet_addr(argv[2]);
        server.sin_family = AF_INET;
        server.sin_port = htons(8889);
        // Connect to remote server
        if (connect(self_socket, (struct sockaddr *)&server, sizeof(server)) < 0) {
            perror("connect failed. Error");
            return 1;
        }
        puts("Connected\n");
        // keep communicating with server
        while (1) {
            printf("Enter message : ");

            scanf("%s", message);
            printf("input es: %s\n", message);
            // Send some data
            if (send(self_socket, message, strlen(message), 0) < 0) {
                puts("Send failed");
                return 1;
            }

            // un select para verificar dato no quedarse esperando infinitamente
            struct timeval tv;
            fd_set readfds;
            tv.tv_sec = 5;  // 5 segundos de timeout para respuesta
            tv.tv_usec = 0;
            FD_ZERO(&readfds);
            FD_SET(self_socket, &readfds);
            int n = self_socket + 1;

            int bytes_read;

            FILE *f = fopen("test.txt", "wb");  // archivo para meter lo que leamos, para facilitar la respuesta a pagina web
            if (f == NULL) {
                printf("Error opening file!\n");
                exit(1);
            }

            do {
                select(n, &readfds, NULL, NULL, &tv);

                if (FD_ISSET(self_socket, &readfds)) {  // revisamos si llegaron datos, si si, los imprimimos/guardamos
                    bytes_read = recv(self_socket, server_reply, BUFFER_SIZE, 0);
                    if (bytes_read == -1) {
                        perror("recv");
                        exit(1);
                    } else {
                        fwrite(server_reply, 1, strlen(server_reply), f);
                        memset(server_reply, 0, strlen(server_reply));
                    }
                } else {
                    printf("Tiempo de espera terminado!\n");
                    break;
                }  // pasaron 5 segundos y no ha llegado ningun otro stream de datos

            } while (bytes_read > 0);

            fclose(f);
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