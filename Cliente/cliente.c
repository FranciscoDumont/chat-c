//
// Created by utnso on 17/03/20.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <signal.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <altaLibreria/connections.h>
#include <altaLibreria/structures.h>
#include <readline/readline.h>
#include <readline/history.h>

t_log* logger;
t_config* config_file;

char* config_server_ip;
int config_server_port;
int server_socket = 0;
int listen_port;

void *server_function(void *arg);
void tests_server();
int enviar_mensaje(char* mensaje);
void mostrar_mensaje(chat_mensaje* mensaje);
void * crear_consola();
void handshake();

chat_usuario* self_usuario;

int main() {
    logger = log_create("cliente.log", "CLIENTE", 1, LOG_LEVEL_TRACE);

    config_file = config_create("cliente_config");

    if (!config_file) {
        log_error(logger, "No se encontró el archivo de configuración");
        return 1;
    }

    config_server_ip = config_get_string_value(config_file, "SERVER_IP");
    config_server_port = config_get_int_value(config_file, "SERVER_PORT");

    log_info(logger, \
        "Configuración levantada\n\tSERVER IP: %s\n\tSERVER_PORT: %d", \
        config_server_ip, \
        config_server_port);

    if ((server_socket = create_socket()) == -1) {
        printf("Error al crear el socket\n");
        return -1;
    }

    if (connect_socket(server_socket, config_server_ip, config_server_port) == -1) {
        printf("Error al conectarse al servidor\n");
        return -1;
    }

    pthread_t server_thread;
    pthread_create(&server_thread, NULL, server_function, NULL);
    pthread_detach(server_thread);

    nanosleep((const struct timespec[]){{0, 500000000L}}, NULL); // Duermo medio seg
    char* username = readline("\n\nPonete un nombre changuito\n> ");
    self_usuario = malloc(sizeof(chat_usuario));
    self_usuario->nombre = username;
    self_usuario->nombre_length = strlen(username) + 1;
    self_usuario->id = 0;

    // Hace el handshake para conseguir el id
    handshake();

    pthread_t console_thread;
    pthread_create(&console_thread, NULL, crear_consola, NULL);

    log_info(logger, "Se creo el usuario: %s#%d", self_usuario->nombre, self_usuario->id);

    //tests_server();

    pthread_join(console_thread, NULL);
    free(self_usuario);
    return 0;

}


void *server_function(void *arg) {

    int socket;

    if((socket = create_socket()) == -1) {
        log_error(logger, "Error al crear el socket");
    }

    // Bindeo el socket a un puerto random entre [6000-6999]
    srand(time(NULL));
    int todo_mal = 1;
    while(todo_mal){
        listen_port = rand()%1000;
        listen_port += 6000;
        if ((bind_socket(socket, listen_port)) == -1) {
            log_error(logger, "Error al bindear el socket");
        } else {
            todo_mal = 0;
        }
    }


    //--Funcion que se ejecuta cuando se conecta un nuevo programa
    void new(int fd, char *ip, int port) {
        if(&fd != null && ip != null && &port != null) {
            log_info(logger, "Nueva conexión");
        }
    }

    //--Funcion que se ejecuta cuando se pierde la conexion con un cliente
    void lost(int fd, char *ip, int port) {
        if(&fd == null && ip == null && &port == null){
            log_info(logger, "Se perdió una conexión");
            //Cierro la conexión fallida
            log_info(logger, "Cerrando conexión");
            close(fd);
        }
    }

    //--funcion que se ejecuta cuando se recibe un nuevo mensaje de un cliente ya conectado
    void incoming(int fd, char *ip, int port, MessageHeader *headerStruct) {

        t_list *cosas = receive_package(fd, headerStruct);

        switch (headerStruct->type) {
            case MOSTRAR_MENSAJE:;
                {
                    chat_mensaje* mensaje = void_a_mensaje(list_get(cosas, 0));
                    mostrar_mensaje(mensaje);
                    break;
                }

            default: {
                log_warning(logger, "Operacion desconocida. No quieras meter la pata\n");
                break;
            }
        }
    }
    log_info(logger, "Hilo de servidor iniciado...");
    start_multithread_server(socket, &new, &lost, &incoming);
}


void * crear_consola() {
    char *linea;
    int quit = 0;
    while(quit == 0){
        linea = readline("");
        if (linea && !linea[0]) {
            quit = 1;
        }else{
            add_history(linea);
            if((strcmp(linea,"exit")==0)){
                quit = 1;
            }else{
                enviar_mensaje(linea);
            }
        }
        free(linea);
    }
    return EXIT_SUCCESS;
}


int enviar_mensaje(char* mensaje){
    t_paquete *package = create_package(ENVIAR_MENSAJE);
    chat_mensaje* nuevo_mensaje = crear_mensaje(mensaje, self_usuario->id, self_usuario->nombre);
    int nuevo_mensaje_size = sizeof(int)*3 + nuevo_mensaje->mensaje_length + nuevo_mensaje->nombre_usuario_length;

    chat_mensaje* test = void_a_mensaje(mensaje_a_void(nuevo_mensaje));

    add_to_package(package, mensaje_a_void(nuevo_mensaje), nuevo_mensaje_size);
    if(send_package(package, server_socket) == -1){
        log_error(logger, "Error al enviar el mensaje.");
        free_package(package);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


void handshake(){
    t_paquete *package = create_package(HANDSHAKE);
    add_to_package(package, (void*)self_usuario->nombre, self_usuario->nombre_length);
    void* _listen_port = malloc(sizeof(int));
    *((int*) _listen_port) = listen_port;
    add_to_package(package, _listen_port, sizeof(int));
    send_package(package, server_socket);
    free(_listen_port);
    free_package(package);
    recv(server_socket, &self_usuario->id, sizeof(int), 0);
}

void mostrar_mensaje(chat_mensaje* mensaje){
    custom_print("%s#%d: %s\n", mensaje->nombre_usuario, mensaje->id_usuario, mensaje->mensaje);
}



void tests_server(){

//    //mem_assert recive mensaje de error y una condicion, si falla el test lo loggea
//    #define test_assert(message, test) do { if (!(test)) { log_error(test_logger, message); tests_fail++; } tests_run++; } while (0)
//    t_log* test_logger = log_create("memory_tests.log", "MEM", true, LOG_LEVEL_TRACE);
//    int tests_run = 0;
//    int tests_fail = 0;
//
//    int id = 1;
//    muse_init(id, "localhost", 5003);
//
//    int tmp;
//    tmp = muse_alloc(10, id);
//    test_assert("Alloc 1", tmp == 5);
//
//    tmp = muse_alloc(16, id);
//    test_assert("Alloc 2", tmp == 5+10+5);
//
//    log_warning(test_logger, "Pasaron %d de %d tests", tests_run-tests_fail, tests_run);
//    log_destroy(test_logger);
}