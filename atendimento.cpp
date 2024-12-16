#include <iostream>

//Multi-Process Handling
#include <pthread.h>

//Process 
#include <csignal>
#include <unistd.h>

//Semaphore
#include <semaphore.h>
#include <fcntl.h>

//Time
#include <sys/time.h>
// Client struct
struct{
	pid_t pid;
	// Arrive time in u seconds (10^{-6})
	timeval arrive,exit;
	int priority;
	// wait time for service
	double wTime;
}typedef Client;

//Arg struct to pthread
struct reception_struct {
	int n;
	FILE* file;
};

sem_t* sem_atend;
sem_t* sem_block;

int randomPriority (){
	return (rand()%2);
}

void *stop (void* args){
	int i = 0;
	while(getchar()!='s')
		i = i+1;
	exit(0);
	return args;
}


void *service (void* arg){
	return nullptr;
}
void *reception (void *arguments){
	struct  reception_struct *args = (struct reception_struct *)arguments;
	sem_atend = sem_open("/sem_atend", O_CREAT | O_EXCL, 0644, 1);
	if (sem_atend == SEM_FAILED) {
		sem_unlink("/sem_atend");
		sem_atend = sem_open("/sem_atend", O_CREAT | O_EXCL, 0644, 1);
	}
	sem_block = sem_open("/sem_block", O_CREAT | O_EXCL, 0644, 1);
	if (sem_block == SEM_FAILED) {
		sem_unlink("/sem_block");
		sem_block = sem_open("/sem_block", O_CREAT | O_EXCL, 0644, 1);
	}
	std::cout << "Processes: " + args->n << '\n';
	exit(0);
	return nullptr;
}


int main (int narg,char *argv[]){ 
		int nProcesses;
	if (narg >= 2)
		nProcesses = atoi(argv[1]);
	else
		nProcesses = 0;
	sem_close(sem_atend);
	sem_unlink("/sem_atend");
	sem_close(sem_block);
	sem_unlink("/sem_block");
	int numCPU = sysconf(_SC_NPROCESSORS_ONLN);
	std::cout << "Avaliable Cores:  " << numCPU << "\n";
	FILE* lng = fopen("lng.txt","w+");
	struct reception_struct args;
	args.n = nProcesses;
	args.file = lng;
	pthread_t thread1,thread2;
	pthread_create (&thread1,NULL,&reception,(void*)&args);
	pthread_join(thread1,NULL);
	if ()
	pthread_create (&thread2,NULL,&stop,NULL);
	pthread_join(thread2,NULL);
	return 0;
}