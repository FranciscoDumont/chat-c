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

void *server_function(void *arg);
void tests_server();

int main() {
    logger = log_create("servr.log", "SERVER", 1, LOG_LEVEL_TRACE);

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
        //TODO:retornar algun error?
    }
    if ((bind_socket(socket, config_listen_port)) == -1) {
        log_error(logger, "Error al bindear el socket");
        //TODO:retornar algun error?
    }

    //TODO revisar si esta bien
    //--Funcion que se ejecuta cuando se conecta un nuevo programa
    void new(int fd, char *ip, int port) {
        if(&fd != null && ip != null && &port != null) {
            log_info(logger, "Nueva conexión");
        }
    }

    //TODO revisar si esta bien
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
                    char *path = ((char *) list_get(cosas, 0));
                    size_t length = *((size_t*) list_get(cosas, 1));
                    int flags = *((int*) list_get(cosas, 2));
                    muse_map(path,length,flags);
                    break;
                }

            default: {
                log_warning(logger, "Operacion desconocida. No quieras meter la pata\n");
                break;
            }
        }
    }
    log_info(logger, "Hilo de servidor iniciado...");
    start_server(socket, &new, &lost, &incoming);
}


void * crear_consola() {

    comando_t comando;

    char *linea;
    int quit = 0;

    printf("Bienvenido/a a la consola de %s\n",unString);

    while(quit == 0){

        linea = readline("> ");
        if (linea && !linea[0]) {
            quit = 1;
        }else{
            add_history(linea);
            vaciar_comando(&comando);
            cargar_comando(&comando,linea);

            if((strcmp(comando.comando,"exit")==0)){
                quit = 1;
            }else{
                (*execute)(&comando);
            }
        }
        free(linea);
    }
    return EXIT_SUCCESS;
}


void tests_server(){

    //mem_assert recive mensaje de error y una condicion, si falla el test lo loggea
    #define test_assert(message, test) do { if (!(test)) { log_error(test_logger, message); tests_fail++; } tests_run++; } while (0)
    t_log* test_logger = log_create("memory_tests.log", "MEM", true, LOG_LEVEL_TRACE);
    int tests_run = 0;
    int tests_fail = 0;

    int id = 1;
    muse_init(id, "localhost", 5003);

    int tmp;
    tmp = muse_alloc(10, id);
    test_assert("Alloc 1", tmp == 5);

    tmp = muse_alloc(16, id);
    test_assert("Alloc 2", tmp == 5+10+5);

    log_warning(test_logger, "Pasaron %d de %d tests", tests_run-tests_fail, tests_run);
    log_destroy(test_logger);
}
