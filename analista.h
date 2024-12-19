// Bibliotecas Padroes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Bibliotecas de System-Process
#include <unistd.h>
#include <signal.h>

// Bibliotecas de Semaphoro
#include <semaphore.h>
#include <fcntl.h>

//função pra gravar o pid do analista no arquivo pidanalista.txt
void* allocmem (size_t size);
void fillmem (void* ptr,size_t size);
void start_analist();
void rewrite_lng(int buffer[],int c);
void* print_pids(int PID_MAX_PRINT);