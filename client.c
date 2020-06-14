
/******************************************************
* tunelizador http 
* client.c 
*******************************************************/

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <netdb.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "config.h"
#include "common.h"


/******************
* macros aux 
*******************/

#undef PROMPT
#define PROMPT "(client) "

#define HTTP_SEND 0x10
#define HTTP_RECV 0x20
#define HTTP_CTRL 0x30


/******************
* variables globales
*******************/

// variables globales
int sock_h, sock_s, sock_c, b_port, http_port, f_port, verbose;
char http_server[SZ_HOST], f_host[SZ_HOST], b_host[SZ_HOST];
char url[SZ_URL];


/******************
* funciones
*******************/

// libera recursos
void clean() 
{
	MSG("exiting...\n");

	// cerramos socket
	MSG("closing sockets...\n");
	close(sock_s);
	close(sock_c);
	close(sock_h);

	MSG(">>> done <<<\n\n");
}


// envia peticion http
int http(int action, char *sbuf, int sz_sbuf)
{
#if HTTP_VER == 1
	static int con = 0;
#else
	int con = 0;
#endif
	int sz, opt, r = -1, cod = 0;
	struct sockaddr_in a;
	struct hostent *host;
	char *idx;
	long arg;
	socklen_t lon;
	static char req[SZ_REQ];
	struct timeval tv;
	fd_set set;

	// conectado?
	if (!con)
	{
		// resolucion host
		host = gethostbyname(http_server);

		// parametros conexion
		memset(&a, 0, sizeof(a));
		a.sin_family = AF_INET;
		a.sin_port = htons(http_port);
		a.sin_addr = *((struct in_addr *)host->h_addr);

		// creamos socket
		TRY(sock_h = socket(AF_INET, SOCK_STREAM, 0));

		// sin bloqueo
		arg = fcntl(sock_h, F_GETFL, 0); 
		fcntl(sock_h, F_SETFL, arg | O_NONBLOCK); 

		// conectamos
		if (connect(sock_h, (struct sockaddr *)&a, sizeof(a)) == -1)
		{
			// error o espera?
			if (errno == EINPROGRESS)
			{
				// timeout
				tv.tv_sec = HTTP_TIMEOUT;
				tv.tv_usec = 0;

				// reset fds
				FD_ZERO(&set);
				FD_SET(sock_h, &set);

				// definimos timeout de conexion
				if (select(sock_h + 1, 0, &set, 0, &tv) > 0)
				{
					lon = sizeof(int);
					getsockopt(sock_h, SOL_SOCKET, SO_ERROR, &opt, &lon);

					// no conectado
					if (opt) ABORT("http connect\n");
				}
			}

			// error
			else ABORT("http connect\n");
		}

		// con bloqueo
		arg = fcntl(sock_h, F_GETFL, 0); 
		fcntl(sock_h, F_SETFL, arg & ~O_NONBLOCK); 

		// conectados 
		con = 1;
	}

	// -----------------
	// enviamos peticion
	// -----------------

	// accion ?
	switch (action)
	{
		case HTTP_CTRL: 

			// start con f_host y f_port 
			sprintf(req, "GET /%s?cmd=ctrl&ip=%s&port=%d HTTP/1.%d\r\n", 
				url, f_host, f_port, HTTP_VER);

			break;

		case HTTP_SEND: 

			// enviamos datos socket 
			sprintf(req, "POST /%s?cmd=send&ip=%s&port=%d HTTP/1.%d\r\n", 
				url, f_host, f_port, HTTP_VER);

			break;

		case HTTP_RECV: 

			// solicitamos datos socket
			sprintf(req, "GET /%s?cmd=recv&ip=%s&port=%d HTTP/1.%d\r\n", 
				url, f_host, f_port, HTTP_VER);

			break;
	}

	// construimos peticion post
	sprintf(req, "%sHost: %s\r\n", req, http_server);
	sprintf(req, "%sUser-Agent: %s\r\n", req, USER_AGENT);
	sprintf(req, "%sContent-Type: application/octet-stream\r\n", req);
	sprintf(req, "%sContent-Length: %d\r\n\r\n", req, sz_sbuf);
	
	// tamanio peticion
	sz = strlen(req);

	// anexamos payload ?
	if (action == HTTP_SEND)
	{
		idx = req + sz;
		memcpy(idx, sbuf, sz_sbuf);
		sz += sz_sbuf;
	}

	// envio peticion 
	TRY(send(sock_h, req, sz, 0));

	//
	// respuesta
	//

#if HTTP_VER == 1
        TRY(sz = xrecv(sock_h, req, SZ_REQ, HTTP_TIMEOUT));
#else
	// init vars
	idx = req; sz = 0;
	memset(req, 0, SZ_REQ);

	// respuesta hasta -1,  por trozos ;)
	while (((sz_sbuf = xrecv(sock_h, idx, SZ_REQ - sz, HTTP_TIMEOUT)) > 0) && sz < SZ_REQ)
	{
		sz += sz_sbuf; 
		idx += sz_sbuf;
	}
#endif

	// recibido datos? 
	if (sz) 
	{
		// codigo = 200 ?
		if (! sscanf(req, "HTTP/1.1 %d", &cod) || cod != 200) 
		{	
			ERR("http code != 200: %d ->\n%s\n", cod, req);	
		}

		// http error
		else 
		{
			// donde comienza payload ?
			if ((action == HTTP_RECV) && (idx = strstr(req, "\r\n\r\n")))
			{
				*idx = 0; idx += 4;
				r = sz - (idx - req);
				memcpy(sbuf, idx, r);
			}
		}
	
		// ------------
		// fin peticion
		// ------------

		// se ha cerrado la conexion?
		if (strstr(req, "Connection: close"))
		{
			// cerramos socket
			close(sock_h);
			
			// no conectados
			con = 0;
		}
	}

	// esperamos
	usleep(HTTP_INTERVAL * 1000);

	return action == HTTP_RECV ? r : 0;
}


