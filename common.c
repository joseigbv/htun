
/******************************************************
* tunelizador http 
* common.c
*******************************************************/

#include <signal.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <time.h>
#include "common.h"

// no warnings
void clean();


// salida limpia
void do_exit(int c)
{
	clean();
	exit(0);
}


// configura manejadores int
void set_handlers()
{
	struct sigaction sig;

	sig.sa_handler = do_exit;
	sig.sa_flags = 0;
	sigemptyset(&sig.sa_mask);

	sigaction(SIGTERM, &sig, 0);
	sigaction(SIGINT, &sig, 0);
}


// segundos actuales
unsigned long crono()
{
        struct timeval tim;

        gettimeofday(&tim, NULL);

        return (unsigned long) ((tim.tv_sec * 1000000) + 
                tim.tv_usec) / 1000000;
}


// recv no block
int xrecv(int s, char *sbuf, int sz_sbuf, int timeout)
{
	int sz = 0, r; 
	fd_set rs;
	struct timeval tv;

	// configuramos fd
	FD_ZERO(&rs);
	FD_SET(s, &rs);

	// timeout
	tv.tv_sec = timeout;
	tv.tv_usec = 0;

	// lectura socket async 
	if ((r = select(s + 1, &rs, 0, 0, &tv)) > 0)
	{
		if (FD_ISSET(s, &rs))
			sz = recv(s, sbuf, sz_sbuf, 0);

		// pendiente 
		// r = (sz == 0 ? -1 : r); 

		// cerro la conexion?
		if (!sz) 
		{ 
			close(s);
		}
	}

	return r == -1 ? r : sz;
}


//  socket cerrado?
int is_closed(int sock)
{
	int err = 0;
	socklen_t len = sizeof(err);

	return getsockopt (sock, SOL_SOCKET, SO_ERROR, &err, &len );
}


// string timestamp
char *time_stamp()
{
	static char sbuf[32];
	time_t t;

	t = time(0);
	strncpy(sbuf, ctime(&t), 32);
	sbuf[strlen(sbuf) - 1] = 0;

	return sbuf;
}

