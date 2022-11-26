#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 2000
/*
int get_file(int *sock, char *server_reply[BUFFER_SIZE], char *parameter[60]) {
    FILE *file;
    file = fopen(*parameter, "ab");

    if (file == NULL) {
        print_red("[!] Hubo un error al crear el archivo de recepción!");
        return -1;
    }

    printf("Recibiendo archivo\n");

    int received_data;
    int binary_data = read(*sock, *server_reply, BUFFER_SIZE);
    received_data += binary_data;
    int index = 0;
    while (binary_data > 0) {
        printf("%s\n", *server_reply);
        fwrite(*server_reply, 1, BUFFER_SIZE, file);

        binary_data = read(*sock, *server_reply, BUFFER_SIZE);
        usleep(10000);
    }
    fclose(file);

    return 0;
}
*/
/*
// es un hilo creado por el listener thread, se encarga de atender a los clientes que se unan al servicio
void *connection_handler(void *socket_desc) {
    // Get the socket descriptor
    int sock = *(int *)socket_desc;

    int read_size, c;
    char *message, client_message[BUFFER_SIZE];
    FILE *file;

    char command[60];    // comando prncipal (open, close, cd, get, etc)
    char parameter[60];  // parametros del commando (ip, archivo, etc)

    // Receive a message from client
    while ((read_size = recv(sock, client_message, BUFFER_SIZE, 0)) > 0) {
        // dividimos el input entre commando-parametro
        sscanf(client_message, "%s %s", command, parameter);

        if (strcmp(command, "cd") == 0) {
            char pwd[100];

            chdir(parameter);
            strcpy(client_message, getcwd(pwd, 100));
            send(sock, client_message, strlen(client_message), 0);

        } else if (strcmp(command, "get") == 0) {
            // vamos a leer y devolver un mensaje con el archivo!!!!!

            file = fopen(parameter, "rb");
            if (file == NULL) {
                printf("Error opening file!\n");
                strcpy(client_message, "");
                send(sock, client_message, strlen(client_message), 0);
                memset(client_message, 0, sizeof(client_message));
                continue;
            }

            print_blue("Iniciando envio del archivo... \n");

            int sended_data;
            int binary_data = fread(client_message, 1, sizeof(client_message), file);
            while (1) {
                sended_data = 0;
                while (sended_data < binary_data) {
                    int l = send(sock, client_message, strlen(client_message), 0);
                    usleep(10000);
                    sended_data += l;
                }
                memset(client_message, 0, sizeof(client_message));
                if (feof(file)) break;
                binary_data = fread(client_message, 1, sizeof(client_message), file);
            }

            printf("Archivo enviado! \n");

            fclose(file);
        } else if (strcmp(command, "lcd") == 0) {
            printf("commando lcd!");
        } else if (strcmp(command, "ls") == 0) {
            strcpy(client_message, print_directorio());
            send(sock, client_message, strlen(client_message), 0);
        } else if (strcmp(command, "put") == 0) {
            printf("Recibiendo el archivo...\n");
            memset(client_message, 0, sizeof(client_message));

            char *reply_ptr = client_message;
            char *par_ptr = parameter;
            get_file(&sock, &reply_ptr, &par_ptr);
        } else if (strcmp(command, "pwd") == 0) {
            char pwd[100];

            strcpy(client_message, getcwd(pwd, 100));
            send(sock, client_message, strlen(client_message), 0);
        }

        memset(client_message, 0, sizeof(client_message));
    }
    if (read_size == 0) {
        puts("Client disconnected");
        actual_conections--;
        fflush(stdout);
    } else if (read_size == -1) {
        perror("recv failed");
    }
    // Free the socket pointer
    free(socket_desc);
    return 0;
}

*/

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
        while ((read_size = recv(client_socket, client_message, BUFFER_SIZE, 0)) > 0) {
            // Send the message back to client
            /*send(client_socket, client_message, strlen(client_message), 0);
            memset(client_message, 0, strlen(client_message));*/
            file = fopen(client_message, "rb");
            if (file == NULL) {
                printf("Error opening file!\n");
                continue;
            }

            printf("Iniciando envio del archivo... \n");

            int sended_data;
            int binary_data = fread(client_message, 1, sizeof(client_message), file);
            while (1) {
                sended_data = 0;
                while (sended_data < binary_data) {
                    int l = send(client_socket, client_message, strlen(client_message), 0);
                    sended_data += l;
                }
                memset(client_message, 0, strlen(client_message));
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
                        //fprintf(f, "%.*s", bytes_read, buf);
                        fwrite(server_reply, 1, BUFFER_SIZE, f);
                        memset(server_reply, 0, strlen(server_reply));
                    }
                } else  // pasaron 5 segundos y no ha llegado ningun otro stream de datos
                    break;

            } while (bytes_read > 0);

            fclose(f);
            // Receive a reply from the server
            /*if (recv(self_socket, server_reply, BUFFER_SIZE, 0) < 0) {
                puts("recv failed");
                break;
            }
            printf("Server reply : %s \n", server_reply);

            memset(server_reply, 0, strlen(server_reply));*/
        }
        close(self_socket);

    }
    // manejo de errores
    else if (argc < 2) {
        printf("[!] ERROR: Debe ingresar al menos un argumento! \n");
    } else {
        printf("[!] ERROR: Cantidad de argumentos no valida! \n");
    }

    /*

    // creamos el thread listener (server)
    pthread_t listener;
    int return_code = pthread_create(&listener, NULL, listener_thread, NULL);
    if (return_code) {
        printf("ERROR; return code from pthread_create() is %d\n", return_code);
        exit(-1);
    }

    // client
    int sock;
    struct sockaddr_in server;
    char server_reply[BUFFER_SIZE];

    // menu loop
    while (1) {
        clean_terminal();
        print_menu(actual_conections);

        // menu input
        fgets(input, 60, stdin);
        sscanf(input, "%s %s", command, parameter);
        // condiciones del menu

        if (strcmp(command, "open") == 0) {
            // Create socket
            if (sock != -1) {
                sock = socket(AF_INET, SOCK_STREAM, 0);
                if (sock == -1) {
                    printf("Could not create socket");
                }

                server.sin_addr.s_addr = inet_addr(parameter);
                server.sin_family = AF_INET;
                server.sin_port = htons(8888);
                // Connect to remote server
                if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
                    print_red(" [!] Connect failed. Error");
                }
            }
        } else if (strcmp(command, "close") == 0) {
            if (sock != -1) {
                close(sock);
            }

        } else if (strcmp(command, "get") == 0) {
            if (sock != -1) {
                // Send some data
                if (send(sock, input, strlen(input), 0) < 0) {
                    print_red("[!] Send failed");
                    getchar();
                    continue;
                }

                memset(server_reply, 0, sizeof(server_reply));  // limapiamos el buffer

                printf("Recibiendo el archivo...\n");
                char *reply_ptr = server_reply;
                char *par_ptr = parameter;
                get_file(&sock, &reply_ptr, &par_ptr);
            }

        } else if (strcmp(command, "put") == 0) {
            // Send some data
            if (send(sock, input, strlen(input), 0) < 0) {
                print_red("[!] Send failed");
                getchar();
                continue;
            }
            usleep(250000);

            FILE *file;
            file = fopen(parameter, "rb");
            if (file == NULL) {
                printf("Error opening file!\n");
                getchar();
                continue;
            }

            print_blue("Iniciando envio del archivo... \n");

            int sended_data;
            int binary_data = fread(server_reply, 1, sizeof(server_reply), file);
            while (1) {
                sended_data = 0;
                while (sended_data < binary_data) {
                    int l = send(sock, server_reply, strlen(server_reply), 0);
                    usleep(10000);
                    sended_data += l;
                }
                memset(server_reply, 0, sizeof(server_reply));
                if (feof(file)) break;
                binary_data = fread(server_reply, 1, sizeof(server_reply), file);
            }

            printf("Archivo enviado! \n");

            fclose(file);

        } else {
            print_red("[!] Comando Desconocido!\n");
        }
        getchar();
        memset(server_reply, 0, sizeof(server_reply));  // limapiamos el buffer
        memset(command, 0, sizeof(command));
        memset(parameter, 0, sizeof(parameter));
    }*/
}