#include <iostream>
#include <fstream>
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

struct{
	pid_t pid;
	// Arrive time in u seconds (10^{-6})
	timeval arrive,exit;
	int priority;
	// wait time for service
	double wTime;
}typedef Client;

sem_t* sem_atend;
sem_t* sem_block;

int randomPriority (){
	return (rand()%2);
}

void *stop (void* args){
	int i = 0;
	while(getchar()!='s')
		i++;
	exit(0);
}


void *service (void* arg){

}
void *reception (void *args){
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


}
struct reception_struct {
	int n;
	FILE* file;
};

int main (int narg,char *argv[]){ 
		int nProcesses;
	if (narg >= 2)
		nProcesses = atoi(argv[1]);
	else
		nProcesses = 100;
	sem_close(sem_atend);
	sem_unlink("/sem_atend");
	sem_close(sem_block);
	sem_unlink("/sem_block");
	int numCPU = sysconf(_SC_NPROCESSORS_ONLN);
	std::cout << nProcesses << '\n';
	std::cout << numCPU << "\n";
	FILE* lng = fopen("lng.txt","w+");
	struct reception_struct args;
	args.n = nProcesses;
	args.file = lng;
	pthread_t thread1,thread2;
	//pthread_create (&thread1,NULL,&reception,(void*)&args);
	pthread_create (&thread2,NULL,&stop,NULL);
	pthread_join(thread2,NULL);
	return 0;
}