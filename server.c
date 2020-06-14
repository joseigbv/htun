
/******************************************************
* tunelizador http 
* server.c
*******************************************************/

#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "config.h"
#include "common.h"


/******************
* macros aux
*******************/

#undef PROMPT
#define PROMPT "(server) "


/******************
*  variables globales
*******************/

int fifo, sock, f_log, pid_s, f_port, verbose = VERBOSE;
char f_host[SZ_HOST], f_send[SZ_FILE], f_recv[SZ_FILE], f_pid[SZ_FILE];


/******************
*  funciones
*******************/

// libera recursos
void clean()
{
        MSG("exiting...\n");

	// matamos hijo
	MSG("killing server\n");
	kill(pid_s, SIGTERM); 

	// cerramos  sockets
	MSG("closing sockets...\n");
	close(sock);

	// cerramos  fifos
	MSG("closing fifos...\n");
	close(fifo);

	// y borramos 
	MSG("removing fifos...\n");
	unlink(f_recv);
	unlink(f_send);

	// fuera pid
	MSG("removing files...\n");
	unlink(f_pid);

	// cerramos log
	MSG("closing files\n");
	close(f_log);

        MSG(">>> done <<<\n\n");
}


// uso 
void usage()
{
	printf("Usage: server [host [port]]\n\n");
	exit(-1);
}


// server
void server()
{
	char sbuf[SZ_SBUF];
	struct sockaddr_in a; 
	struct hostent *host;
	int sz, conn = 0;

	MSG("running server...\n");

	// creamos fifos si no existen
	MSG("creating fifos...\n");
	mknod(f_recv, S_IFIFO | F_MOD, 0);
	mknod(f_send, S_IFIFO | F_MOD, 0);

	// creamos socket
	MSG("creating socket...\n");
	TRY(sock = socket(AF_INET, SOCK_STREAM, 0));
		
	// resolucion host
	MSG("resolving remote host...\n");
	host = gethostbyname(f_host);

	// parameros conexion
	memset(&a, 0, sizeof(a));
	a.sin_family = AF_INET;     
	a.sin_port = htons(f_port);   
	a.sin_addr = *((struct in_addr *)host->h_addr);

	// hasta CTRL+C
	while (1)
	{
		// abrimos fifo
		MSG("reading http_tunnel...\n");
		TRY(fifo = open(f_recv, O_RDONLY));
		TRY(sz = read(fifo, sbuf, SZ_SBUF));
		close(fifo); 

		// copiamos a socket (si > 0)
		if (sz) 
		{
			// reconectamos a server
			if (! conn)
			{
				MSG("connecting to %s:%d...\n", f_host, f_port);
				TRY(conn = !connect(sock, (struct sockaddr *)&a, sizeof(a)));
			}

			// enviamos datos
			TRY(send(sock, sbuf, sz, 0));
		}

		MSG("http_tunnel -> socket: %d bytes\n", sz);

		// leemos de socket
		MSG("reading socket...\n");
		TRY(sz = xrecv(sock, sbuf, SZ_SBUF, RECV_TIMEOUT));

		// copiamos de socket (aunque 0)
		TRY(fifo = open(f_send, O_WRONLY));
		TRY(write(fifo, sbuf, sz));
		close(fifo); 

		MSG("socket -> http_tunnel: %d bytes\n", sz);
	}
}


// monit server
void watchdog()
{
	struct stat st;
	int status, intv;
	time_t t_fi, t_fo, ti_fi, ti_fo;
	unsigned int t;

	MSG("checking server...\n");

	// fecha mod inicial fifos
	stat(f_recv, &st); ti_fi = st.st_ctime;
	stat(f_send, &st); ti_fo = st.st_ctime;

	// crono
	t = crono();

	// esperamos hasta salida
	while (! waitpid(pid_s, &status, WNOHANG))
	{
		// intervalo ?
		intv = crono() - t; 

		// fecha modificacion act fifos
		stat(f_recv, &st); t_fi = st.st_ctime;
		stat(f_send, &st); t_fo = st.st_ctime;

		// algun cambio ?
		if ((t_fi == ti_fi) && (t_fo == ti_fo))
		{
			// max time inactiv 
			if (intv > MAX_TIME) ABORT("timeout\n");
		}

		// reset crono
		else t = crono();

		// guardamos ultimo valor
		ti_fi = t_fi;
		ti_fo = t_fo;

		MSG("waiting (%d/%d)...\n", intv, MAX_TIME);

		// esperamos 1 seg.
		sleep(1);
	}
}


// funcion principal
int main(int argc, char *argv[])
{
	int sid, pid, f;
	char s_pid[7];

	// desactivamos buffer salida
	setbuf(stdout, 0);

	// banner 
        printf("%s\n", BANNER);
        printf("Starting server...\n\n");

	//
	// usage
	//

	// server 
	if (argc == 1) 
	{
		strncpy(f_host, F_HOST, SZ_HOST);
		f_port = F_PORT;
	}

	// server host 
	else if (argc == 2)
	{
		strncpy(f_host, argv[1], SZ_HOST);
		f_port = F_PORT;
	}

	// server host port 
	else if (argc == 3)
	{
		strncpy(f_host, argv[1], SZ_HOST);
		f_port = atoi(argv[2]);
	}

	// error ?
	else  usage();

	//
	// end usage
	//

	// filenames
	sprintf(f_send, "%s.%d", F_SEND, f_port);
	sprintf(f_recv, "%s.%d", F_RECV, f_port);
	sprintf(f_pid, "%s.%d", F_PID, f_port);

	// borramos pipes antiguos
	unlink(f_recv);
	unlink(f_send);

	// server corriendo ?
	TRY(f = open(f_pid, O_RDWR | O_CREAT, F_MOD));

	// si existe, paramos
	if (read(f, s_pid, sizeof(s_pid)))
	{
		pid = atoi(s_pid);
		kill(pid, SIGTERM);
	}

	// lanzamos fork
	TRY(pid = fork());

	// hijo, daemonizamos
	if (!pid)
	{
		// manejador int
		set_handlers();

		// default mask
		umask(0);
		
		// cambiamos sesion
		TRY(sid = setsid());

		// directorio por defecto
		chdir(WORK_DIR);

		// ajusta prioridad a baja
		nice(PRIO);

		// abrimos fichero logs
		TRY(f_log = open(F_LOG, O_WRONLY | 
			O_CREAT | O_APPEND, F_MOD));

		// stdout y stderr -> f
		dup2(f_log, 1);
		dup2(f_log, 2);

		// no stdin
		close(0);

		MSG(">>> http_tunnel sever <<<\n");
		MSG("initializing...\n");

		// lanzamos fork
		MSG("fork...\n");
		TRY(pid_s = fork());

		// lanzamos server y watchdog
		if (pid_s == 0) server(); 
			else watchdog();
	}

	// padre 
	else
	{
		// guardamos pid
		sprintf(s_pid, "%d", pid);
		lseek(f, 0, SEEK_SET);
		write(f, s_pid, strlen(s_pid));
		close(f);
	}

	return 0;
}
