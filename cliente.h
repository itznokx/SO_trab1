#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

typedef struct Cliente{
	struct Cliente* next;
	pid_t pid;
	struct timeval arrive,end;
	int serviceTime;
	int priority;
	long patience;
}Cliente;

