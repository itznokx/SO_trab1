#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

struct{
	pid_t pid;
	struct timeval arrive,end;
	int serviceTime;
	int priority;
	int patience;
}typedef Cliente;
