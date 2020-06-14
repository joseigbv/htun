
/******************************************************
* tunelizador http 
* common.h
*******************************************************/

#ifndef COMMON_H
#define COMMON_H

#include <errno.h>

/******************
* macros aux 
*******************/

#define PROMPT "(unknown) "

// muestra error
#define ERR(...) { \
			if (verbose) \
			{ \
				fprintf(stderr, PROMPT); \
				fprintf(stderr, "%s * ", time_stamp()); \
				fprintf(stderr, "error: %s, line: %d\n", __FILE__, __LINE__); \
				fprintf(stderr, PROMPT); \
				fprintf(stderr, "%s * ", time_stamp()); \
			} \
			fprintf(stderr, "error: "); \
                        fprintf(stderr, __VA_ARGS__); \
		}

// muestra msg 
#define MSG(...) if (verbose)\
		{ \
                        printf(PROMPT); \
			printf("%s * ", time_stamp()); \
                        printf(__VA_ARGS__); \
                }

//  msg error y sale 
#define ABORT(...) { \
			if (verbose) \
			{ \
				fprintf(stderr, PROMPT); \
				fprintf(stderr, "%s * ", time_stamp()); \
				fprintf(stderr, "abort: %s, line: %d\n", __FILE__, __LINE__); \
				if (errno) \
				{ \
					fprintf(stderr, PROMPT); \
					fprintf(stderr, "%s * ", time_stamp()); \
					perror("abort"); \
				} \
				fprintf(stderr, PROMPT); \
				fprintf(stderr, "%s * ", time_stamp()); \
			} \
			else  perror("abort"); \
			fprintf(stderr, "abort: "); \
			fprintf(stderr, __VA_ARGS__); \
			clean(); exit(-1); \
		} 

// ejecuta, si -1, sale
#define TRY(x) if ((x) == -1) ABORT("try\n"); 


/******************
* funciones auxiliares
*******************/

int is_closed(int sock);
void set_handlers();
int xrecv(int s, char *sbuf, int sz_sbuf, int timeout);
unsigned long crono();
void do_exit(int e);
char *time_stamp();

#endif
