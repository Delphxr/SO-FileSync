void clean_terminal() {
    printf("\e[1;1H\e[2J");
}

void print_red(char message[]){
    printf("\033[31m%s\033[0m", message);
}

void print_blue(char message[]){
    printf("\033[34;1m%s\033[0m", message);
}


void print_yellow(char message[]){
    printf("\033[1;33m%s\033[0m", message);
}

void progress_bar(double percentage) {
    char bar[] = "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||";
    int pb_width = 70;
    if (percentage > 1) percentage = 1;
    int val = (int)(percentage * 100);
    int lpad = (int)(percentage * pb_width);
    int rpad = pb_width - lpad;
    printf("\r\033[31;1m [\033[34;1m%.*s%*s\033[0m\033[31;1m]\033[1;33m %3d%%\033[0m", lpad, bar, rpad, "", val);
    fflush(stdout);
}

// imprimimos una de las opciones del menu, con todo y color
void print_menu_option(char command[], char parameters[], char description[]) {
    printf(
        "\033[1;24;36m"
        " > %s\t",
        command);
    printf(
        "\033[34;1m"
        "%s\t"
        "\033[7;34;37m",
        parameters);
    printf("%s\033[0m \n", description);
}

// imprimimos una linea para gui
void print_line() {
    printf("\033[1;33m------------------------------------------------------------------------\n\033[0m");
}

void print_logo(){
    printf(
        "                 \033[1;33m██████\033[31;1m╗       \033[1;33m███████\033[31;1m╗\033[1;33m████████\033[31;1m╗\033[1;33m██████\033[31;1m╗ \n"
        "                 \033[1;33m██\033[31;1m╔══\033[1;33m██\033[31;1m╗      \033[1;33m██\033[31;1m╔════╝╚══\033[1;33m██\033[31;1m╔══╝\033[1;33m██\033[31;1m╔══\033[1;33m██\033[31;1m╗\n"
        "                 \033[1;33m██████\033[31;1m╔╝\033[1;33m█████\033[31;1m╗\033[1;33m█████\033[31;1m╗     \033[1;33m██\033[31;1m║   \033[1;33m██████\033[31;1m╔╝\n"
        "                 \033[1;33m██\033[31;1m╔══\033[1;33m██\033[31;1m╗╚════╝\033[1;33m██\033[31;1m╔══╝     \033[1;33m██\033[31;1m║   \033[1;33m██\033[31;1m╔═══╝ \n"
        "                 \033[1;33m██████\033[31;1m╔╝      \033[1;33m██\033[31;1m║        \033[1;33m██\033[31;1m║   \033[1;33m██\033[31;1m║     \n"
        "                 \033[31;1m╚═════╝       ╚═╝        ╚═╝   ╚═╝     \n \033[0m");
}

void print_menu(int conections) {
    print_logo();
    print_yellow("Conexiones actuales:");
    printf("\033[1;33m%50d\n", conections);
    print_line();

    print_menu_option("open ", "<dirección-ip>", "Establece una conexión remota");
    print_menu_option("close", "        ", "Cierra la conexión actual");
    print_menu_option("cd   ", "<directorio> ", "Cambia de directorio remoto");
    print_menu_option("get  ", "<archivo> ", "Recupera un archivo remoto");
    print_menu_option("lcd  ", "<directorio>  ", "Cambia de directorio local");
    print_menu_option("ls   ", "        ", "Lista los archivos del directorio remoto");
    print_menu_option("put  ", "<archivo>", "Envía un archivo a la máquina remota");
    print_menu_option("pwd  ", "        ", "Muestra el directorio activo remoto");
    print_menu_option("quit ", "        ", "Termina el programa");
    print_line();

    print_blue("\n > ");
}