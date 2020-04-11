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

t_log* logger;
t_config* config_file;

int config_listen_port;
char* config_server_name;
t_list* lista_usuarios;

void *server_function(void *arg);
void tests_server();
chat_usuario* find_usuario(int id_buscado);
void enviar_mensaje(chat_usuario* usuario,chat_mensaje* mensaje);


int main() {
    logger = log_create("server.log", "SERVER", 1, LOG_LEVEL_TRACE);

    config_file = config_create("server_config");

    if (!config_file) {
        log_error(logger, "No se encontró el archivo de configuración");
        return 1;
    }

    config_listen_port = config_get_int_value(config_file, "LISTEN_PORT");
    config_server_name = config_get_string_value(config_file, "SERVER_NAME");

    log_info(logger, \
        "Configuración levantada\n\tLISTEN_PORT: %d\n\tSERVER_NAME: %s.", \
        config_listen_port, \
        config_server_name);

    lista_usuarios = list_create();

    pthread_t server_thread;
    pthread_create(&server_thread, NULL, server_function, NULL);
    //tests_server();
    pthread_join(server_thread, NULL);

    return 0;

}


void *server_function(void *arg) {

    int socket;

    if((socket = create_socket()) == -1) {
        log_error(logger, "Error al crear el socket");
    }

    if ((bind_socket(socket, config_listen_port)) == -1) {
        log_error(logger, "Error al bindear el socket");
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
            case ENVIAR_MENSAJE:;
                {
                    chat_mensaje* mensaje = void_a_mensaje(list_get(cosas, 0));
                    for (int i = 0; i < list_size(lista_usuarios); ++i) {
                        chat_usuario* usuario = list_get(lista_usuarios, i);
                        enviar_mensaje(usuario, mensaje);
                    }
                    break;
                }
            case HANDSHAKE:;
                {
                    // Cuando se conecta alguien nuevo lo agrego a la lista de usuarios
                    chat_usuario* nuevo_usuario = malloc(sizeof(chat_usuario));

                    int usuario_socket;
                    usuario_socket = create_socket();
                    int hola = connect_socket(usuario_socket, "localhost", 6006);

                    //En el handshake el server completa el username y el cliente el id
                    char* username = list_get(cosas, 0);

                    nuevo_usuario->id = usuario_socket;
                    nuevo_usuario->nombre_length = 5;
                    nuevo_usuario->nombre = strdup(username);
                    list_add(lista_usuarios, nuevo_usuario);

                    send(fd, &usuario_socket, sizeof(int), 0);
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


chat_usuario* find_usuario(int id_buscado){
    int key_search(chat_usuario* un_segmento){
        return un_segmento->id == id_buscado;
    }

     return list_find(lista_usuarios,(void*)key_search);
}


void enviar_mensaje(chat_usuario* usuario, chat_mensaje* mensaje){
    t_paquete *package = create_package(MOSTRAR_MENSAJE);
    int mensaje_size = sizeof(int) + mensaje->mensaje_length + sizeof(int);
    add_to_package(package, mensaje_a_void(mensaje), mensaje_size);
    log_info(logger, "Envio el mensaje %s, al socket %d", mensaje->mensaje, usuario->id);
    if(send_package(package, usuario->id) == -1){
        log_error(logger, "Error al enviar el mensaje.");
        free_package(package);
        return;
    }
}


void tests_server(){
//
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