// usage
void usage()
{
	printf("Usage: client -u http://hostname:port/url"
		" [-vh] [-b bind_ip:port] [-f forward_ip:port]\n\n");

	exit(-1);
}


// funcion principal
int main(int argc, char *argv[])
{
	int sz, on = 1, c;
	char sbuf[SZ_SBUF], *idx;
	struct sockaddr_in ads, adc; 
	socklen_t len; 


        // desactivamos buffer salida
        setbuf(stdout, 0); 

	// banner
	printf("%s\n", BANNER);
	printf("Starting client...\n\n");

	//
	// usage 
	//

	// config por defecto
	verbose = VERBOSE;
	strncpy(http_server, HTTP_SERVER, SZ_HOST);
	strncpy(b_host, B_HOST, SZ_HOST);
	strncpy(f_host, F_HOST, SZ_HOST);
	http_port = HTTP_PORT;
	b_port = B_PORT;
	f_port = F_PORT;

	// proc cmd line
	while ((c = getopt (argc, argv, "b:u:f:vh")) != -1)
	{
		switch (c)
		{
			// -b bind_ip:port 
			case 'b':
					// ip:port ?
					if (*optarg != ':') 
					{
						strncpy(b_host, (idx = strtok(optarg, ":")) ? idx : "", SZ_HOST);
						b_port = atoi((idx = strtok(0, ":")) ? idx : "");
					}
	
					// solo :port
					else b_port = atoi((idx = strtok(optarg, ":")) ? idx : "");
			break;

			// -u http://hostname.domain:port/url?
			case 'u':
					// http://  -> ignoramos
					strtok(optarg, ":/"); 

					// hostname.domain
					strncpy(http_server, (idx = strtok(0, ":/")) ? idx : "", SZ_HOST);

					// port
					http_port = atoi((idx = strtok(0, "/")) ? idx : "");

					// url
					strncpy(url, (idx = strtok(0, "?")) ? idx : "", SZ_URL);
			break;

			// -f forward_ip:port 
			case 'f':
					// ip:port ?	
					if (*optarg != ':')
					{
						strncpy(f_host, (idx = strtok(optarg, ":")) ? idx : "", SZ_HOST);
						f_port = atoi((idx = strtok(0, ":")) ? idx : "");
					}
	
					// solo :port
					else f_port = atoi((idx = strtok(optarg, ":")) ? idx : "");
			break;

			// verbose 
			case 'v': verbose = 1; break; 

			// help 
			case 'h': usage(); exit(0); break;

			// error ?
			default: usage(0); exit(-1);
		}
	}

	// todo ok?
	if (!f_port || !http_port || !b_port || !strlen(f_host) || 
		!strlen(http_server) || !strlen(b_host) || !strlen(url))
	{
		usage();
		exit(-1);
	}

	printf("* listening on %s:%d\n", b_host, b_port);
	printf("* connecting to http://%s:%d/%s\n", http_server, http_port, url);
	printf("* forwarding to %s:%d\n\n", f_host, f_port);

	// manejador int 
	set_handlers();

        // creamos socket
	MSG("creating socket...\n");
        TRY(sock_s = socket(AF_INET, SOCK_STREAM, 0));

        // parametros conexion
	memset(&ads, 0, sizeof(ads));
        ads.sin_family = AF_INET;     
        ads.sin_port = htons(b_port);   
	ads.sin_addr.s_addr = inet_addr(b_host);

	// permitimos reutilizacion ip:port (time_wait)
    	TRY(setsockopt(sock_s, SOL_SOCKET, SO_REUSEADDR, 
		(const char *) &on, sizeof(on)));

        // bind a ip:puerto
	TRY(bind(sock_s, (struct sockaddr *)&ads, sizeof(ads)));

	// ponemos a escucha
	MSG("listening %s:%d...\n", b_host, b_port);
	TRY(listen(sock_s, 1));

	// hasta CTRL+C
	while (1)
	{
		// abrimos conexion
		if (is_closed(sock_c))
		{
			// aceptamos conexiones
			len = sizeof(adc);
			TRY(sock_c = accept(sock_s, (struct sockaddr *)&adc, &len));

			// lanzamos server
			MSG("starting tunnel...\n");
			http(HTTP_CTRL, "", 0); sleep(1);
		}

		// socket -> http (aunque 0)
		MSG("reading socket...\n");

		TRY(sz = xrecv(sock_c, sbuf, SZ_SBUF, RECV_TIMEOUT));
		TRY(http(HTTP_SEND, sbuf, sz));

		MSG("socket -> http_tunnel: %d bytes\n", sz);

		// http -> socket (si > 0)
		MSG("reading http_tunnel...\n");

		if ((sz = http(HTTP_RECV, sbuf, 0)) > 0)
			send(sock_c, sbuf, sz, 0);

		// error en lectura ?
		if (sz == -1) close(sock_c);

		MSG("http_tunnel -> socket: %d bytes\n", sz);
	}

	// nunca llega aqui
	return 0;
}
