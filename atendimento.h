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
	int priority;
}FilaCliente;

struct timeval start,end;
int		random_priority(int alloc_size);
Cliente* new_Client(pid_t pid,int serviceTime,int priority);
void 	destroy_queue(FilaCliente* queue);
void	start_queue(FilaCliente *queue,int alloc_size,int priority);
void  	enqueue(FilaCliente *queue,Cliente *client);
Cliente* dequeue(FilaCliente *queue);
void*	stop_program(void* args);
int		start_analist();
void	wake_analist();
void	calculate_satisfaction();
long	calculate_time_difference(	struct timeval arrive,
									struct timeval exit);
void 	calculate_program_time(struct timeval start,struct timeval end);
void* 	service(void* args);
void* 	reception(void* args);
void	clean();