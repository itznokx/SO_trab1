// Bibliotecas básicas
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Bibliotecas de processo
#include <unistd.h> 
#include <signal.h>

// Bibliotecas de Multi-threading
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

// Bibliotecas de tempo
#include <sys/time.h>
#include <sys/wait.h>

// Bibliotecas de Memória
#include <sys/mman.h>

// Bilbiotecas auxiliares
#include "cliente.h"

struct{
	Cliente queue[1000];
	int front;
	int size;
	int count;
}typedef Queue;

struct timeval start,end;

void  	enqueueNormal(Cliente client);
Cliente dequeueNormal();
void  	enqueuPriority(Cliente client);
Cliente dequeuePriority();
void*	stop_program(void* args);
int		start_analist();
void	wake_analist();
void	calculate_satisfaction();
long	calculate_time_difference(	struct timeval arrive,
									struct timeval exit);
void* 	service(void* args);
void* 	reception(void* args);
void	clean();