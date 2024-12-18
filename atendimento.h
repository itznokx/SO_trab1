// Bibliotecas básicas
#include <stdio.h>
#include <stdlib.h>
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
#include <errno.h>

typedef struct FilaCliente
{
	Cliente* first;
	Cliente* last;
	int size;
	int max_size;
	// Bloqueador de fila
	pthread_mutex_t queue_mutex;
	// Condição de fila não cheia -> dequeue function use
	pthread_cond_t queue_not_full;
	// Condição de fila não vazia -> enqueue function use
	pthread_cond_t queue_not_empty;
}FilaCliente;

struct timeval start,end;
int		random_priority(int alloc_size);
Cliente* new_Client(pid_t pid,int serviceTime,int priority);
void 	destroy_queue(FilaCliente* queue);
void	start_queue(FilaCliente *queue,int alloc_size);
void  	enqueue(FilaCliente *queue,Cliente *client);
Cliente* dequeue(FilaCliente *queue);
void*	stop_program(void* args);
int		start_analist();
void	wake_analist();
void	calculate_satisfaction();
long	calculate_time_difference(	struct timeval arrive,
									struct timeval exit);
void* 	service(void* args);
void* 	reception(void* args);
void	clean();