
/******************************************************
* tunelizador http
* config.h
*******************************************************/

#ifndef CONFIG_H
#define CONFIG_H

/******************
* configuracion
*******************/

// tamanio baffers
#define SZ_SBUF 1024 
#define SZ_REQ 4096
#define SZ_HOST 64
#define SZ_URL 128
#define SZ_FILE 64

// banner
#define BANNER "HTTP Tunnel v0.3 (C) 2013 Wassamei Likesomaway"

// debug ?
#define VERBOSE 1

// puerto de escucha (client)
#define B_HOST "127.0.0.1"
#define B_PORT 61000

// forward a host:port (server)
#define F_HOST "127.0.0.1"
#define F_PORT 22

//  pendiente...
#define URL "server.jsp"

// user-agent, protocolo http (client)
#define USER_AGENT "Mozilla/5.0 (Windows NT 6.0; rv:12.0)"

// datos conexion servidor http (client)
#define HTTP_SERVER "127.0.0.1"
#define HTTP_PORT 80
#define HTTP_VER 0

// tunning conexion http (client)
#define HTTP_TIMEOUT 15	// en sec
#define HTTP_INTERVAL 100 // en msec (100)

// timeout lectura socket (client, server)
#define RECV_TIMEOUT 0

// fifos de envio y recepcion datos (server)
#define F_RECV "/tmp/.recv"
#define F_SEND "/tmp/.send"

// fichero pid  (server)
#define F_PID "/tmp/.pid"

// directorio de trabajo daemon (server)
#define WORK_DIR "/tmp"

// fichero de logs (server)
#define F_LOG "server.log"

// permisos rw files (server)
#define F_MOD 0644

// prioridad daemon (19 mas baja) (server)
#define PRIO 19

// tiempo inactividad (client, server)
#define MAX_TIME 300

#endif
