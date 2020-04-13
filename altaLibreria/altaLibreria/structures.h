#ifndef ALTALIBRERIA_STRUCTURES_H_
#define ALTALIBRERIA_STRUCTURES_H_
#define IP_LENGTH 20

#include "connections.h"

typedef enum _NetworkDebugLevel {
	NW_NO_DISPLAY,
	NW_NETWORK_ERRORS,
	NW_ALL_DISPLAY
} NetworkDebugLevel;
NetworkDebugLevel NETWORK_DEBUG_LEVEL = NW_NO_DISPLAY;

typedef enum _MessageType {
    // Chat
    ENVIAR_MENSAJE,
    MOSTRAR_MENSAJE,
    HANDSHAKE

} MessageType;

typedef struct _MessageHeader {
	MessageType type;
	int data_size;
} MessageHeader;


typedef struct {
	MessageHeader *header;
	void* stream;
} t_paquete;


typedef struct {
    int id;
    int nombre_length;
    char* nombre;
} chat_usuario;


typedef struct {
    int id_usuario;
    int nombre_usuario_length;
    char* nombre_usuario;
    int mensaje_length;
    char* mensaje;
} chat_mensaje;


#endif /* ALTALIBRERIA_STRUCTURES_H_ */
